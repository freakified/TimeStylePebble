#!/usr/bin/env bash
# Builds a looping, cross-fading GIF from the PNGs in each subdirectory of ../screenshots.
# Output: ../screenshots/<dirname>.gif

set -euo pipefail

ROOT="../screenshots"
MORPH=10 # blended frames between each pair of images
HOLD=200 # linger time on each image (hundredths of a second)
FADE=5   # delay per blended frame (hundredths of a second)

shopt -s nullglob

for dir in "$ROOT"/*/; do
  dir="${dir%/}"
  name="$(basename "$dir")"
  pngs=("$dir"/*.png) # glob expands alphabetically

  if ((${#pngs[@]} < 2)); then
    echo "Skipping $name: needs at least 2 PNGs"
    continue
  fi

  echo "Creating $name.gif from ${#pngs[@]} images..."
  magick "${pngs[@]}" \( -clone 0 \) -morph "$MORPH" \
    -set delay "%[fx:t==n-1?$FADE:(t%$((MORPH + 1))==0?$HOLD:$FADE)]" \
    -layers optimize -loop 0 "$ROOT/$name.gif"
done

echo "Done."
