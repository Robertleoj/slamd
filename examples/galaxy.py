"""
Spiral galaxy — 150k stars in a rotating galaxy with arms, bulge, and dust.
"""

import slamd
import numpy as np


def make_galaxy(n: int, rng: np.random.Generator) -> tuple[np.ndarray, np.ndarray]:
    n_bulge = n // 5
    n_disk = n - n_bulge

    # Spiral disk
    r = rng.exponential(3.0, n_disk)

    # Add spiral arm structure — stars cluster around 3 arms
    n_arms = 3
    arm = rng.integers(0, n_arms, n_disk)
    arm_offset = arm * (2 * np.pi / n_arms)
    spiral_angle = arm_offset + r * 0.6 + rng.normal(0, 0.3 / (1 + r * 0.1), n_disk)

    x = r * np.cos(spiral_angle)
    y = r * np.sin(spiral_angle)
    z = rng.normal(0, 0.1 + 0.02 * r, n_disk)

    disk_pos = np.column_stack([x, y, z])

    # Hot blue-white in arms, redder further out
    arm_phase = np.cos(spiral_angle * n_arms - r * 0.6 * n_arms)
    brightness = np.clip(0.5 + 0.5 * arm_phase, 0.2, 1.0)
    temp = np.clip(1.0 - r / 15.0, 0.0, 1.0)

    disk_colors = np.column_stack(
        [
            0.9 * brightness + 0.1,
            0.7 * brightness * temp + 0.3 * brightness,
            0.9 * brightness * temp,
        ]
    )

    # Central bulge
    br = rng.exponential(0.8, n_bulge)
    btheta = rng.uniform(0, 2 * np.pi, n_bulge)
    bphi = rng.normal(0, 0.5, n_bulge)

    bx = br * np.cos(btheta) * np.cos(bphi)
    by = br * np.sin(btheta) * np.cos(bphi)
    bz = br * np.sin(bphi) * 0.4

    bulge_pos = np.column_stack([bx, by, bz])

    bulge_brightness = np.clip(1.0 - br / 3.0, 0.3, 1.0)
    bulge_colors = np.column_stack(
        [
            bulge_brightness,
            bulge_brightness * 0.85,
            bulge_brightness * 0.5,
        ]
    )

    positions = np.concatenate([disk_pos, bulge_pos]).astype(np.float32)
    colors = np.concatenate([disk_colors, bulge_colors]).astype(np.float32)

    return positions, colors


def main():
    vis = slamd.Visualizer("Galaxy")
    scene = vis.scene("scene")

    n = 1_000_000
    rng = np.random.default_rng(42)
    positions, colors = make_galaxy(n, rng)

    cloud = slamd.geom.PointCloud(positions, colors, 0.08, 0.7)
    scene.set_object("/galaxy", cloud)


if __name__ == "__main__":
    main()
