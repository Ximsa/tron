#include "tron.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
// Game Timer
#define IRQ_0 (1193181.81818181 / 65536.0)

#define PLAYFIELD_WIDTH 24
#define PLAYFIELD_HEIGHT 20

#define MAX_PLAYERS 8

// player

#define PLAYER_DEAD 0
#define PLAYER_UP 1
#define PLAYER_LEFT 2
#define PLAYER_DOWN 3
#define PLAYER_RIGHT 4
#define PLAYER_DYING 5

#define MOVE_SUCESS 0
#define MOVE_CRASH 1

// text

#define CHAR_HEIGHT 7
#define CHAR_WIDTH 5

#define CHATBOX_X 216
#define CHATBOX_Y 8
#define CHATBOX_WIDTH 96
#define CHATBOX_HEIGHT 184
#define CHATBOX_LINES (CHATBOX_HEIGHT / (CHAR_HEIGHT + 1)) // 23
#define CHATBOX_LINELENGTH (CHATBOX_WIDTH / (CHAR_WIDTH + 1)) // 16

#define NAME_LENGTH 4
#define MESSAGE_LENGTH CHATBOX_LINELENGTH

#define PLAYERBOX_X 8
#define PLAYERBOX_Y 176
#define PLAYERBOX_WIDTH 192
#define PLAYERBOX_HEIGHT 8

int chatbox_y = CHATBOX_Y + 1;
SDL_AudioDeviceID dev = {0};
SDL_Window * win = NULL;
SDL_Renderer * renderer = NULL;
SDL_Texture * texture = NULL;
SDL_Palette * palette = NULL;

uint8_t colors[][3] =
  {
    {0x00, 0x00, 0x00}, /*  0 */
    {0x00, 0x00, 0xaa}, /*  1 */
    {0x00, 0xaa, 0x00}, /*  2 */
    {0x00, 0xaa, 0xaa}, /*  3 */
    {0xaa, 0x00, 0x00}, /*  4 */
    {0xaa, 0x00, 0xaa}, /*  5 */
    {0xaa, 0x55, 0x00}, /*  6 */
    {0xaa, 0xaa, 0xaa}, /*  7 */
    {0x55, 0x55, 0x55}, /*  8 */
    {0x55, 0x55, 0xff}, /*  9 */
    {0x55, 0xff, 0x55}, /* 10 */
    {0x55, 0xff, 0xff}, /* 11 */
    {0xff, 0x55, 0x55}, /* 12 */
    {0xff, 0x55, 0xff}, /* 13 */
    {0xff, 0xff, 0x55}, /* 14 */
    {0xff, 0xff, 0xff}, /* 15 */
  };
uint8_t screen_buffer[SCREEN_WIDTH*SCREEN_HEIGHT*4];
Uint16 audio_buffer[SOUND_MAX_LENGTH * SAMPLE_RATE * sizeof(Uint16)];
int graphics_enabled = 1;
int sound_enabled = 1;
int wait_for_tick = 0;

uint8_t field[PLAYFIELD_HEIGHT][PLAYFIELD_WIDTH] = {0};

typedef struct {
  int state;
  char name[NAME_LENGTH];
  int x;
  int y;
  uint32_t color;
  char message[MESSAGE_LENGTH];
  int lifetime;
} Player;

Player players[MAX_PLAYERS] = {0};

void draw()
{
  if(graphics_enabled)
    {
      SDL_UpdateTexture(texture, NULL, screen_buffer, SCREEN_WIDTH * 4);
      SDL_RenderCopy(renderer, texture, NULL, NULL);
      SDL_RenderPresent(renderer);
    }
}

void wait_n_ticks(uint16_t ticks)
{
  if(wait_for_tick && (graphics_enabled || sound_enabled))
    SDL_Delay((2*1000*ticks/(IRQ_0))); // wait for every second IRQ
}

