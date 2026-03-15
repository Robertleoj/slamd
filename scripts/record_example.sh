#!/bin/bash
#
# Record a slamd window to GIF.
#
# Usage: ./scripts/record_example.sh <example_script> <output.gif> [duration_seconds] [wait_seconds]
#
# Example: ./scripts/record_example.sh examples/galaxy.py images/galaxy.gif 5 10

set -euo pipefail

EXAMPLE="${1:?Usage: record_example.sh <script.py> <output.gif> [duration] [wait]}"
OUTPUT="${2:?Usage: record_example.sh <script.py> <output.gif> [duration] [wait]}"
DURATION="${3:-5}"
SETTLE_TIME="${4:-3}"
FPS=30
GIF_WIDTH=800

# Start the example in background
python3 "$EXAMPLE" &
PY_PID=$!
trap "kill $PY_PID 2>/dev/null" EXIT

# Wait for any new window to appear, then let the scene settle
echo "Waiting for window..."
sleep "$SETTLE_TIME"

# Find the most recently mapped window (last in the list)
WID=$(xdotool search --onlyvisible --name "." 2>/dev/null | tail -1)

if [ -z "$WID" ]; then
    echo "ERROR: Could not find window to record"
    exit 1
fi

echo "Found window: $(xdotool getwindowname "$WID")"

# Get window geometry
eval $(xdotool getwindowgeometry --shell "$WID")

echo "Recording ${WIDTH}x${HEIGHT} at ($X,$Y) for ${DURATION}s..."

# Record raw video first
TMP_VID="/tmp/_slamd_rec.mp4"
ffmpeg -y -loglevel warning \
    -video_size "${WIDTH}x${HEIGHT}" -framerate "$FPS" \
    -f x11grab -i "${DISPLAY}+${X},${Y}" \
    -t "$DURATION" \
    -c:v libx264 -crf 18 -preset ultrafast \
    "$TMP_VID"

kill $PY_PID 2>/dev/null || true

# Generate palette
TMP_PAL="/tmp/_slamd_palette.png"
ffmpeg -y -loglevel warning -i "$TMP_VID" \
    -vf "fps=$FPS,scale=$GIF_WIDTH:-1:flags=lanczos,palettegen=max_colors=128:stats_mode=diff" \
    "$TMP_PAL"

# Convert to GIF using palette
ffmpeg -y -loglevel warning -i "$TMP_VID" -i "$TMP_PAL" \
    -lavfi "fps=$FPS,scale=$GIF_WIDTH:-1:flags=lanczos[x];[x][1:v]paletteuse=dither=bayer:bayer_scale=3" \
    "$OUTPUT"

rm -f "$TMP_VID" "$TMP_PAL"
echo "Saved to $OUTPUT"
