import slamd
import numpy as np
import time


def main():
    vis = slamd.Visualizer("lots_of_paths")
    scene = vis.scene("boxes")

    num_boxes = 10000

    for i in range(num_boxes):
        random_loc = np.random.uniform(-100, 100, 3)

        transform = np.eye(4)
        transform[:3, 3] = random_loc

        pth = f"/box_bruh_stuff_long_ass_path_what_is_this_{i}"

        scene.set_transform(pth, transform)
        scene.set_object(pth, slamd.geom.Box(np.array([1.0, 1.0, 1.0], dtype=np.float32), np.array([0.8, 0.2, 0.0], dtype=np.float32)))
    time.sleep(10)


if __name__ == "__main__":
    main()
