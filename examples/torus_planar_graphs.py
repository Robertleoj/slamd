import slamd
import numpy as np
from itertools import combinations


TAU = 2 * np.pi

VERTEX_COLORS = np.array(
    [
        [1.0, 0.3, 0.3],
        [0.3, 1.0, 0.3],
        [0.3, 0.3, 1.0],
        [1.0, 1.0, 0.3],
        [1.0, 0.3, 1.0],
        [0.3, 1.0, 1.0],
        [1.0, 0.6, 0.2],
    ],
    dtype=np.float32,
)

EDGE_COLOR = np.array([0.9, 0.9, 0.9], dtype=np.float32)


def torus_point(u, v, R=3.0, r=1.0):
    x = (R + r * np.cos(v)) * np.cos(u)
    y = (R + r * np.cos(v)) * np.sin(u)
    z = r * np.sin(v)
    return np.array([x, y, z])


def torus_points(u, v, R=3.0, r=1.0):
    x = (R + r * np.cos(v)) * np.cos(u)
    y = (R + r * np.cos(v)) * np.sin(u)
    z = r * np.sin(v)
    return np.column_stack([x, y, z]).astype(np.float32)


def make_torus_mesh(R=3.0, r=1.0, nu=80, nv=40, offset=None):
    u = np.linspace(0, TAU, nu, endpoint=False)
    v = np.linspace(0, TAU, nv, endpoint=False)
    uu, vv = np.meshgrid(u, v)
    uu = uu.ravel()
    vv = vv.ravel()

    vertices = torus_points(uu, vv, R, r)
    if offset is not None:
        vertices += np.array(offset, dtype=np.float32)

    u_norm = uu / TAU
    v_norm = vv / TAU
    colors = np.column_stack(
        [
            0.15 + 0.15 * np.sin(TAU * u_norm),
            0.2 + 0.15 * np.cos(TAU * v_norm),
            0.25 + 0.1 * np.sin(TAU * (u_norm + v_norm)),
        ]
    ).astype(np.float32)

    indices = []
    for j in range(nv):
        for i in range(nu):
            p00 = j * nu + i
            p10 = j * nu + (i + 1) % nu
            p01 = ((j + 1) % nv) * nu + i
            p11 = ((j + 1) % nv) * nu + (i + 1) % nu
            indices.extend([p00, p10, p01])
            indices.extend([p10, p11, p01])

    return vertices, colors, indices


def shortest_wrap(a, b):
    diff = b - a
    if diff > np.pi:
        diff -= TAU
    elif diff < -np.pi:
        diff += TAU
    return diff


def edge_on_torus(uv_a, uv_b, R=3.0, r=1.0, offset=None, n=60):
    t = np.linspace(0, 1, n)
    du = shortest_wrap(uv_a[0], uv_b[0])
    dv = shortest_wrap(uv_a[1], uv_b[1])
    u = uv_a[0] + t * du
    v = uv_a[1] + t * dv
    pts = torus_points(u, v, R, r * 1.06)
    if offset is not None:
        pts += np.array(offset, dtype=np.float32)
    return pts


def vertex_uvs(n):
    """Vertex positions via Heffter construction (mod base, stride 2).

    K5: mod 5, stride 2 (coprime).
    K6: uses mod 7, stride 2 (first 6 of K7) since no stride is both
        coprime to 6 and non-degenerate.
    K7: mod 7, stride 2 (coprime).
    """
    if n <= 5:
        base, s = n, 2
    else:
        base, s = 7, 2
    return [(TAU * i / base, TAU * (s * i % base) / base) for i in range(n)]


def add_kn_on_torus(scene, n, R, r, offset, path_prefix):
    verts, colors, indices = make_torus_mesh(R, r, offset=offset)
    scene.set_object(f"{path_prefix}/torus", slamd.geom.Mesh(verts, colors, indices))

    uvs = vertex_uvs(n)
    off = np.array(offset)
    positions = np.array(
        [torus_point(u, v, R, r * 1.08) + off for u, v in uvs],
        dtype=np.float32,
    )
    radii = np.full(n, 0.18, dtype=np.float32)
    scene.set_object(
        f"{path_prefix}/vertices",
        slamd.geom.Spheres(positions, VERTEX_COLORS[:n], radii, 0.3),
    )

    for i, j in combinations(range(n), 2):
        pts = edge_on_torus(uvs[i], uvs[j], R, r, offset=offset)
        scene.set_object(
            f"{path_prefix}/edges/{i}_{j}",
            slamd.geom.PolyLine(pts, 0.04, EDGE_COLOR, 0.4),
        )


def main():
    vis = slamd.Visualizer("Complete Graphs on Tori")
    scene = vis.scene("scene")

    R, r = 3.0, 1.0
    spacing = 2 * (R + r) + 2.0

    add_kn_on_torus(scene, 5, R, r, offset=(-spacing, 0, 0), path_prefix="/k5")
    add_kn_on_torus(scene, 6, R, r, offset=(0, 0, 0), path_prefix="/k6")
    add_kn_on_torus(scene, 7, R, r, offset=(spacing, 0, 0), path_prefix="/k7")


if __name__ == "__main__":
    main()
