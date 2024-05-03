#!/bin/bash

rm -f dump

if [ $# -ne 1 ]; then
    echo "Usage: $0 <path>"
    exit 1
fi

./main "$1" --animate

if [ $? -ne 0 ]; then
    echo "Voronoi algorithm failed."
    exit 1
fi

python external/animation.py

rm -f dump