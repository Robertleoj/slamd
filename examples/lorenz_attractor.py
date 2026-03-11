"""
Lorenz attractor — multiple trails tracing the strange attractor with color gradients.

This is a fun math demo, not representative of typical SlamDunk usage.
See the other examples for more practical use cases.
"""

import slamd
import numpy as np
import time


def lorenz_deriv(state, sigma=10.0, rho=28.0, beta=8 / 3):
    x, y, z = state[0], state[1], state[2]
    return np.array([sigma * (y - x), x * (rho - z) - y, x * y - beta * z])


def rk4_step(state, dt, sigma=10.0, rho=28.0, beta=8 / 3):
    k1 = lorenz_deriv(state, sigma, rho, beta)
    k2 = lorenz_deriv(state + 0.5 * dt * k1, sigma, rho, beta)
    k3 = lorenz_deriv(state + 0.5 * dt * k2, sigma, rho, beta)
    k4 = lorenz_deriv(state + dt * k3, sigma, rho, beta)
    return state + (dt / 6) * (k1 + 2 * k2 + 2 * k3 + k4)


TRAIL_COLORS = [
    (1.0, 0.3, 0.1),   # red-orange
    (0.2, 0.7, 1.0),   # sky blue
    (1.0, 0.85, 0.1),  # gold
    (0.4, 1.0, 0.5),   # mint green
    (1.0, 0.4, 0.7),   # pink
    (0.6, 0.4, 1.0),   # purple
]


def main():
    vis = slamd.Visualizer("Lorenz Attractor")
    scene = vis.scene("scene")

    n_trails = 6
    dt = 0.01

    # Known point on the attractor — no pre-run needed
    base = np.array([-6.2, -8.3, 22.0])
    states = [base + np.array([i * 0.05, 0, 0]) for i in range(n_trails)]

    max_len = 600
    trails: list[list[np.ndarray]] = [[] for _ in range(n_trails)]

    frame = 0
    while True:
        for _ in range(3):
            for i in range(n_trails):
                states[i] = rk4_step(states[i], dt)
                trails[i].append(states[i].astype(np.float32))
                if len(trails[i]) > max_len:
                    trails[i] = trails[i][-max_len:]

        frame += 1
        if frame % 2 != 0:
            time.sleep(0.01)
            continue

        for i in range(n_trails):
            if len(trails[i]) < 10:
                continue
            points = np.array(trails[i], dtype=np.float32)
            r, g, b = TRAIL_COLORS[i % len(TRAIL_COLORS)]
            color = np.array([r, g, b], dtype=np.float32)
            polyline = slamd.geom.PolyLine(points, 0.2, color, 0.5)
            scene.set_object(f"/trail_{i}", polyline)

        time.sleep(0.01)


if __name__ == "__main__":
    main()
