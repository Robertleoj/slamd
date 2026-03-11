"""
Particle vortex — thousands of particles swirling around a central axis,
forming a tornado-like structure.

This is a fun animation demo, not representative of typical SlamDunk usage.
See the other examples for more practical use cases.
"""

import slamd
import numpy as np
import time


def main():
    vis = slamd.Visualizer("Particle Vortex")
    scene = vis.scene("scene")

    n = 8000
    rng = np.random.default_rng(7)

    # Each particle has: angle, radius, height, vertical speed, angular speed
    angles = rng.uniform(0, 2 * np.pi, n).astype(np.float32)
    radii = rng.exponential(3.0, n).astype(np.float32).clip(0.5, 12.0)
    heights = rng.uniform(-10, 10, n).astype(np.float32)
    v_speed = rng.normal(0, 0.3, n).astype(np.float32)
    a_speed = (1.5 / (radii + 0.5) + rng.normal(0, 0.1, n)).astype(np.float32)

    point_radii = (0.08 + 0.04 * np.exp(-radii / 4)).astype(np.float32)

    cloud = None

    t0 = time.monotonic()
    while True:
        t = time.monotonic() - t0

        # Particles spiral upward and wrap around
        heights += v_speed * 0.016
        heights = np.where(heights > 12, heights - 24, heights)
        heights = np.where(heights < -12, heights + 24, heights)

        current_angles = angles + a_speed * t

        # Slight radius pulsing
        r = radii * (1.0 + 0.15 * np.sin(t * 0.8 + heights * 0.3))

        x = r * np.cos(current_angles)
        y = r * np.sin(current_angles)
        z = heights

        positions = np.stack((x, y, z), axis=1).astype(np.float32)

        # Color by height + radius: inner/low = hot, outer/high = cool
        h_norm = (heights - heights.min()) / (heights.max() - heights.min() + 1e-8)
        r_norm = (radii - radii.min()) / (radii.max() - radii.min() + 1e-8)

        colors = np.zeros((n, 3), dtype=np.float32)
        # Inner particles: bright warm tones, outer: cooler blues/teals
        colors[:, 0] = np.clip(1.0 - r_norm * 1.2, 0.05, 1.0)  # red
        colors[:, 1] = np.clip(0.3 + 0.5 * h_norm - 0.2 * r_norm, 0.05, 0.9)  # green
        colors[:, 2] = np.clip(0.2 + r_norm * 0.8, 0.1, 1.0)  # blue

        if cloud is None:
            cloud = slamd.geom.PointCloud(positions, colors, point_radii, 0.3)
            scene.set_object("/vortex", cloud)
        else:
            cloud.update_positions(positions)
            cloud.update_colors(colors)

        time.sleep(0.016)


if __name__ == "__main__":
    main()
