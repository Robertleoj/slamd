from __future__ import annotations
import numpy
import typing
__all__: list[str] = ['Arrows', 'Box', 'CameraFrustum', 'Circles2D', 'Geometry', 'Image', 'Mesh', 'Plane', 'PointCloud', 'Points2D', 'PolyLine', 'PolyLine2D', 'Sphere', 'Spheres', 'Triad']
class Arrows(Geometry):
    pass
class Box(Geometry):
    pass
class CameraFrustum(Geometry):
    pass
class Circles2D(Geometry):
    def update_colors(self, colors: numpy.ndarray) -> None:
        ...
    def update_positions(self, positions: numpy.ndarray) -> None:
        ...
    def update_radii(self, radii: list[float] | numpy.ndarray) -> None:
        ...
class Geometry:
    pass
class Image(Geometry):
    pass
class Mesh(Geometry):
    def update_colors(self, colors: numpy.ndarray) -> None:
        ...
    def update_normals(self, normals: numpy.ndarray) -> None:
        ...
    def update_positions(self, positions: numpy.ndarray, recompute_normals: bool = True) -> None:
        ...
class Plane(Geometry):
    pass
class PointCloud(Geometry):
    def update_colors(self, colors: numpy.ndarray) -> None:
        ...
    def update_positions(self, positions: numpy.ndarray) -> None:
        ...
    def update_radii(self, radii: list[float] | numpy.ndarray) -> None:
        ...
class Points2D(Geometry):
    pass
class PolyLine(Geometry):
    pass
class PolyLine2D(Geometry):
    pass
class Sphere(Geometry):
    pass
class Spheres(Geometry):
    def update_colors(self, colors: numpy.ndarray) -> None:
        ...
    def update_positions(self, positions: numpy.ndarray) -> None:
        ...
    def update_radii(self, radii: list[float] | numpy.ndarray) -> None:
        ...
class Triad(Geometry):
    pass
