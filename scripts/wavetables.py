# Adrien Fauconnet - 2018
# this file will generate wavetables.h
# usage : python wavetables.py > ../include/wavetables.h

import numpy as np
import numpy.random as rd
from math import pi

size = 256
min_value = 0
max_value = 255
datatype = np.uint8

def print_const(arr, name):
    print("const uint8_t "+name.upper()+" [] = ")
    print(np.array2string(arr, separator=",", threshold=size+1).replace("[", "{").replace("]", "};"))
    print("")

saw = np.linspace(min_value, max_value, size, dtype=datatype)

angles = np.linspace(0, 2*pi, size)
sine = np.array(max_value/2*(1 + np.sin(angles)), dtype=datatype)

tri_section_1, tri_step = np.linspace(max_value/2, max_value, size/4, retstep=True, dtype=datatype)
tri_section_23 = np.linspace(max_value-tri_step, min_value, size/2, dtype=datatype)
tri_section_4 = np.linspace(min_value+tri_step, max_value/2-tri_step, size/4, dtype=datatype)
triangle = np.concatenate((tri_section_1, tri_section_23, tri_section_4), axis=0)

square = np.array([max_value]*(size/2) + [0]*(size/2), dtype=datatype)

noise = rd.randint(min_value, max_value, size, datatype)

print("#ifndef WAVETABLES_H")
print("#define WAVETABLES_H")
print("#define N {}".format(size))
print_const(saw, "SAW")
print_const(sine, "SINE")
print_const(triangle, "TRIANGLE")
print_const(square, "SQUARE")
print_const(noise, "NOISE")
print("#endif")
