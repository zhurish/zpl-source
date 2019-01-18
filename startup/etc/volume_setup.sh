#!/bin/sh


#DSP
amixer cset numid=45 1

amixer cset numid=3 2

amixer cset numid=101 0

# aplay
amixer cset numid=83 1

amixer cset numid=82 1

amixer cset numid=63 1

amixer cset numid=67 1

amixer cset numid=53 1

amixer cset numid=52 1

amixer cset numid=93 1

amixer cset numid=91 1

amixer cset numid=7 1 1
echo 1 > /sys/class/leds/lm4890\:status/brightness

# record
#amixer cset numid=45 1

#amixer cset numid=3 2

#amixer cset numid=101 0

#amixer cset numid=100 0

amixer cset numid=112 1

amixer cset numid=110 1

amixer cset numid=128 1

amixer cset numid=131 1

amixer cset numid=17 1 1
amixer cset numid=18 100 100