#define PLAY_SOUND(sound) play_sound(sound,sizeof(sound))
void play_sound(uint16_t sound[][2], size_t sound_size)
{
  if(sound_enabled)
    {
      size_t sound_length = sound_size / 4;
      size_t buffer_length = sound_length * SAMPLE_RATE;
      size_t note = 0;
      size_t i = 0;
      for (size_t note_counter = sound[note][1] * SAMPLE_RATE / 18; i < buffer_length; i++, note_counter--)
	{
	  double frequency = (double)SOUND_MAX_FREQ / (double)sound[note][0];
	  audio_buffer[i] = ((size_t)(i * frequency * 2.0 / SAMPLE_RATE) % 2)*SOUND_VOLUME;
	  if (note_counter == 0)
	    {
	      note++;
	      note_counter = (size_t)((size_t)sound[note][1] * SAMPLE_RATE / IRQ_0);
	    }
	}
      SDL_ClearQueuedAudio(dev);
      SDL_QueueAudio(dev, audio_buffer, sizeof(Uint16) * i);
    }
}

void init_sound()
{
  SDL_Init(SDL_INIT_AUDIO);
  SDL_AudioSpec audio_want =
    {
      .freq = SAMPLE_RATE, // number of samples per second
      .format = AUDIO_U16, // sample type (here: unsigned short 16 bit)
      .channels = 1,
      .samples = SAMPLE_RATE / 12, // buffer-size (1/12s)
      .callback = NULL,	// function SDL calls periodically to refill the buffer
      .userdata = NULL,
    };
  SDL_AudioSpec audio_have;
  dev = SDL_OpenAudioDevice(NULL, 0, &audio_want, &audio_have, SDL_AUDIO_ALLOW_ANY_CHANGE);
  SDL_PauseAudioDevice(dev, 0);
}

#define INIT_GRAPHIC(graphic) load_rgb24(graphic, sizeof(graphic))
void load_rgb24(uint8_t * filename_and_buffer, size_t rgb_size)
{
  FILE * f = fopen((char*)filename_and_buffer, "rb");
  size_t bytes_read = fread(filename_and_buffer, 1, rgb_size, f);
  fclose(f);
  for(int i = bytes_read, j = rgb_size; j >=0 && i >= 0; i--,j--)
    {
      filename_and_buffer[j] = filename_and_buffer[i];
      if(i%3 == 0)
	j--;
    }
}

void init_graphics()
{
  INIT_GRAPHIC(GRAPHIC_UI);
}


uint8_t is_black(const uint8_t * color)
{
  return color[0] == 0 && color[1] == 0 && color[2] == 0;
}

// assumes RGBA32
void blit(uint16_t width, uint16_t height, // dimensions to blit
	  uint16_t src_x, uint16_t src_y, // source offset
	  uint16_t dest_x, uint16_t dest_y, // destiantion offset
	  size_t src_width, uint8_t * src_pixels, // source image
	  size_t dest_width, uint8_t * dest_pixels) // destination image
{
  if(graphics_enabled){
    for(size_t y = 0; y < height; y++)
      for(size_t x = 0; x < width; x++)
	{
	  size_t src_index = (y+src_y)*src_width+x+src_x;
	  size_t dest_index = (dest_y+y)*dest_width +dest_x+x;
	  ((uint32_t*)dest_pixels)[dest_index] = ((uint32_t*)src_pixels)[src_index];
	}
  }
}

// since it is impossible to Update a texture with alpha channel -> manual loop
void blit_to_screen(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t * src_pixels)
{
  blit(w, h, 0, 0, x, y, w, src_pixels, SCREEN_WIDTH, screen_buffer);
}

