#!/bin/bash


BIN=./borticles

rm -f "${BIN}" && clear && gcc ./main.c ./glad/src/glad.c -lglfw -lGL -lm -I. -I./glad/include -o "${BIN}" && "${BIN}"
