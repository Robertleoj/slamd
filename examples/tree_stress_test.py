"""Stress test for the tree overlay: multiple views, deep/wide trees, long names."""

import slamd
import numpy as np
import time


def make_transform(x: float, y: float, z: float) -> np.ndarray:
    T = np.eye(4, dtype=np.float32)
    T[0, 3] = x
    T[1, 3] = y
    T[2, 3] = z
    return T


def populate_scene_deep(scene: slamd.Scene):
    """Deep tree with varied name lengths."""
    # A deeply nested chain
    for depth in range(8):
        prefix = "/deep_hierarchy"
        for d in range(depth + 1):
            if d % 2 == 0:
                prefix += f"/level_{d}"
            else:
                prefix += f"/this_is_a_deliberately_long_name_at_level_{d}"

        scene.set_object(
            prefix + "/triad",
            slamd.geom.Triad(make_transform(depth * 3.0, 0, 0)),
        )

    # Wide tree at one level
    for i in range(30):
        if i % 3 == 0:
            name = f"/objects/short_{i}"
        elif i % 3 == 1:
            name = f"/objects/medium_length_object_number_{i}"
        else:
            name = f"/objects/this_is_object_with_a_really_long_descriptive_name_{i}"

        scene.set_object(name, slamd.geom.Sphere(0.5))
        scene.set_transform(name, make_transform(i * 2.0, 5.0, 0))

    # Several mid-depth branches
    categories = ["sensors", "actuators", "reference_frames", "debug_vis"]
    for cat in categories:
        for j in range(5):
            path = f"/{cat}/unit_{j}/geometry"
            scene.set_object(path, slamd.geom.Box(np.array([1.5, 1.5, 1.5], dtype=np.float32), np.array([0.3, 0.6, 0.9], dtype=np.float32)))
            scene.set_transform(
                path,
                make_transform(j * 2.0, -5.0 - categories.index(cat) * 3.0, 0),
            )


def populate_scene_wide(scene: slamd.Scene):
    """Very wide tree with many siblings."""
    # Flat list of many objects
    for i in range(50):
        scene.set_object(
            f"/item_{i:03d}",
            slamd.geom.Triad(
                make_transform(
                    (i % 10) * 3.0,
                    (i // 10) * 3.0,
                    0,
                )
            ),
        )

    # A few nested groups
    for g in range(5):
        for sub in range(10):
            path = f"/group_{g}/subgroup_with_longer_name_{sub}/sphere"
            scene.set_object(path, slamd.geom.Sphere(0.3))
            scene.set_transform(path, make_transform(g * 5.0, sub * 2.0, 5.0))


def main():
    vis = slamd.Visualizer("tree overlay stress test")

    scene1 = vis.scene("deep tree")
    scene2 = vis.scene("wide tree")

    populate_scene_deep(scene1)
    populate_scene_wide(scene2)
    time.sleep(10)


if __name__ == "__main__":
    main()
