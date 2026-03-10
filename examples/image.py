import slamd
import imageio.v3 as iio
from pathlib import Path


def main():
    vis = slamd.Visualizer("image")

    canvas = slamd.Canvas()

    image_path = Path(__file__).parent.parent / "images" / "logo.png"

    image = iio.imread(image_path)

    canvas.set_object("/image", slamd.geom2d.Image(image))

    vis.add_canvas("canvas", canvas)

    vis.hang_forever()


if __name__ == "__main__":
    main()
