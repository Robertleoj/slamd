"""Stress test for polyline rendering at sharp angles."""

import time

import slamd
import numpy as np

vis = slamd.Visualizer("Polyline stress test")
scene = vis.scene("scene")

x_offset = 0.0

# 1. Zigzag with increasing sharpness
points = []
for i in range(20):
    x = i * 0.5
    y = (1.0 if i % 2 == 0 else -1.0) * min(i * 0.3, 3.0)
    points.append([x + x_offset, y, 0.0])

scene.set_object(
    "/zigzag",
    slamd.geom.PolyLine(
        np.array(points, dtype=np.float32),
        0.15,
        np.array([1.0, 0.4, 0.1], dtype=np.float32),
        0.4,
    ),
)

x_offset += 12.0

# 2. 90-degree corners (staircase)
staircase = []
for i in range(8):
    staircase.append([x_offset + i, 0.0, float(i)])
    staircase.append([x_offset + i, 0.0, float(i + 1)])

scene.set_object(
    "/staircase",
    slamd.geom.PolyLine(
        np.array(staircase, dtype=np.float32),
        0.12,
        np.array([0.4, 1.0, 0.5], dtype=np.float32),
        0.4,
    ),
)

x_offset += 10.0

# 3. Hairpin / tight U-bend
hairpin = np.array(
    [
        [0, 0, 0],
        [0, 2, 0],
        [0.2, 2.3, 0],
        [0.4, 2.3, 0],
        [0.6, 2, 0],
        [0.6, 0, 0],
    ],
    dtype=np.float32,
)
hairpin[:, 0] += x_offset
scene.set_object(
    "/hairpin",
    slamd.geom.PolyLine(
        hairpin,
        0.15,
        np.array([1.0, 0.85, 0.1], dtype=np.float32),
        0.4,
    ),
)

x_offset += 3.0

# 4. Spiral tightening (decreasing radius)
t = np.linspace(0, 6 * np.pi, 200, dtype=np.float32)
r = 3.0 * np.exp(-0.1 * t)
spiral = np.column_stack([r * np.cos(t) + x_offset + 3, r * np.sin(t), np.zeros_like(t)])
scene.set_object(
    "/spiral",
    slamd.geom.PolyLine(
        spiral.astype(np.float32),
        0.08,
        np.array([0.6, 0.4, 1.0], dtype=np.float32),
        0.4,
    ),
)

x_offset += 10.0

# 5. Very short segments with sharp angles (worst case for overlap)
short_zigzag = []
for i in range(15):
    x = i * 0.2
    y = (0.3 if i % 2 == 0 else -0.3)
    short_zigzag.append([x + x_offset, y, 0.0])

scene.set_object(
    "/short_zigzag",
    slamd.geom.PolyLine(
        np.array(short_zigzag, dtype=np.float32),
        0.1,
        np.array([1.0, 0.3, 0.7], dtype=np.float32),
        0.4,
    ),
)

x_offset += 5.0

# 6. Near-180 degree reversal
reversal = np.array(
    [
        [0, 0, 0],
        [2, 0, 0],
        [2.05, 0.1, 0],
        [0, 0.2, 0],
    ],
    dtype=np.float32,
)
reversal[:, 0] += x_offset
scene.set_object(
    "/reversal",
    slamd.geom.PolyLine(
        reversal,
        0.1,
        np.array([0.2, 0.8, 0.8], dtype=np.float32),
        0.4,
    ),
)

x_offset += 5.0

# 7. Square (four 90-degree corners in sequence)
square = np.array(
    [
        [0, 0, 0],
        [2, 0, 0],
        [2, 2, 0],
        [0, 2, 0],
        [0, 0, 0],
    ],
    dtype=np.float32,
)
square[:, 0] += x_offset
scene.set_object(
    "/square",
    slamd.geom.PolyLine(
        square,
        0.12,
        np.array([1.0, 1.0, 1.0], dtype=np.float32),
        0.4,
    ),
)

x_offset += 5.0

# 8. Helix (smooth reference — should look perfect)
t = np.linspace(0, 8 * np.pi, 300, dtype=np.float32)
helix = np.column_stack([np.cos(t) + x_offset, np.sin(t), t * 0.1])
scene.set_object(
    "/helix",
    slamd.geom.PolyLine(
        helix.astype(np.float32),
        0.08,
        np.array([0.2, 0.7, 1.0], dtype=np.float32),
        0.4,
    ),
)

x_offset += 5.0

# 9. Random walk through uniformly sampled points in a box
rng = np.random.default_rng(42)
random_points = rng.uniform(-2.0, 2.0, size=(30, 3)).astype(np.float32)
random_points[:, 0] += x_offset + 2.0
scene.set_object(
    "/random_box",
    slamd.geom.PolyLine(
        random_points,
        0.08,
        np.array([1.0, 0.6, 0.2], dtype=np.float32),
        0.4,
    ),
)

time.sleep(10)
