from __future__ import annotations
import bindings._geom
import numpy
import typing
__all__ = ['Circles', 'Image', 'Points', 'PolyLine']
def Circles(positions: numpy.ndarray, colors: numpy.ndarray, radii: list[float] | numpy.ndarray, thickness: typing.SupportsFloat | typing.SupportsIndex = 0.1) -> bindings._geom.Circles2D:
    """
    Create a set of circles
    """
def Image(image: numpy.ndarray) -> bindings._geom.Image:
    """
    Create an Image geometry from a NumPy uint8 array (H, W, C)
    """
def Points(positions: numpy.ndarray, colors: numpy.ndarray, radii: list[float] | numpy.ndarray) -> bindings._geom.Points2D:
    """
    Create 2D points with per-point color and radius
    """
def PolyLine(points: numpy.ndarray, color: numpy.ndarray, thickness: typing.SupportsFloat | typing.SupportsIndex) -> bindings._geom.PolyLine2D:
    """
    Create a 2D poly line
    """
