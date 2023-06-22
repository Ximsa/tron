#include <stdint.h>

#define CHAR_HEIGHT 7
#define CHAR_WIDTH 5

#define CHATBOX_X 216
#define CHATBOX_Y 8
#define CHATBOX_WIDTH 96
#define CHATBOX_HEIGHT 184
#define CHATBOX_LINES (CHATBOX_HEIGHT / (CHAR_HEIGHT + 1)) // 23
#define CHATBOX_LINELENGTH (CHATBOX_WIDTH / (CHAR_WIDTH + 1)) // 16

#define PLAYERBOX_X 8
#define PLAYERBOX_Y 176
#define PLAYERBOX_WIDTH 192
#define PLAYERBOX_HEIGHT 8

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

void write_char(int x, int y, uint32_t color, char c)
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
  blit_
}

void write_text(int x, int y, uint32_t color, int length, char * text)
{
  
}
