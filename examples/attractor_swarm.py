"""
Aizawa attractor — thousands of particles swirling through a strange attractor.

The Aizawa attractor produces a beautiful mushroom/tornado shape with particles
spiraling around a central axis, periodically collapsing inward and exploding
outward. With a swarm of particles you see the attractor's structure come alive.
"""

import slamd
import time
import numpy as np


def aizawa_deriv(states, a=0.95, b=0.7, c=0.6, d=3.5, e=0.25, f=0.1):
    """Vectorized Aizawa attractor derivatives for N particles."""
    x, y, z = states[:, 0], states[:, 1], states[:, 2]
    x2 = x * x
    y2 = y * y
    z2 = z * z
    z3 = z2 * z

    dx = (z - b) * x - d * y
    dy = d * x + (z - b) * y
    dz = c + a * z - z3 / 3.0 - (x2 + y2) * (1.0 + e * z) + f * z * x2 * x

    return np.column_stack([dx, dy, dz])


def rk4_step(states, dt):
    k1 = aizawa_deriv(states)
    k2 = aizawa_deriv(states + 0.5 * dt * k1)
    k3 = aizawa_deriv(states + 0.5 * dt * k2)
    k4 = aizawa_deriv(states + dt * k3)
    return states + (dt / 6.0) * (k1 + 2 * k2 + 2 * k3 + k4)


def main():
    vis = slamd.Visualizer("Aizawa Attractor", spawn=True, port=6002)
    scene = vis.scene("scene")

    n = 50000
    rng = np.random.default_rng(42)

    # Seed all particles near the attractor with small random offsets
    base = np.array([0.1, 0.0, 0.0])
    states = base + rng.normal(0, 0.05, (n, 3))

    # Warm up — let particles settle onto the attractor
    dt = 0.005
    for _ in range(200):
        states = rk4_step(states, dt)

    spheres = None
    scale = 3.0

    while True:
        for _ in range(3):
            states = rk4_step(states, dt)

        positions = (states * scale).astype(np.float32)

        # Color by angle around z-axis + height
        angle = np.arctan2(states[:, 1], states[:, 0])
        angle_norm = (angle + np.pi) / (2 * np.pi)
        z_norm = np.clip((states[:, 2] + 1.5) / 3.0, 0, 1)

        colors = np.column_stack([
            0.9 * (0.5 + 0.5 * np.sin(angle_norm * 6.28 + 0.0)),
            0.3 + 0.6 * z_norm,
            0.9 * (0.5 + 0.5 * np.sin(angle_norm * 6.28 + 2.5)),
        ]).astype(np.float32)

        radii = np.full(n, 0.025, dtype=np.float32)

        if spheres is None:
            spheres = slamd.geom.Spheres(positions, colors, radii, 0.15)
            scene.set_object("/swarm", spheres)
        else:
            spheres.update_positions(positions)
            spheres.update_colors(colors)

        time.sleep(0.01)


if __name__ == "__main__":
    main()
