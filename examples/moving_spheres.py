import slamd
import time
import numpy as np


def uniform_grid_points(n: int, a: float) -> np.ndarray:
    side = int(np.sqrt(n))
    x = np.linspace(-a, a, side)
    y = np.linspace(-a, a, side)
    xv, yv = np.meshgrid(x, y)
    grid = np.stack([xv.ravel(), yv.ravel()], axis=1)
    return grid[:n]


def main():
    vis = slamd.Visualizer("moving spheres", spawn=True, port=6001)
    scene = vis.scene("scene")

    coords = uniform_grid_points(100000, 30.0)
    dist = np.sqrt(coords[:, 0] ** 2 + coords[:, 1] ** 2)
    spheres = None

    t0 = time.monotonic()
    while True:
        time.sleep(0.01)
        t = time.monotonic() - t0

        # Calm concentric ripple
        z = 2.0 * np.sin(dist * 0.4 - t * 1.5) * np.exp(-dist * 0.03)

        points = np.column_stack([coords[:, 0], coords[:, 1], z]).astype(np.float32)

        # Smooth gradient: deep blue in troughs, warm orange on peaks
        h = (z - z.min()) / (z.max() - z.min() + 1e-8)
        colors = np.column_stack(
            [
                h * 1.0,
                h * 0.4 + 0.1,
                (1.0 - h) * 0.8 + 0.2,
            ]
        ).astype(np.float32)

        # Radii: gentle swell that follows the ripple outward
        radii = (0.15 + 0.08 * np.sin(dist * 0.4 - t * 1.5)).astype(np.float32)

        if spheres is None:
            spheres = slamd.geom.Spheres(points, colors, radii, 0.3)
            scene.set_object("/spheres", spheres)
        else:
            spheres.update_positions(points)
            spheres.update_colors(colors)
            spheres.update_radii(radii)


if __name__ == "__main__":
    main()
