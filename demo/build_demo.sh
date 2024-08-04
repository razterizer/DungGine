#!/bin/bash


additional_flags="-I../.."

../../Core/build.sh demo "$1" "${additional_flags[@]}"

### Post-Build Actions ###

mkdir -p bin/fonts/
cp ../../../lib/Termin8or/fonts/* bin/fonts/

mkdir -p bin/textures/
cp textures/* bin/textures/