void init_UI()
{
  SDL_Init(SDL_INIT_VIDEO);
  win = SDL_CreateWindow("TRON",
			 SDL_WINDOWPOS_UNDEFINED,
			 SDL_WINDOWPOS_UNDEFINED,
			 SCREEN_WIDTH*4,
			 200*4,
			 SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
  SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
  init_graphics();
  blit_to_screen(0, 0, 320, 200, GRAPHIC_UI);
  draw();
}

// text

char CHARMAP[CHAR_HEIGHT][CHAR_WIDTH][64] =
  {
    {
      "%357BDEFHKLMNPRTUVWXYZbhk",
      "\"#%&')0235789>?@ABCDEFGIOPQRSTZ[]`lt}",
      "!$&'012356789?@ABCDEFGIJOPQRSTZ[]^fil|",
      "\"#(023456789<?A@BCDEFGIJOPQRSTZ[]fj{",
      "357EFHJKMNSTUVWXYZd",

    },
    {
      "%&025789?@ABCDEFGHKLMNOPQRSUVWXY\\bhk",
      "\"#$%16:;M[^fgt",
      "!$'()*+14:;<>IT`gl{|}",
      "\"#$&34DJKM^g",
      "$%/02789?@ABCGHMNOPQRUVWXYZdf",
    },
    {
      "#$&*05689=ABCDEFGHKLMNOPQRSUVWY^bghkmnprtuvwxyz",
      "\"#'(45:;<=NX[\\acdefimopqstz",
      "!#$&*+135:;=IKMTabcdehijlnopqrstz{|}",
      "\"#%)/0457=>JXZ]`abcehjkmnoprsz",
      "#$*0289=?@ABDHMNOPQRUVWY^dgmquvwxyz",
    },
    {
      "+-046<ABCEFGHKLMNOPQRUVWbcdefghkmnopqrsuvwy",
      "#$&(*+-689@BEFHKPRSY[bfhnrtx{",
      "!$%*+-/016789@BEFGHIMNPRSTWXZ\\fiklm|",
      "#$)*+-234689?BEFGHJPRSY]djqxz}",
      "+-059>@ADGHMNOQUVWabdeghmnopqruvwy",
    },
    {
      "#&*0468=@ABCDEFGHKLMNOPQRUVWbcdehkmnopruvw",
      "#%(,/047:;<=AXZ[aefgkpqsty",
      "#$&*+,124:;=?@AIKQRTWYaegilmpqswxyz{|}",
      "#)4=>AJNX[\\aegjpqsy",
      "#$&)*0345689=@ABDGHMNOQSUVWabdeghmnoquvwy",
    },
    {
      "$%&/03568@ABCDEFGHJKLMNOPQRUWXZabcdehjkmnopruw",
      "#$.27:V[ftvxz",
      "$()*+,.1:;<>@ITWY[]iklw{|}",
      "#$%&49DJKQRVjuvx",
      "%03568@ABCGHMNOSUWX\\abcdghmnoqstuwy",
    },
    {
      "2ABDEFHKLMNPRSXZ_bhkmnprsxz",
      "#&),.0122356789;>@BCDEGIJLOQSUWZ[]_abcdefgijlosuwyz}",
      "!$&.01235689?@BCDEGIJLOQSTUVYZ[]_abcdegijlostzuvy|",
      "#%(012345668<@BCEGILOSUWZ[]_abcdegiklostwyz{",
      "%&2AEGHKLMNQRXZ_adhmnquxz",
    },			
  };

void write_char(int char_x, int char_y, uint32_t color, char c)
{
  uint32_t bitmap[5*7] = {};
  for(int y = 0; y < CHAR_HEIGHT; y++)
    for(int x = 0; x < CHAR_WIDTH; x++)
      //look if char is in charmap
      for(char * current = CHARMAP[y][x]; *current != 0; current++)
	if(*current == c)
	  { // found
	    bitmap[y*CHAR_WIDTH+x] = color;
	    break;
	  }
  blit_to_screen(char_x, char_y, CHAR_WIDTH, CHAR_HEIGHT, (uint8_t*) bitmap);
}

void write_text(int x, int y, uint32_t color, int length, char * text)
{
  for(int i = 0; i < length; i++)
    {
      write_char(x + (CHAR_WIDTH + 1) * i, y, color, text[i]);
    }
}

void write_message(uint32_t color, int message_length, char * message)
{
  // truncate strings
  message_length = message_length > MESSAGE_LENGTH? MESSAGE_LENGTH : message_length;
  write_text(CHATBOX_X, chatbox_y, color, message_length, message);
  chatbox_y = chatbox_y + CHAR_HEIGHT + 1;
  if(chatbox_y > CHATBOX_Y + CHATBOX_HEIGHT)
    chatbox_y = CHATBOX_Y + 1;
}

int field_to_screen_x(int x)
{
  return (x*8)+8;
    
}
int field_to_screen_y(int y)
{
  return ((PLAYFIELD_HEIGHT-y-1)*8)+8;
}

int count_players()
{
  int count = 0;
  for(int i = 0; i < MAX_PLAYERS; i++)
    if(players[i].name[0] != 0)
      count++;
  return count;
}

void write_player_names()
{
  int y = PLAYERBOX_Y + 1;
  for(int player_id = 0; player_id < MAX_PLAYERS; player_id++)
    {
      Player * player = players + player_id;
      if(player->name[0] != 0)
	{
	  int x = field_to_screen_x(player->x) + 3 - (2*CHAR_WIDTH);
	  write_text(x, y, player->color, NAME_LENGTH, player->name);
	}
    }
}

// game

void draw_field()
{
  for(int y = 0; y < PLAYFIELD_HEIGHT; y++)
    for(int x = 0; x < PLAYFIELD_WIDTH; x++)
      {
	uint8_t player_id = field[y][x];
	uint32_t color = field[y][x] == 0xFF? 0x00000000 : players[player_id].color;
	uint32_t colors[] = {[0 ... (8*8-1)] = color};
	blit_to_screen(field_to_screen_x(x), field_to_screen_y(y), 8, 8, (uint8_t*)colors);
      }
}

void remove_player(int player_id)
{
  for(int y = 0; y < PLAYFIELD_HEIGHT; y++)
    for(int x = 0; x < PLAYFIELD_WIDTH; x++)
      if(field[y][x] == player_id)
	field[y][x] = 0xFF;
  players[player_id].state = PLAYER_DEAD;
}

int move(int player_id, int x, int y, int dx, int dy)
{
  x = (x+PLAYFIELD_WIDTH+dx) % PLAYFIELD_WIDTH;
  y = (y+PLAYFIELD_HEIGHT+dy) % PLAYFIELD_HEIGHT;
  if(field[y][x] != 0xFF)
    return MOVE_CRASH;
  field[y][x] = player_id;
  players[player_id].x = x;
  players[player_id].y = y;
  return MOVE_SUCESS;
}

int tick()
{
  int player_count = 0;
  for(int player_id = 0; player_id < MAX_PLAYERS; player_id++)
    {
      Player * player = players + player_id;
      if(player->name[0] != 0)
	{
	  int x = player->x;
	  int y = player->y;
	  int result = MOVE_SUCESS;
	  if(player->state != PLAYER_DEAD)
	    {
	      player_count++;
	      (player->lifetime)++;
	    }
	  switch(player->state)
	    {
	    case PLAYER_DEAD:
	      break;
	    case PLAYER_DOWN:
	      result = move(player_id, x, y, 0, -1);
	      break;
	    case PLAYER_LEFT:
	      result = move(player_id, x, y, -1, 0);
	      break;
	    case PLAYER_RIGHT:
	      result = move(player_id, x, y, 1, 0);
	      break;
	    case PLAYER_UP:
	      result = move(player_id, x, y, 0, 1);
	      break;
	    default:
	      break;
	    }
	  if(result == MOVE_CRASH)
	    player->state = PLAYER_DYING;
	}
    }
  // remove dead players and handle messages
  for(int player_id = 0; player_id < MAX_PLAYERS; player_id++)
    {
      Player * player = players + player_id;
      if(player->state != PLAYER_DEAD && *(player->message) != 0)
	{
	  write_message(player->color, MESSAGE_LENGTH, player->message);
	  memset(player->message, 0, MESSAGE_LENGTH);
	}
      if(player->state == PLAYER_DYING)
	{
	  SDL_Log("player %i dying\n", player_id);
	  remove_player(player_id);
	}
    }
  // draw if needed
  if(graphics_enabled)
    {
      draw_field();
      draw();
    }
  return player_count;
}

void initialize_game()
{
  //clear field
  for(int y = 0; y < PLAYFIELD_HEIGHT; y++)
    for(int x = 0; x < PLAYFIELD_WIDTH; x++)
      field[y][x] = 0xFF;
  // determine player starting locations and
  // place players
  float player_count = count_players();
  for(int player_id = 0; player_id < MAX_PLAYERS &&
	*(players[player_id].name) != 0; player_id++)
    {
      Player * player = players + player_id;
      player->state = PLAYER_UP;
      float distance_y = (PLAYFIELD_HEIGHT / player_count);
      player->y = distance_y / 2 + distance_y * player_id;
      float distance_x = (PLAYFIELD_WIDTH / player_count);
      player->x = distance_x / 2 + distance_x * player_id;
      field[player->y][player->x] = player_id;
    }
  if(graphics_enabled)
    {
      write_player_names();
      draw_field();
      draw();
    }
}

void register_player(int name_length, char * name)
{
  for(int i = 0; i < MAX_PLAYERS; i++)
    {
      if(players[i].name[0] == 0) // free slot
	{
	  // copy name
	  for(int j = 0; name != 0 && j < NAME_LENGTH && j < name_length; name++, j++)
	    players[i].name[j] = *name;
	  players[i].x = 0;
	  players[i].y = 0;
	  players[i].state = PLAYER_UP;
	  players[i].lifetime = 0;
	  break;
	}
    }
}

void reset()
{
  for(int player_id = 0; player_id < MAX_PLAYERS; player_id++)
    {
      Player * player = players + player_id;
      memset(player->name,0,NAME_LENGTH);
      player->color = *((uint32_t*)colors[player_id+4]);
      player->state = PLAYER_DEAD;
      player->x = 0;
      player->y = 0;
      player->lifetime = 0;
    }
  if(graphics_enabled)
    {
      blit_to_screen(0, 0, 320, 200, GRAPHIC_UI);
      draw();
    }
}

void setup(uint8_t graphics, uint8_t sound, uint8_t fast)
{
  graphics_enabled = graphics;
  sound_enabled = sound;
  wait_for_tick = !fast;
  if(graphics_enabled) init_UI();
  if(sound_enabled) init_sound();
  reset();
  if(graphics_enabled)
    {
      blit_to_screen(0, 0, 320, 200, GRAPHIC_UI);
      draw();
    }
}

void get_player_stats(int * xs, int * ys, int * states, int * lifetimes)
{
  for(size_t player_id = 0; player_id < MAX_PLAYERS; player_id++)
    {
      xs[player_id] = players[player_id].x;
      ys[player_id] = players[player_id].y;
      states[player_id] = players[player_id].state;
      lifetimes[player_id] = players[player_id].lifetime;
    }
}
void set_player_direction(int player_id, int d)
{
  switch (d)
    {
    case 0: players[player_id].state = PLAYER_UP; break;
    case 1: players[player_id].state = PLAYER_LEFT; break;
    case 2: players[player_id].state = PLAYER_DOWN; break;
    case 3: players[player_id].state = PLAYER_RIGHT; break;
    default: break;
    }
}
void send_message(int player_id, int message_length, char message[])
{
  for(int j = 0; message != 0 && j < MESSAGE_LENGTH && j < message_length; message++, j++)
    players[player_id].message[j] = *message;
}

void get_playfield(uint8_t f[])
{
  for(int y = 0; y < PLAYFIELD_HEIGHT; y++)
    for(int x = 0; x < PLAYFIELD_WIDTH; x++)
      f[y*PLAYFIELD_WIDTH+x] = field[y][x];
}
void set_graphics(int graphics)
{
  graphics_enabled = graphics;
}
void set_wait_for_tick(int wait)
{
  wait_for_tick = wait;
}


int main(int argc, char * argv[])
{
  setup(1,1,0);
  register_player(3, "123");
  register_player(3, "456");
  register_player(3, "789");
  initialize_game();
  SDL_Event event;
  while(1)
    {
      tick();
      wait_n_ticks(1);
      int result = SDL_WaitEvent(&event);
      if (event.type == SDL_QUIT)
	return 0;
    }
  return 0;
}
