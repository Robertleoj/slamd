import slamd
import time
import numpy as np


def symmetric_root(x):
    # return np.sign(x) * (np.abs(x) ** 0.5)
    return np.abs(x) ** 0.5


def f(inp: np.ndarray, t: float) -> np.ndarray:
    x = inp[:, 0]
    y = inp[:, 1]

    return (
        np.sin(symmetric_root(10 * x) - t)
        * np.sin(symmetric_root(10 * y) + t)
        * symmetric_root(x * y)
    )


def uniform_grid_points_with_mesh(n: int, a: float) -> tuple[np.ndarray, list[int]]:
    side = int(np.sqrt(n))
    x = np.linspace(-a, a, side)
    y = np.linspace(-a, a, side)
    xv, yv = np.meshgrid(x, y)
    grid = np.stack([xv.ravel(), yv.ravel()], axis=1)

    # Build triangle indices
    indices = []
    for j in range(side - 1):
        for i in range(side - 1):
            top_left = j * side + i
            top_right = top_left + 1
            bottom_left = top_left + side
            bottom_right = bottom_left + 1

            # Triangle 1
            indices.extend([top_left, top_right, bottom_left])
            # Triangle 2
            indices.extend([top_right, bottom_right, bottom_left])

    return grid[:n], indices


def main():
    vis = slamd.Visualizer("hello python", port=6000)

    coords, indices = uniform_grid_points_with_mesh(100000, 30.0)

    scene = slamd.Scene()
    vis.add_scene("scene", scene)

    mesh = None

    t = 0
    while True:
        time.sleep(0.01)
        z = f(coords, t)
        points = np.concatenate((coords, z[:, None]), axis=1)

        red = np.exp(-points[:, 2])
        blue = 1.0 - red
        green = np.exp(2.0 - points[:, 2])

        colors = np.zeros(points.shape, dtype=np.float32)
        colors[:, 0] = red
        colors[:, 1] = blue
        colors[:, 2] = green

        if mesh is None:
            mesh = slamd.geom.Mesh(points, colors, indices)

            scene.set_object("/mesh", mesh)
        else:
            mesh.update_positions(points)
            mesh.update_colors(colors)

        t += 0.02


if __name__ == "__main__":
    main()
