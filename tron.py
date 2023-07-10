import ctypes
from numpy.ctypeslib import ndpointer
from toolz import partition
import pathlib
import numpy as np
import matplotlib.pyplot as plt
import torch
import torch.nn as nn
import random
import neat
import visualize
import skimage

libname = pathlib.Path().absolute() / "libtron.so"
tron = ctypes.CDLL(libname)
playfield_shape = [24,20]

UP = 0
LEFT = 1
DOWN = 2
RIGHT = 3

# tron c interface
def setup(graphics=1, sound=1, fast=0): tron.setup(graphics,sound,fast)
def reset(): tron.reset()
def register_player(player_name):
    return tron.register_player(len(player_name), bytes(player_name, encoding='ascii'))
def initialize_game(): tron.initialize_game()
def tick(): return tron.tick()
tron.get_player_stats.argtypes = [ndpointer(ctypes.c_int), ndpointer(ctypes.c_int), ndpointer(ctypes.c_int), ndpointer(ctypes.c_int), ndpointer(ctypes.c_int)]
def get_player_stats():
    xs = np.zeros(8, dtype=ctypes.c_int)
    ys = np.zeros(8, dtype=ctypes.c_int)
    states = np.zeros(8, dtype=ctypes.c_int)
    lifetimes = np.zeros(8, dtype=ctypes.c_int)
    dead_by = np.zeros(8, dtype=ctypes.c_int)
    tron.get_player_stats(xs,ys,states,lifetimes, dead_by)
    return xs,ys,states,lifetimes,dead_by
def send_message(player_id, message): tron.send_message(player_id, len(message), bytes(message, encoding='ascii'))
tron.get_playfield.argtypes = [ndpointer(ctypes.c_uint8)]
def get_playfield():
    field = np.zeros(24*20, dtype=ctypes.c_uint8)
    tron.get_playfield(field)
    field = field.reshape([20,24])
    return field
def set_wait_for_tick(wait):
    tron.set_wait_for_tick(wait)
def set_graphics(graphics):
    tron.set_graphics(graphics)
def wait_n_ticks(ticks):
    tron.wait_n_ticks(ticks)
def set_player_direction(player_id, d):
    tron.set_player_direction(player_id, d)
    
def get_playfield_centered(x,y):
    return np.vectorize(lambda x: 0 if x == 0xFF else 1)(np.roll(get_playfield(), [12-x,10-y], [1,0]))

def get_distances(x,y):
    playfield = get_playfield()
    right = 1
    while(playfield[y,(x+right+24)%24] == 0xFF):
        right = right + 1
    left = 1
    while(playfield[y,(x-left+24)%24] == 0xFF):
        left = left + 1
    up = 1
    while(playfield[(y+up+20)%20,x] == 0xFF):
        up = up + 1
    down = 1
    while(playfield[(y-down+20)%20,x] == 0xFF):
        down = down + 1
    return [up, left, down, right]

def floodfill(x, y, field):
    x = (x+24)%24
    y = (y+20)%20
    result = 0
    if field[y,x] == 0:
        mask = skimage.segmentation.flood(field, (y,x),connectivity=1)
        result = mask.sum()
    return result
    

def get_potential(x,y):
    playfield = get_playfield_centered(x,y) # 12,10
    return [1+floodfill(12,11, (playfield)),
            1+floodfill(11,10, (playfield)),
            1+floodfill(12,9, (playfield)),
            1+floodfill(13,10, (playfield)),]

def prevent_crash(x,y,action_scores, sample=False):
    playfield = get_playfield()
    actions = [0,0,0,0]
    if playfield[y,(x+24+1)%24] == 0xFF:
        actions[RIGHT] = 1
    if playfield[y,(x+24-1)%24] == 0xFF:
        actions[LEFT] = 1
    if playfield[(y+20+1)%20,x] == 0xFF:
        actions[UP] = 1
    if playfield[(y+20-1)%20,x] == 0xFF:
        actions[DOWN] = 1
    scores = np.multiply(action_scores, actions)
    if sample:
        return int(np.random.choice(np.argwhere(scores == np.amax(scores)).flatten(),1))
    else:
        return int(np.argmax(scores))

