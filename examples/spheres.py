import slamd
import numpy as np


def main():
    vis = slamd.Visualizer("spheres example", spawn=True, port=6006)
    scene = vis.scene("scene")

    n = 500
    rng = np.random.default_rng(42)

    # Random positions on a sphere surface
    phi = rng.uniform(0, 2 * np.pi, n)
    cos_theta = rng.uniform(-1, 1, n)
    theta = np.arccos(cos_theta)
    r = 5.0

    positions = np.column_stack(
        [
            r * np.sin(theta) * np.cos(phi),
            r * np.sin(theta) * np.sin(phi),
            r * np.cos(theta),
        ]
    ).astype(np.float32)

    # Color by height
    height_norm = (positions[:, 2] - positions[:, 2].min()) / (
        positions[:, 2].max() - positions[:, 2].min()
    )
    colors = np.column_stack(
        [
            height_norm,
            0.3 * np.ones(n),
            1.0 - height_norm,
        ]
    ).astype(np.float32)

    radii = rng.uniform(0.1, 0.4, n).astype(np.float32)

    spheres = slamd.geom.Spheres(positions, colors, radii, 0.3)
    scene.set_object("/spheres", spheres)


if __name__ == "__main__":
    main()
