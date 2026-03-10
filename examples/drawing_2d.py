import slamd
import numpy as np
import time


def cool_spiral(n: int, t: float) -> np.ndarray:
    t_arr = np.linspace(0, 10 * np.pi, n) + t
    r = np.linspace(0, 1, n)
    x = r * np.cos(t_arr)
    y = r * np.sin(t_arr)
    return np.stack((x, y), axis=1)


if __name__ == "__main__":
    vis = slamd.Visualizer("drawing_2d")

    canvas = slamd.Canvas()

    vis.add_canvas("canvas", canvas)

    t = 0.0
    while True:
        line_points = cool_spiral(10000, t)

        poly_line = slamd.geom2d.PolyLine(
            line_points,
            np.array([0.5, 0.5, 0.0]),
            0.1,
        )

        canvas.set_object("/line", poly_line)

        time.sleep(10 / 1000)
        t += 0.05
