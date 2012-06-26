#!/bin/bash

IN=/tmp/faces.dat
OUT=/tmp/faces.png

convert -size 1000x600 xc:black $OUT

FIRST=""
PREV=""

exec<$IN
while read line
do
    if [ -z $FIRST ]
    then
        FIRST=$line
        PREV=""
        echo ""
    fi

    if [ -z $line ]
    then
        line=$FIRST

        FIRST=
    fi

    if [ ! -z $PREV ]
    then
        echo "$PREV -> $line"
        mogrify -fill white -draw "line $PREV $line" $OUT
    fi

    PREV=$line
done

mogrify -flip $OUT
