"""Demo scene using boxes — an exploded Rubik's cube."""

import slamd
import numpy as np
from scipy.spatial.transform import Rotation


def make_transform(pos, rot=None):
    T = np.eye(4, dtype=np.float32)
    T[:3, 3] = pos
    if rot is not None:
        T[:3, :3] = rot
    return T


vis = slamd.Visualizer("Boxes")
scene = vis.scene("scene")

# Rubik's cube face colors (classic scheme)
face_colors = {
    ( 0,  0,  1): [1.0, 0.0, 0.0],  # front  = red
    ( 0,  0, -1): [1.0, 0.5, 0.0],  # back   = orange
    ( 0,  1,  0): [1.0, 1.0, 1.0],  # top    = white
    ( 0, -1,  0): [1.0, 1.0, 0.0],  # bottom = yellow
    ( 1,  0,  0): [0.0, 0.0, 0.8],  # right  = blue
    (-1,  0,  0): [0.0, 0.6, 0.0],  # left   = green
}

cube_size = 0.45
gap = 0.55
explode = 0.3  # how far apart the layers spread

rng = np.random.default_rng(42)

for x in range(-1, 2):
    for y in range(-1, 2):
        for z in range(-1, 2):
            pos = np.array([x, y, z], dtype=np.float32) * (gap + explode)

            # Pick color: outermost face determines it
            color = np.array([0.12, 0.12, 0.12], dtype=np.float32)  # interior = dark
            for axis, face_col in face_colors.items():
                ax = np.array(axis)
                coord = np.array([x, y, z])
                if np.dot(coord, ax) == 1:
                    color = np.array(face_col, dtype=np.float32)
                    break

            # Slight random tumble for the exploded feel
            tumble = Rotation.from_rotvec(
                rng.normal(0, 0.15, size=3)
            ).as_matrix().astype(np.float32)

            scene.set_object(
                f"/rubik/{x}_{y}_{z}",
                slamd.geom.Box(
                    np.array([cube_size] * 3, dtype=np.float32),
                    color,
                ),
            )
            scene.set_transform(
                f"/rubik/{x}_{y}_{z}",
                make_transform(pos, tumble),
            )

vis.hang_forever()
