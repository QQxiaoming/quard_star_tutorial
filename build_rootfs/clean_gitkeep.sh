#!/bin/bash

if [ -d "$1/" ]; then  
cd $1/
find . -name ".gitkeep" -print0 | xargs -0 rm -rf
fi
