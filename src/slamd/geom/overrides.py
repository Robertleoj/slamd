import numpy as np
from ..bindings.geom import (
    PointCloud as PointCloud_internal,
    PolyLine as PolyLine_internal,
    Sphere as Sphere_internal,
    Arrows as Arrows_internal,
    Plane as Plane_internal,
    Triad as Triad_internal,
)
from .._utils.colors import Color
from .._utils.handle_input import process_color, process_radii, process_single_color


def PointCloud(
    positions: np.ndarray,
    colors: np.ndarray | tuple[int, int, int] = Color.black,
    radii: np.ndarray | float = 1.0,
    min_brightness: float = 1.0,
):
    """A 3D point cloud.

    Args:
        positions: An N x 3 array of the 3D point positions.
        colors: The color of the points. Can be one of:
            - array of shape N x 3 of RGB colors in (0, 1)
            - array of shape 3 with a single RGB color in (0, 1)
            - tuple of an RGB value, 0–255
        radii: The radius of each point. Can be:
            - array of shape N with a radius per point
            - single float for uniform radius
        min_brightness: Minimum brightness applied to the points.
    """
    n = positions.shape[0]
    colors_np = process_color(colors, n)
    radii_np = process_radii(radii, n)

    return PointCloud_internal(positions, colors_np, radii_np, min_brightness)


def PolyLine(
    points: np.ndarray,
    thickness: float = 1.0,
    color: np.ndarray | tuple[int, int, int] = Color.red,
    min_brightness: float = 1.0,
):
    """A 3D polyline made of straight segments.

    Args:
        points: An N x 3 array of points the polyline passes through.
        thickness: Thickness of the line.
        color: Either a numpy array with values in (0, 1), or an RGB tuple (0–255).
        min_brightness: Minimum brightness applied to the line.
    """
    color_np = process_single_color(color)
    return PolyLine_internal(points, thickness, color_np, min_brightness)


def Sphere(radius: float, color: np.ndarray | tuple[int, int, int] = Color.blue):
    """A solid 3D sphere.

    Args:
        radius: Radius of the sphere.
        color: Either a numpy array with values in (0, 1), or an RGB tuple (0–255).
    """
    return Sphere_internal(radius, process_single_color(color))


def Arrows(
    starts: np.ndarray,
    ends: np.ndarray,
    colors: np.ndarray | tuple[int, int, int] = Color.dark_red,
    thickness: float = 0.5,
):
    """A collection of 3D arrows from start to end points.

    Args:
        starts: N x 3 array of arrow starting points.
        ends: N x 3 array of arrow end points.
        colors: The color of the arrows. Can be:
            - array of shape N x 3 of RGB colors in (0, 1)
            - array of shape 3 with a single RGB color in (0, 1)
            - tuple of an RGB value, 0–255
        thickness: The thickness of the arrow shafts.
    """

    return Arrows_internal(
        starts, ends, process_color(colors, starts.shape[0]), thickness
    )


def Plane(
    normal: np.ndarray,
    point: np.ndarray,
    color: np.ndarray | tuple[int, int, int] = Color.blue,
    radius: float = 1.0,
    alpha: float = 0.8,
):
    return Plane_internal(normal, point, process_single_color(color), radius, alpha)


def Triad(pose: np.ndarray | None = None, scale: float = 1.0, thickness: float = 1.0):
    return Triad_internal(pose, scale, thickness)
