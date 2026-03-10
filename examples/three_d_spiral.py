import slamd
import numpy as np
import time


def cool_spiral(n: int, t: float) -> np.ndarray:
    t_arr = np.linspace(0, 20 * np.pi, n) + t
    r = np.linspace(0, 10.0, n)
    x = r * np.cos(t_arr)
    y = r * np.sin(t_arr)

    z = np.cos(x) * np.sin(y) + r
    return np.stack((x, y, z), axis=1)


if __name__ == "__main__":
    vis = slamd.Visualizer("3d_spiral", True, 6000)

    scene = slamd.Scene()

    vis.add_scene("scene", scene)

    pink = np.array([212, 13, 125]) / 255

    t = 0.0
    while True:
        # make spiral
        coords = cool_spiral(1000, t)
        max_z = coords[:, 2].max()

        poly_line = slamd.geom.PolyLine(coords, 0.7, pink, 0.5)

        scene.set_object("/poly_line", poly_line)

        time.sleep(10 / 1000)
        t += 0.05
