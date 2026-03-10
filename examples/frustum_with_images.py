import slamd
import numpy as np
from PIL import Image, ImageDraw
from scipy.spatial.transform import Rotation
import time


def get_camera_intrinsics_and_image():
    width, height = 1280, 720
    fx = fy = 1000
    cx, cy = width / 2, height / 2

    K = np.array([[fx, 0, cx], [0, fy, cy], [0, 0, 1]])

    image = Image.new("RGB", (width, height), (120, 200, 255))
    draw = ImageDraw.Draw(image)

    draw.polygon(
        [
            (width // 2 - 100, height),
            (width // 2 + 100, height),
            (width // 2 + 20, height // 2),
            (width // 2 - 20, height // 2),
        ],
        fill=(50, 50, 50),
    )

    for x in range(100, width, 200):
        draw.rectangle([x, height // 2 - 80, x + 20, height // 2], fill=(60, 40, 20))
        draw.ellipse(
            [x - 30, height // 2 - 130, x + 50, height // 2 - 30], fill=(34, 139, 34)
        )

    draw.ellipse([width - 100, 50, width - 20, 130], fill=(255, 255, 0))

    return K, width, height, np.array(image)


if __name__ == "__main__":
    vis = slamd.Visualizer("camera and image")

    scene = slamd.Scene()
    scene.set_object("/origin", slamd.geom.Triad())

    vis.add_scene("scene", scene)

    K, width, height, img = get_camera_intrinsics_and_image()

    frustum = slamd.geom.CameraFrustum(K, width, height, img, 1.0)

    scene.set_object("/rot1/tr/rot2/cam/triad", slamd.geom.Triad(0.5))
    scene.set_object("/rot1/tr/rot2/cam/frustum", frustum)

    rot2 = np.eye(4)
    rot2[:3, :3] = Rotation.from_euler("x", -90.0, degrees=True).as_matrix()

    scene.set_transform("/rot1/tr/rot2", rot2)

    tr = np.eye(4)
    tr[1, 3] = -5.0

    scene.set_transform("/rot1/tr", tr)

    t = 0.0
    while True:
        rot1 = np.eye(4)
        rot1[:3, :3] = Rotation.from_euler("z", t).as_matrix()
        scene.set_transform("/rot1", rot1)
        t += 0.01
        time.sleep(10 / 1000)
