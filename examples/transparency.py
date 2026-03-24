"""Depth peeling demo — transparent planes and meshes mixed with opaque objects."""

import time

import slamd
import numpy as np
from scipy.spatial.transform import Rotation


def make_transform(pos, rot=None):
    T = np.eye(4, dtype=np.float32)
    T[:3, 3] = pos
    if rot is not None:
        T[:3, :3] = rot
    return T


def uv_sphere(radius, u_res=32, v_res=16):
    """Generate a UV sphere mesh. Returns (verts, indices, normals)."""
    u = np.linspace(0, 2 * np.pi, u_res, endpoint=False)
    v = np.linspace(0, np.pi, v_res + 1)
    verts, normals = [], []
    for vi in range(len(v)):
        for ui in range(len(u)):
            nx = np.sin(v[vi]) * np.cos(u[ui])
            ny = np.sin(v[vi]) * np.sin(u[ui])
            nz = np.cos(v[vi])
            normals.append([nx, ny, nz])
            verts.append([radius * nx, radius * ny, radius * nz])

    indices = []
    for vi in range(len(v) - 1):
        for ui in range(len(u)):
            i0 = vi * len(u) + ui
            i1 = vi * len(u) + (ui + 1) % len(u)
            i2 = i0 + len(u)
            i3 = i1 + len(u)
            indices.extend([i0, i2, i1, i1, i2, i3])

    return (
        np.array(verts, dtype=np.float32),
        np.array(indices, dtype=np.uint32),
        np.array(normals, dtype=np.float32),
    )


def torus(major_r, minor_r, u_res=48, v_res=24):
    """Generate a torus mesh. Returns (verts, indices, normals)."""
    u = np.linspace(0, 2 * np.pi, u_res, endpoint=False)
    v = np.linspace(0, 2 * np.pi, v_res, endpoint=False)
    verts, normals = [], []
    for ui in range(len(u)):
        cu, su = np.cos(u[ui]), np.sin(u[ui])
        for vi in range(len(v)):
            cv, sv = np.cos(v[vi]), np.sin(v[vi])
            x = (major_r + minor_r * cv) * cu
            y = (major_r + minor_r * cv) * su
            z = minor_r * sv
            verts.append([x, y, z])
            normals.append([cv * cu, cv * su, sv])

    indices = []
    for ui in range(u_res):
        for vi in range(v_res):
            i0 = ui * v_res + vi
            i1 = ui * v_res + (vi + 1) % v_res
            i2 = ((ui + 1) % u_res) * v_res + vi
            i3 = ((ui + 1) % u_res) * v_res + (vi + 1) % v_res
            indices.extend([i0, i2, i1, i1, i2, i3])

    return (
        np.array(verts, dtype=np.float32),
        np.array(indices, dtype=np.uint32),
        np.array(normals, dtype=np.float32),
    )


vis = slamd.Visualizer("Transparency")
scene = vis.scene("scene")
rng = np.random.default_rng(7)

# --- Opaque objects ---

# Scattered spheres
n_spheres = 40
positions = rng.uniform(-4, 4, size=(n_spheres, 3)).astype(np.float32)
sphere_colors = rng.uniform(0.3, 1.0, size=(n_spheres, 3)).astype(np.float32)
radii = rng.uniform(0.1, 0.35, size=n_spheres).astype(np.float32)
scene.set_object("/opaque/spheres", slamd.geom.Spheres(positions, sphere_colors, radii))

# Boxes at various rotations
boxes = [
    ([2.5, 0, 0], [0.9, 0.3, 0.1], [0.8, 1.2, 0.6], [15, 0, 10]),
    ([-2.5, 1, -1], [0.1, 0.4, 0.9], [1.0, 0.5, 0.8], [0, 25, -10]),
    ([0, -2.5, 2], [0.2, 0.8, 0.3], [0.6, 0.6, 1.0], [-20, 10, 30]),
    ([1, 2, -2], [0.9, 0.7, 0.1], [0.5, 0.5, 0.5], [45, 15, 0]),
]
for i, (pos, color, dims, euler_deg) in enumerate(boxes):
    rot = Rotation.from_euler("xyz", euler_deg, degrees=True).as_matrix()
    scene.set_object(
        f"/opaque/box_{i}",
        slamd.geom.Box(
            np.array(dims, dtype=np.float32),
            np.array(color, dtype=np.float32),
        ),
    )
    scene.set_transform(f"/opaque/box_{i}", make_transform(pos, rot.astype(np.float32)))

# Polyline threading through the scene
t = np.linspace(0, 4 * np.pi, 100, dtype=np.float32)
path = np.column_stack([2 * np.cos(t), 2 * np.sin(t), np.linspace(-3, 3, len(t))])
scene.set_object(
    "/opaque/helix",
    slamd.geom.PolyLine(path.astype(np.float32), 0.06, np.array([1.0, 1.0, 1.0], dtype=np.float32)),
)

# Origin triad
scene.set_object("/opaque/triad", slamd.geom.Triad())

# --- Transparent planes (intersecting at origin) ---

planes = [
    ("yz", [1, 0, 0], [0, 0, 0], [0.9, 0.2, 0.1], 5.0, 0.3),
    ("xz", [0, 1, 0], [0, 0, 0], [0.1, 0.8, 0.2], 5.0, 0.3),
    ("xy", [0, 0, 1], [0, 0, 0], [0.1, 0.3, 0.9], 5.0, 0.3),
    ("diag", [1, 1, 1], [1, 1, 1], [1.0, 0.85, 0.1], 3.0, 0.2),
]
for name, normal, pos, color, radius, alpha in planes:
    scene.set_object(
        f"/transparent/plane_{name}",
        slamd.geom.Plane(
            np.array(normal, dtype=np.float32),
            np.array(pos, dtype=np.float32),
            np.array(color, dtype=np.float32),
            radius,
            alpha,
        ),
    )

# --- Transparent meshes ---

# Glass sphere at origin
sv, si, sn = uv_sphere(2.0)
scene.set_object(
    "/transparent/glass_sphere",
    slamd.geom.Mesh(
        sv,
        np.full_like(sv, [0.6, 0.8, 1.0]),
        si,
        sn,
        alpha=0.2,
    ),
)

# Magenta torus around the y axis
tv, ti, tn = torus(3.0, 0.6)
# Rotate so it sits around the y axis
rot_torus = Rotation.from_euler("x", 90, degrees=True).as_matrix().astype(np.float32)
tv_rotated = (rot_torus @ tv.T).T
tn_rotated = (rot_torus @ tn.T).T
scene.set_object(
    "/transparent/torus",
    slamd.geom.Mesh(
        tv_rotated,
        np.full_like(tv, [0.8, 0.2, 0.7]),
        ti,
        tn_rotated,
        alpha=0.35,
    ),
)

# Small offset green sphere
sv2, si2, sn2 = uv_sphere(1.2)
sv2 += np.array([2.0, 1.5, 0.0], dtype=np.float32)
scene.set_object(
    "/transparent/green_sphere",
    slamd.geom.Mesh(
        sv2,
        np.full_like(sv2, [0.2, 0.9, 0.3]),
        si2,
        sn2,
        alpha=0.25,
    ),
)

time.sleep(10)
