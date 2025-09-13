#!/bin/bash


additional_flags="-I../include \
  -I../../Core/include \
  -I../../Termin8or/include"

../../Core/build.sh demo "$1" "${additional_flags[@]}"

# Capture the exit code of Core/build.sh
exit_code=$?

if [ $exit_code -ne 0 ]; then
  echo "Core/build.sh failed with exit code $exit_code"
  exit $exit_code
fi

### Post-Build Actions ###

mkdir -p bin/fonts/
cp ../../Termin8or/include/Termin8or/title/fonts/* bin/fonts/

mkdir -p bin/textures/
cp textures/* bin/textures/
