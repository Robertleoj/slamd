from __future__ import annotations
import numpy
import slamd.bindings._geom
import typing
__all__: list[str] = ['Circles', 'Image', 'Points', 'PolyLine']
def Circles(positions: numpy.ndarray, colors: numpy.ndarray, radii: list[float] | numpy.ndarray, thickness: typing.SupportsFloat | typing.SupportsIndex = 0.1) -> slamd.bindings._geom.Circles2D:
    """
    Create a set of circles
    """
def Image(image: numpy.ndarray) -> slamd.bindings._geom.Image:
    """
    Create an Image geometry from a NumPy uint8 array (H, W, C)
    """
def Points(positions: numpy.ndarray, colors: numpy.ndarray, radii: list[float] | numpy.ndarray) -> slamd.bindings._geom.Points2D:
    """
    Create 2D points with per-point color and radius
    """
def PolyLine(points: numpy.ndarray, color: numpy.ndarray, thickness: typing.SupportsFloat | typing.SupportsIndex) -> slamd.bindings._geom.PolyLine2D:
    """
    Create a 2D poly line
    """
