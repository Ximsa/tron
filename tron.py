import ctypes
from numpy.ctypeslib import ndpointer
from toolz import partition
import pathlib
import numpy as np
import matplotlib.pyplot as plt
import torch
import torch.nn as nn

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
tron.get_player_stats.argtypes = [ndpointer(ctypes.c_int), ndpointer(ctypes.c_int), ndpointer(ctypes.c_int), ndpointer(ctypes.c_int)]
def get_player_stats():
    xs = np.zeros(8, dtype=ctypes.c_int)
    ys = np.zeros(8, dtype=ctypes.c_int)
    states = np.zeros(8, dtype=ctypes.c_int)
    lifetimes = np.zeros(8, dtype=ctypes.c_int)
    tron.get_player_stats(xs,ys,states,lifetimes)
    return xs,ys,states,lifetimes
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

def create_population():
    return [torch.nn.Sequential(nn.Linear(20*24, 4), nn.ReLU()) for i in range(8*8)]

def get_model_action(model, playfield):
    return torch.argmax(model(torch.from_numpy(playfield.flatten()).float())).item()

def run_games(population):
    partitioned_population = partition(8, population)
    fitnesses = np.array([])
    set_graphics(1)
    for part in partitioned_population:
        reset()
        for individual in part:
            register_player("bot")
        initialize_game()
        alive_players = 8
        while(alive_players > 0):
            i = 0
            for x, y, state, lifetime in zip(*get_player_stats()):
                i = i + 1
                if state != 0:
                    action = get_model_action(part[i], get_playfield_centered(x,y))
                    set_player_direction(i, action)
            wait_n_ticks(1)
            alive_players = tick()
        xs, ys, states, lifetimes = get_player_stats()
        fitnesses = np.append(fitnesses, lifetimes)
        set_graphics(0)
    return fitnesses

setup(1,0,1)
run_games(create_population())
register_player("123")
initialize_game()
set_wait_for_tick(1)
set_graphics(1)
tick()
reset()
# ga
send_message(2,"hello world")
model = torch.nn.Sequential(nn.Linear(20*24, 4), nn.ReLU())
model[0].weight
torch.argmax(get_model_action(model, get_playfield_centered(0,0)))
