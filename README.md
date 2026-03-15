<img src="./images/logo.png" width="100%">

---

<img src="./images/galaxy.gif" width="49%"> <img src="./images/lorenz_attractor.gif" width="49%">

<img src="./images/double_pendulum.gif" width="49%"> <img src="./images/moving_spheres.gif" width="49%">

`slamd` is a 3D visualization library for Python. `pip install`, write a few lines, and you have a GPU-accelerated interactive 3D viewer. No event loops, no boilerplate — just set geometry and it shows up.

```bash
pip install slamd
```

## Why slamd?

Most 3D visualization in Python is either painfully slow (matplotlib's 3D mode), or requires you to learn a massive framework. `slamd` is neither.

- **Dead simple** — create a visualizer, set geometry, done. The viewer window spawns automatically in a separate process. No event loop, no main thread hijacking, no callbacks.
- **Real 3D rendering** — GPU-accelerated OpenGL. Handles millions of points, animated meshes, and real geometry at interactive framerates. This is not a plot library pretending to do 3D.
- **Pose tree** — objects live in a transform tree (like ROS TF or a scene graph). Set a parent transform and everything underneath moves. Makes articulated and hierarchical scenes trivial.
- **The right primitives** — point clouds, meshes, camera frustums, triads, arrows, polylines, spheres, planes. The stuff you actually need when working with 3D data, robotics, or SLAM.

## How is this different from Rerun?

`slamd` is a **stateful viewer**, not a logging database. There's no append-only log, no timeline, no timestamps forced onto your data. You have a tree of geometry — you set objects, move them, delete them, and what you see is what's there right now.

Rerun is powerful, but it's a big tool with a lot of concepts. `slamd` does one thing: show your geometry, right now, with minimal API surface. If you want a data recording platform with time-series scrubbing, use Rerun. If you want to throw some geometry on screen and look at it, use `slamd`.

## Quick Start

```python
import slamd

vis = slamd.Visualizer("Hello world")
scene = vis.scene("scene")
scene.set_object("/origin", slamd.geom.Triad())
```

![](./images/hello_world.gif)

That's it. A window opens with an interactive 3D view.

Objects live in a transform tree — move a parent and children follow:

```python
scene.set_object("/robot/camera/frustum", slamd.geom.CameraFrustum(K, w, h, scale=0.2))
scene.set_object("/robot/lidar/cloud", slamd.geom.PointCloud(pts, colors, point_size))

# Move the whole robot — camera and lidar come with it
scene.set_transform("/robot", pose)
```

## Multiple Windows

Create multiple scenes — each gets its own sub-window with ImGui docking. Drag, tab, float, or dock them however you like:

```python
vis = slamd.Visualizer("multi-view")
scene1 = vis.scene("RGB camera")
scene2 = vis.scene("point cloud")

scene1.set_object("/frustum", slamd.geom.CameraFrustum(K, w, h, img, 1.0))
scene2.set_object("/cloud", slamd.geom.PointCloud(pts, colors, 0.3, 0.5))
```

![](./images/two_windows.gif)

## Geometry

- Point clouds — `slamd.geom.PointCloud`
- Meshes — `slamd.geom.Mesh`
- Camera frustums (with image) — `slamd.geom.CameraFrustum`
- Arrows — `slamd.geom.Arrows`
- Polylines — `slamd.geom.PolyLine`
- Spheres — `slamd.geom.Sphere` / `slamd.geom.Spheres`
- Triads — `slamd.geom.Triad`
- Planes — `slamd.geom.Plane`
- Boxes — `slamd.geom.Box`

## Installation

Wheels on [PyPI](https://pypi.org/project/slamd/) for Linux and macOS (Python >= 3.11):

```bash
pip install slamd
```

Only runtime dependency is `numpy >= 1.23`.

## Examples

See [examples/](./examples) for the full set.

## License

Apache 2.0 — see [LICENSE](./LICENSE).
