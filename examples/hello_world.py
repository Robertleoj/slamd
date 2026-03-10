import slamd

if __name__ == "__main__":
    vis = slamd.Visualizer("Hello world")

    scene = vis.scene("scene")

    scene.set_object("/origin", slamd.geom.Triad())

    vis.hang_forever()
