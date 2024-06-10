#!/bin/bash

set -x

gcc main.c -o SpaceInvader $(pkg-config allegro-5 allegro_font-5 allegro_primitives-5 allegro_audio-5 allegro_acodec-5 allegro_image-5 --libs --cflags) && ./SpaceInvader
