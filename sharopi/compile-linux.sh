#!/bin/sh

clang -g -O0 -Wall -Werror -Wextra -pedantic -std=c99 -ljack -lm -o shalopro aio_jack.c audio_core.c main_linux.c
