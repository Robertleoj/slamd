import slamd
import numpy as np


def main():
    vis = slamd.Visualizer(name="plane demo")

    scene = vis.scene("plane demo")

    num_planes = 50

    random_normals = np.random.randn(num_planes, 3) * 20
    random_positions = np.random.randn(num_planes, 3) * 20

    random_radii = np.random.uniform(5, 20, num_planes)
    random_colors = np.random.uniform(0, 1, (num_planes, 3))
    random_alphas = np.random.uniform(0, 1.0, num_planes)

    for i, (normal, pos, radius, color, alpha) in enumerate(
        zip(
            random_normals, random_positions, random_radii, random_colors, random_alphas
        )
    ):
        scene.set_object(
            f"/plane_{i}", slamd.geom.Plane(normal, pos, color, radius, alpha)
        )

    vis.hang_forever()


if __name__ == "__main__":
    main()
