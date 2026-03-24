import slamd
import numpy as np
import time


def main():
    vis = slamd.Visualizer("large scale stress test")
    scene = vis.scene("scene")

    # Objects at scales of hundreds of thousands to millions
    # Simulates e.g. millimeter-unit coordinate systems
    scale = 500_000.0
    triad_size = scale * 0.1

    # Place triads in a grid pattern at large scale
    for i, x in enumerate(np.linspace(-scale, scale, 5)):
        for j, y in enumerate(np.linspace(-scale, scale, 5)):
            pose = np.eye(4)
            pose[0, 3] = x
            pose[1, 3] = y
            scene.set_object(
                f"/triad_{i}_{j}",
                slamd.geom.Triad(pose, triad_size),
            )

    # A big point cloud spread across the large space
    n_points = 1000
    points = np.random.uniform(-scale, scale, size=(n_points, 3))
    points[:, 2] = np.abs(points[:, 2]) * 0.1  # flatten Z a bit
    colors = np.random.uniform(0.2, 1.0, size=(n_points, 3))
    scene.set_object(
        "/point_cloud",
        slamd.geom.PointCloud(points, colors, triad_size * 0.2),
    )
    time.sleep(10)


if __name__ == "__main__":
    main()
