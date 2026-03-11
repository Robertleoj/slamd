import slamd
import numpy as np
import time


def main():
    vis = slamd.Visualizer("🌀 Slamd Circle Rave — Juiced Edition 🌀")
    canvas = slamd.Canvas()
    vis.add_canvas("canvas", canvas)

    # Fewer, beefier circles
    n = 500
    positions = np.random.uniform(-500.0, 500.0, (n, 2))
    velocities = np.random.uniform(-50.0, 50.0, (n, 2))
    colors = np.zeros((n, 3))
    radii = np.random.uniform(20.0, 40.0, n)  # more thicc
    base_radii = radii.copy()
    thickness = 0.1

    circles = slamd.geom2d.Circles(positions, colors, radii, thickness)
    canvas.set_object("/circles", circles)

    center = np.array([0.0, 0.0])
    last_time = time.monotonic()

    while True:
        now = time.monotonic()
        dt = now - last_time
        last_time = now
        t = now

        # Orbit swirl
        direction_to_center = center - positions
        distance = np.linalg.norm(direction_to_center, axis=1, keepdims=True) + 1e-6
        normalized = direction_to_center / distance
        perp = np.stack([-normalized[:, 1], normalized[:, 0]], axis=1)
        velocities += 0.5 * perp * (dt / 0.016)
        velocities *= 0.99
        positions += velocities * dt

        # RGB cyclone
        colors = 0.5 + 0.5 * np.stack(
            [
                np.sin(0.001 * positions[:, 0] + t),
                np.sin(0.001 * positions[:, 1] + t + 2.1),
                np.sin(0.001 * (positions[:, 0] + positions[:, 1]) + t + 4.2),
            ],
            axis=1,
        )

        # Pulse bigger bois
        radii = base_radii * (1.0 + 0.4 * np.sin(t * 2 + positions[:, 0] * 0.01))

        circles.update_positions(positions)
        circles.update_colors(colors)
        circles.update_radii(radii)

        time.sleep(0.016)


if __name__ == "__main__":
    main()
