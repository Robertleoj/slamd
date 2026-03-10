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
        np.sin(symmetric_root(10 * x) + t)
        * np.sin(symmetric_root(10 * y) + t)
        * (symmetric_root(x * y))
    )


def uniform_grid_points(n: int, a: float) -> np.ndarray:
    side = int(np.sqrt(n))
    x = np.linspace(-a, a, side)
    y = np.linspace(-a, a, side)
    xv, yv = np.meshgrid(x, y)
    grid = np.stack([xv.ravel(), yv.ravel()], axis=1)
    return grid[:n]


def main():
    vis = slamd.Visualizer("hello python", spawn=True, port=6000)

    # coords = uniform_grid_points(100000, 10.0)
    coords = uniform_grid_points(100000, 30.0)
    print(uniform_grid_points)

    scene = vis.scene("scene")

    point_cloud = None

    t = 0
    while True:
        time.sleep(0.01)
        z = f(coords, t)
        points = np.concatenate((coords, z[:, None]), axis=1)

        red = np.exp(-points[:, 2])
        blue = 1.0 - red
        green = 0.5

        colors = np.zeros(points.shape, dtype=np.float32)
        colors[:, 0] = red
        colors[:, 1] = blue
        colors[:, 2] = green

        if point_cloud is None:
            point_cloud = slamd.geom.PointCloud(points, colors, 0.3, 0.5)

            scene.set_object("/points", point_cloud)
        else:
            point_cloud.update_positions(points)
            point_cloud.update_colors(colors)

        t += 0.02


if __name__ == "__main__":
    main()
