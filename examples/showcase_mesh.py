"""
Showcase: the Stanford Bunny mesh with coordinate frames and annotations.

Download the data first:
    mkdir -p data
    curl -L -o data/bunny.tar.gz \
        "http://graphics.stanford.edu/pub/3Dscanrep/bunny.tar.gz"
    tar -xzf data/bunny.tar.gz -C data/
"""

import slamd
import numpy as np
from pathlib import Path


DATA_PATH = Path(__file__).parent.parent / "data" / "bunny" / "reconstruction" / "bun_zipper.ply"


def load_bunny_ply(path: Path):
    """Load the Stanford Bunny ASCII PLY (vertices + face indices)."""
    vertices = []
    faces = []

    with open(path, "r") as f:
        n_vertices = 0
        n_faces = 0
        in_header = True

        for line in f:
            line = line.strip()
            if in_header:
                if line.startswith("element vertex"):
                    n_vertices = int(line.split()[-1])
                elif line.startswith("element face"):
                    n_faces = int(line.split()[-1])
                elif line == "end_header":
                    in_header = False
                continue

            parts = line.split()
            if len(vertices) < n_vertices:
                vertices.append([float(parts[0]), float(parts[1]), float(parts[2])])
            else:
                n_idx = int(parts[0])
                face_indices = [int(parts[i + 1]) for i in range(n_idx)]
                if n_idx == 3:
                    faces.extend(face_indices)
                elif n_idx == 4:
                    # Triangulate quad
                    faces.extend([face_indices[0], face_indices[1], face_indices[2]])
                    faces.extend([face_indices[0], face_indices[2], face_indices[3]])

    vertices = np.array(vertices, dtype=np.float32)

    # Center and scale up (bunny is tiny by default)
    vertices -= vertices.mean(axis=0)
    scale = 10.0 / (vertices.max() - vertices.min())
    vertices *= scale

    return vertices, faces


def height_color(vertices: np.ndarray) -> np.ndarray:
    """Color vertices by height — warm tones on top, cool on bottom."""
    z = vertices[:, 2]
    t = (z - z.min()) / (z.max() - z.min())

    colors = np.zeros((len(vertices), 3), dtype=np.float32)
    # Bottom: teal (0.15, 0.4, 0.5)  ->  Top: warm sand (0.85, 0.7, 0.45)
    colors[:, 0] = 0.15 + 0.70 * t
    colors[:, 1] = 0.4 + 0.3 * t
    colors[:, 2] = 0.5 - 0.05 * t
    return colors


def main():
    if not DATA_PATH.exists():
        print(f"Data not found at {DATA_PATH}")
        print("Download it with:")
        print("  mkdir -p data")
        print(
            '  curl -L -o data/bunny.tar.gz '
            '"http://graphics.stanford.edu/pub/3Dscanrep/bunny.tar.gz"'
        )
        print("  tar -xzf data/bunny.tar.gz -C data/")
        return

    vis = slamd.Visualizer("Stanford Bunny")
    scene = vis.scene("scene")

    vertices, indices = load_bunny_ply(DATA_PATH)
    colors = height_color(vertices)

    scene.set_object("/bunny/mesh", slamd.geom.Mesh(vertices, colors, indices))

    # Ground plane under the bunny
    ground_y = vertices[:, 2].min()
    scene.set_object(
        "/ground",
        slamd.geom.Plane(
            np.array([0, 0, 1.0]),
            np.array([0, 0, ground_y - 0.05]),
            np.array([0.25, 0.25, 0.28]),
            8.0,
            0.4,
        ),
    )

    # Bounding box corners as small spheres
    mins = vertices.min(axis=0)
    maxs = vertices.max(axis=0)
    corners = np.array(
        [
            [mins[0], mins[1], mins[2]],
            [maxs[0], mins[1], mins[2]],
            [mins[0], maxs[1], mins[2]],
            [maxs[0], maxs[1], mins[2]],
            [mins[0], mins[1], maxs[2]],
            [maxs[0], mins[1], maxs[2]],
            [mins[0], maxs[1], maxs[2]],
            [maxs[0], maxs[1], maxs[2]],
        ],
        dtype=np.float32,
    )

    corner_colors = np.full((8, 3), [0.9, 0.5, 0.2], dtype=np.float32)
    corner_radii = np.full(8, 0.12, dtype=np.float32)
    scene.set_object(
        "/bbox/corners",
        slamd.geom.PointCloud(corners, corner_colors, corner_radii, 0.8),
    )

    # Bounding box edges
    edge_pairs = [
        (0, 1), (2, 3), (4, 5), (6, 7),  # x edges
        (0, 2), (1, 3), (4, 6), (5, 7),  # y edges
        (0, 4), (1, 5), (2, 6), (3, 7),  # z edges
    ]
    for a, b in edge_pairs:
        pts = np.array([corners[a], corners[b]], dtype=np.float32)
        scene.set_object(
            f"/bbox/edge_{a}_{b}",
            slamd.geom.PolyLine(pts, 0.02, np.array([0.6, 0.4, 0.2]), 0.5),
        )

    # Origin triad
    scene.set_object("/origin", slamd.geom.Triad(scale=1.5))

    vis.hang_forever()


if __name__ == "__main__":
    main()
