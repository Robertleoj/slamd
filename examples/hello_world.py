import slamd
import time

if __name__ == "__main__":
    vis = slamd.Visualizer("Hello world")

    scene = vis.scene("scene")

    scene.set_object("/origin", slamd.geom.Triad())

    # let the visualizer connect and sync state
    time.sleep(0.1)
