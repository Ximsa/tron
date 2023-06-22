import ctypes
from numpy.ctypeslib import ndpointer
from toolz import partition
import pathlib
import numpy as np

libname = pathlib.Path().absolute() / "libtron.so"
tron = ctypes.CDLL(libname)
playfield_shape = [24,20]

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
def get_playfield():
    field = np.zeros(24*20, dtype=ctypes.c_int8)
    tron.get_playfield(field)
    return field

# ga tron interaction

np.array([])

def run_games(population):
    partitioned_population = partition(8, population)
    fitnesses = np.array([])
    for part in partitioned_population:
        reset()
        for individual in part:
            register_player("bot")
        initialize_game()
        alive_players = tick()
        while(alive_players > 0):
            alive_players = tick()
        xs, ys, states, lifetimes = get_player_stats()
        fitnesses = np.append(fitnesses, lifetimes)
    return fitnesses
setup(1,0,0)
run_games([0]*8*8)

x = [1,2,3,4,5]
y = [5,6,7,8,9]
np.stack((x,y), axis = -1)
# ga

def create_population():
    return [np.zeros(playfield_shape + [4]) * 8 * 8]
