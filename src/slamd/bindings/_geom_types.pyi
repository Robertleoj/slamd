from __future__ import annotations
import numpy

__all__: list[str] = [
    "Arrows",
    "Box",
    "CameraFrustum",
    "Geometry",
    "Image",
    "Mesh",
    "Plane",
    "PointCloud",
    "PolyLine",
    "Sphere",
    "Spheres",
    "Triad",
]

class Arrows(Geometry):
    pass

class Box(Geometry):
    pass

class CameraFrustum(Geometry):
    pass

class Geometry:
    pass

class Image(Geometry):
    pass

class Mesh(Geometry):
    def update_colors(self, colors: numpy.ndarray) -> None: ...
    def update_normals(self, normals: numpy.ndarray) -> None: ...
    def update_positions(
        self, positions: numpy.ndarray, recompute_normals: bool = True
    ) -> None: ...

class Plane(Geometry):
    pass

class PointCloud(Geometry):
    def update_colors(self, colors: numpy.ndarray) -> None: ...
    def update_positions(self, positions: numpy.ndarray) -> None: ...
    def update_radii(self, radii: list[float] | numpy.ndarray) -> None: ...

class PolyLine(Geometry):
    pass

class Sphere(Geometry):
    pass

class Spheres(Geometry):
    def update_colors(self, colors: numpy.ndarray) -> None: ...
    def update_positions(self, positions: numpy.ndarray) -> None: ...
    def update_radii(self, radii: list[float] | numpy.ndarray) -> None: ...

class Triad(Geometry):
    pass
