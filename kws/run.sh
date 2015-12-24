#!/bin/bash
cd TAR0730/
pocketsphinx_continuous -inmic yes -lm 0730.lm -dict 0730.dic -samprate 16000/8000/48000
