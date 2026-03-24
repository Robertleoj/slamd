import numpy as np
from ..bindings.geom import (
    Box as Box_internal,
    CameraFrustum as CameraFrustum_internal,
    Mesh as Mesh_internal,
    PointCloud as PointCloud_internal,
    Spheres as Spheres_internal,
    PolyLine as PolyLine_internal,
    Sphere as Sphere_internal,
    Arrows as Arrows_internal,
    Plane as Plane_internal,
    Triad as Triad_internal,
)
from .._utils.colors import Color
from .._utils.handle_input import process_color, process_radii, process_single_color


def Box(
    dims: np.ndarray,
    color: np.ndarray | tuple[int, int, int] = Color.orange,
):
    """An axis-aligned box, centered at the node transform.

    Args:
        dims: (3,) float32 array, dimensions along x, y, z.
        color: Either a (3,) float32 array with RGB in (0, 1), or an RGB tuple (0-255).
    """
    return Box_internal(dims, process_single_color(color))


def CameraFrustum(
    intrinsics: np.ndarray,
    image_width: int,
    image_height: int,
    image: np.ndarray | None = None,
    scale: float = 1.0,
):
    """A camera frustum wireframe for visualizing camera poses.

    Args:
        intrinsics: (3, 3) float32 camera intrinsics matrix.
        image_width: Image width in pixels.
        image_height: Image height in pixels.
        image: Optional (H, W, 3) uint8 RGB image to display on the frustum.
        scale: Size of the rendered frustum.
    """
    return CameraFrustum_internal(intrinsics, image_width, image_height, image, scale)


def Mesh(
    positions: np.ndarray,
    colors: np.ndarray,
    indices: np.ndarray,
    normals: np.ndarray | None = None,
    alpha: float = 1.0,
):
    """A triangle mesh.

    Args:
        positions: (N, 3) float32 vertex positions.
        colors: (N, 3) float32 per-vertex RGB colors in (0, 1).
        indices: (M,) uint32 triangle indices (M must be a multiple of 3).
        normals: Optional (N, 3) float32 per-vertex normals. Auto-computed if omitted.
        alpha: Opacity, 0.0 (transparent) to 1.0 (opaque).
    """
    if normals is None:
        return Mesh_internal(positions, colors, indices)
    if alpha == 1.0:
        return Mesh_internal(positions, colors, indices, normals)
    return Mesh_internal(positions, colors, indices, normals, alpha)


def PointCloud(
    positions: np.ndarray,
    colors: np.ndarray | tuple[int, int, int] = Color.black,
    radii: np.ndarray | float = 1.0,
    min_brightness: float = 1.0,
):
    """A 3D point cloud.

    Args:
        positions: (N, 3) float32 point positions.
        colors: Per-point or uniform color:
            - (N, 3) float32 RGB in (0, 1)
            - (3,) float32 single RGB
            - RGB tuple (0-255)
        radii: Per-point or uniform radius:
            - (N,) float32 array
            - single float
        min_brightness: Minimum brightness floor.
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
    """A 3D polyline tube through a sequence of points.

    Args:
        points: (N, 3) float32 positions the line passes through.
        thickness: Tube diameter.
        color: (3,) float32 RGB in (0, 1), or RGB tuple (0-255).
        min_brightness: Minimum brightness floor.
    """
    color_np = process_single_color(color)
    return PolyLine_internal(points, thickness, color_np, min_brightness)


def Sphere(radius: float, color: np.ndarray | tuple[int, int, int] = Color.blue):
    """A solid 3D sphere.

    Args:
        radius: Radius of the sphere.
        color: (3,) float32 RGB in (0, 1), or RGB tuple (0-255).
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
        starts: (N, 3) float32 arrow start positions.
        ends: (N, 3) float32 arrow end positions.
        colors: Per-arrow or uniform color:
            - (N, 3) float32 RGB in (0, 1)
            - (3,) float32 single RGB
            - RGB tuple (0-255)
        thickness: Arrow shaft thickness.
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
    """A flat circular disc in 3D.

    Args:
        normal: (3,) float32 plane normal direction.
        point: (3,) float32 disc center position.
        color: (3,) float32 RGB in (0, 1), or RGB tuple (0-255).
        radius: Disc radius.
        alpha: Opacity, 0.0 (transparent) to 1.0 (opaque).
    """
    return Plane_internal(normal, point, process_single_color(color), radius, alpha)


def Triad(pose: np.ndarray | None = None, scale: float = 1.0, thickness: float = 0.1):
    """An RGB axis triad (X=red, Y=green, Z=blue).

    Args:
        pose: Optional 4x4 float32 homogeneous transform matrix. If None, uses identity.
        scale: Length of each axis arrow.
        thickness: Thickness of the arrow shafts.
    """
    return Triad_internal(pose, scale, thickness)


def Spheres(
    positions: np.ndarray,
    colors: np.ndarray | tuple[int, int, int] = Color.black,
    radii: np.ndarray | float = 1.0,
    min_brightness: float = 0.3,
):
    """A collection of 3D spheres.

    Args:
        positions: (N, 3) float32 sphere center positions.
        colors: Per-sphere or uniform color:
            - (N, 3) float32 RGB in (0, 1)
            - (3,) float32 single RGB
            - RGB tuple (0-255)
        radii: Per-sphere or uniform radius:
            - (N,) float32 array
            - single float
        min_brightness: Minimum brightness floor.
    """
    n = positions.shape[0]
    colors_np = process_color(colors, n)
    radii_np = process_radii(radii, n)

    return Spheres_internal(positions, colors_np, radii_np, min_brightness)
