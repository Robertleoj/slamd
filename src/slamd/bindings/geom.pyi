from __future__ import annotations
import collections.abc
import numpy
import slamd.bindings._geom_types
import typing

__all__: list[str] = [
    "Arrows",
    "Box",
    "CameraFrustum",
    "Mesh",
    "Plane",
    "PointCloud",
    "PolyLine",
    "Sphere",
    "Spheres",
    "Triad",
]

def Arrows(
    starts: numpy.ndarray,
    ends: numpy.ndarray,
    colors: numpy.ndarray,
    thickness: typing.SupportsFloat | typing.SupportsIndex,
) -> slamd.bindings._geom_types.Arrows:
    """
    Create an Arrows geometry
    """

def Box(
    dims: numpy.ndarray = ..., color: numpy.ndarray = ...
) -> slamd.bindings._geom_types.Box:
    """
    Create a Box geometry
    """

def CameraFrustum(
    intrinsics_matrix: numpy.ndarray,
    image_width: typing.SupportsInt | typing.SupportsIndex,
    image_height: typing.SupportsInt | typing.SupportsIndex,
    image: numpy.ndarray | None = None,
    scale: typing.SupportsFloat | typing.SupportsIndex = 1.0,
) -> slamd.bindings._geom_types.CameraFrustum:
    """
    Create a CameraFrustum geometry
    """

@typing.overload
def Mesh(
    vertices: numpy.ndarray,
    vertex_colors: numpy.ndarray,
    triangle_indices: collections.abc.Sequence[
        typing.SupportsInt | typing.SupportsIndex
    ],
) -> slamd.bindings._geom_types.Mesh:
    """
    Create a SimpleMesh geometry from raw data
    """

@typing.overload
def Mesh(
    vertices: numpy.ndarray,
    vertex_colors: numpy.ndarray,
    triangle_indices: collections.abc.Sequence[
        typing.SupportsInt | typing.SupportsIndex
    ],
    vertex_normals: numpy.ndarray,
) -> slamd.bindings._geom_types.Mesh:
    """
    Create a SimpleMesh geometry from raw data
    """

@typing.overload
def Mesh(
    vertices: numpy.ndarray,
    vertex_colors: numpy.ndarray,
    triangle_indices: collections.abc.Sequence[
        typing.SupportsInt | typing.SupportsIndex
    ],
    vertex_normals: numpy.ndarray,
    alpha: typing.SupportsFloat | typing.SupportsIndex,
) -> slamd.bindings._geom_types.Mesh:
    """
    Create a SimpleMesh geometry from raw data with transparency
    """

def Plane(
    normal: numpy.ndarray,
    point: numpy.ndarray,
    color: numpy.ndarray,
    radius: typing.SupportsFloat | typing.SupportsIndex,
    alpha: typing.SupportsFloat | typing.SupportsIndex,
) -> slamd.bindings._geom_types.Plane:
    """
    Create a Plane geometry
    """

def PointCloud(
    positions: numpy.ndarray,
    colors: numpy.ndarray,
    radii: list[float] | numpy.ndarray,
    min_brightness: typing.SupportsFloat | typing.SupportsIndex = 1.0,
) -> slamd.bindings._geom_types.PointCloud:
    """
    Create a PointCloud with per-point color and radius
    """

def PolyLine(
    points: numpy.ndarray,
    thickness: typing.SupportsFloat | typing.SupportsIndex,
    color: numpy.ndarray,
    min_brightness: typing.SupportsFloat | typing.SupportsIndex,
) -> slamd.bindings._geom_types.PolyLine:
    """
    Create a PolyLine geometry
    """

def Sphere(
    radius: typing.SupportsFloat | typing.SupportsIndex = 1.0,
    color: numpy.ndarray = ...,
) -> slamd.bindings._geom_types.Sphere:
    """
    Create a Sphere geometry
    """

def Spheres(
    positions: numpy.ndarray,
    colors: numpy.ndarray,
    radii: list[float] | numpy.ndarray,
    min_brightness: typing.SupportsFloat | typing.SupportsIndex = 0.30000001192092896,
) -> slamd.bindings._geom_types.Spheres:
    """
    Create Spheres with per-point color and radius
    """

def Triad(
    pose: numpy.ndarray | None = None,
    scale: typing.SupportsFloat | typing.SupportsIndex = 1.0,
    thickness: typing.SupportsFloat | typing.SupportsIndex = 0.10000000149011612,
) -> slamd.bindings._geom_types.Triad:
    """
    Create a Triad geometry
    """
