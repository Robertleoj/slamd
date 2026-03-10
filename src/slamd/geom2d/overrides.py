import numpy as np

from .._utils.colors import Color
from ..bindings.geom2d import (
    Points as Points_internal,
    PolyLine as PolyLine_internal,
    Circles as Circles_internal,
)

from .._utils.handle_input import process_color, process_radii, process_single_color


def Points(
    positions: np.ndarray,
    colors: np.ndarray | tuple[int, int, int] = Color.black,
    radii: np.ndarray | float = 1.0,
):
    """A set of 2D points.

    Args:
        positions: a N x 2 array of the 2D point centers.
        colors: The color of the points. Can be one of
            - array of shape N x 3 of RGB colors in (0, 1)
            - array of shape 3 with a single RGB color in (0, 1)
            - tuple an RGB value, 0-255
        radii: The radius of each point. Can be one of
            - array of shape N with a radius for each point
            - a single float specifying the radius of all the points.
    """
    n = positions.shape[0]
    colors_np = process_color(colors, n)
    radii_np = process_radii(radii, n)

    return Points_internal(positions, colors_np, radii_np)


def PolyLine(
    points: np.ndarray,
    color: np.ndarray | tuple[int, int, int] = Color.pink,
    thickness: float = 1.0,
):
    """A piecewise-linear line.

    Args:
        points: The points which the piecewise-linear line goes through.
        color: Either a numpy array with values in (0, 1), or an RGB tuple (0-255)
        thickness: The thickness of the line.
    """
    return PolyLine_internal(points, process_single_color(color), thickness)


def Circles(
    positions: np.ndarray,
    colors: np.ndarray | tuple[int, int, int] = Color.dark_blue,
    radii: np.ndarray | float = 1.0,
    thickness: float = 0.1,
):
    """A set of hollow circles.

    Args:
        positions: a N x 2 array of the 2D point centers.
        colors: The color of the points. Can be one of
            - array of shape N x 3 of RGB colors in (0, 1)
            - array of shape 3 with a single RGB color in (0, 1)
            - tuple an RGB value, 0-255
        radii: The radius of each point. Can be one of
            - array of shape N with a radius for each point
            - a single float specifying the radius of all the points.
        thickness: Thickness of the circle as a proportion of the radius.
    """
    n = positions.shape[0]

    return Circles_internal(
        positions, process_color(colors, n), process_radii(radii, n), thickness
    )
