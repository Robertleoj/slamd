import slamd
import numpy as np

if __name__ == "__main__":
    vis = slamd.Visualizer("two windows")

    scene1 = slamd.Scene()
    scene2 = slamd.Scene()

    vis.add_scene("scene 1", scene1)
    scene1.set_object("/box", slamd.geom.Box())

    vis.add_scene("scene 2", scene2)
    scene2.set_object("/origin", slamd.geom.Triad())

    scene2.set_object("/ball", slamd.geom.Sphere(2.0))

    sphere_transform = np.identity(4, dtype=np.float32)
    sphere_transform[:, 3] = np.array([5.0, 1.0, 2.0, 1.0])

    scene2.set_transform("/ball", sphere_transform)

    vis.hang_forever()