def eval_genomes(genomes, config):
    set_graphics(1)
    # sanity check
    for genome_id, genome in genomes:
        genome.fitness = 1
    random.shuffle(genomes)
    n = random.randint(2,7)
    partitioned_genomes = partition(n, genomes)
    best_genomes = []
    for part in partitioned_genomes:
        reset()
        nets = []
        for genome_id, genome in part:
            #nets.append(neat.nn.feed_forward.FeedForwardNetwork.create(genome, config))
            nets.append(neat.nn.recurrent.RecurrentNetwork.create(genome, config))
            register_player(str(int(genome_id / 100)))
        register_player("rdfx")
        register_player("flix")
        register_player("rd")
        op_id = len(part)
        op2_id = op_id + 1
        op3_id = op2_id + 1
        initialize_game()
        alive_players = 8
        cheat_scores = np.ones(8)
        strat_1 = [random.random() for i in range(4)]
        while(alive_players > 1):
            wait_n_ticks(1)
            i = 0
            for x, y, state, lifetime, dead_by in zip(*get_player_stats()):
                if state != 0:
                    if i == op_id:
                        set_player_direction(i, prevent_crash(x,y,get_potential(x,y), True))
                    elif i == op2_id:
                        set_player_direction(i, prevent_crash(x,y,get_potential(x,y)))
                    elif i == op3_id:
                        set_player_direction(i, prevent_crash(x,y,strat_1))
                    else:
                        genome_id, genome = part[i]
                        
                        action_scores = nets[i].activate(np.concatenate((get_distances(x,y),get_potential(x,y))))
                        #action_scores = nets[i].activate(get_playfield_centered(x,y)[8:15,6:13].flatten())
                        #[9:14,7:12]
                        set_player_direction(i, prevent_crash(x,y,action_scores))
                i = i + 1
            alive_players = tick()
        set_graphics(0)
        # get fitnesses
        i = 0
        _, _, _, lifetimes, _ = get_player_stats()
        #lifetimes = np.argsort(lifetimes)
        # fitness is order of survival times kill score
        for i in range(n):
            part[i][1].fitness = int(lifetimes[i])
        for x, y, state, lifetime, dead_by in zip(*get_player_stats()):
            if(i < len(part) and dead_by != i and dead_by < n and dead_by >= 0):
                part[dead_by][1].fitness = part[dead_by][1].fitness * 2#float(lifetime) - (cheat_scores[i] * 0.5)**2
            i = i + 1
    # show best
    #return
    n = random.randint(2,8)
    sorted_genomes = sorted(genomes, key=lambda x: x[1].fitness, reverse=True)
    set_graphics(1)
    reset()
    nets = []
    cheat_scores = np.ones(8)
    for genome_id, genome in sorted_genomes[:n]:
        nets.append(neat.nn.recurrent.RecurrentNetwork.create(genome, config))
        #nets.append(neat.nn.feed_forward.FeedForwardNetwork.create(genome, config))
        register_player(str(int(genome_id/100)))
    register_player("flix")
    initialize_game()
    alive_players = 8
    while(alive_players > 1):
        i = 0
        for x, y, state, lifetime, _ in zip(*get_player_stats()):
            if state != 0:
                if i == len(nets):
                    set_player_direction(i, prevent_crash(x,y,get_potential(x,y)))
                else:
                    genome_id, genome = sorted_genomes[i]
                    action_scores = nets[i].activate(np.concatenate((get_distances(x,y),get_potential(x,y))))
                    #action_scores = nets[i].activate(get_playfield_centered(x,y)[8:15,6:13].flatten())
                    #action = int(np.argmax(action_scores))
                    action = prevent_crash(x,y,action_scores)
                    set_player_direction(i, action)
            i = i + 1
        wait_n_ticks(1)
        alive_players = tick()

setup(1,0,0)
config = neat.Config(neat.DefaultGenome,
                     neat.DefaultReproduction,
                     neat.DefaultSpeciesSet,
                     neat.DefaultStagnation,
                     'neat-config')

p = neat.Population(config)
p.add_reporter(neat.StdOutReporter(True))
stats = neat.StatisticsReporter()
p.add_reporter(stats)
p.add_reporter(neat.Checkpointer(5))


winner = p.run(eval_genomes, 10000)

visualize.draw_net(config, winner, True)
visualize.draw_net(config, winner, True, prune_unused=True)
visualize.plot_stats(stats, ylog=False, view=True)
visualize.plot_species(stats, view=True)

p = neat.Checkpointer.restore_checkpoint('neat-checkpoint-9')
p.run(eval_genomes, 1)


#setup(1,0,1)
#run_games(create_population(8*8))
#register_player("123")
#initialize_game()
#set_wait_for_tick(1)
#set_graphics(1)
#tick()
#reset()
# ga
#send_message(2,"hello world")
