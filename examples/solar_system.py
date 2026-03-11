"""
Solar System — a showcase of slamd's visualization capabilities.

Features used:
  3D: Sphere, PointCloud, PolyLine, Triad, Arrows, Plane, CameraFrustum, Mesh
  2D: Points, PolyLine, Circles
  Dynamic transforms, scene hierarchy, multi-window (scene + canvas)
"""

import slamd
import numpy as np
import time
from scipy.spatial.transform import Rotation


# ---------------------------------------------------------------------------
# Planet data: (name, orbit_radius, size, speed, color_rgb_255, has_ring)
# ---------------------------------------------------------------------------
PLANETS = [
    ("mercury", 4.0, 0.25, 4.15, (180, 180, 180), False),
    ("venus", 6.5, 0.45, 1.62, (230, 180, 80), False),
    ("earth", 9.0, 0.50, 1.00, (50, 120, 220), False),
    ("mars", 12.0, 0.35, 0.53, (200, 80, 40), False),
    ("jupiter", 17.0, 1.20, 0.08, (210, 170, 120), False),
    ("saturn", 22.0, 1.00, 0.03, (220, 200, 150), True),
    ("uranus", 27.0, 0.70, 0.01, (170, 220, 230), True),
    ("neptune", 32.0, 0.65, 0.006, (60, 100, 220), False),
]

MOON_ORBIT = 1.2
MOON_SPEED = 5.0
MOON_SIZE = 0.15

# Jupiter moons (Galilean)
JUPITER_MOONS = [
    ("io", 1.8, 0.12, 8.0, (220, 200, 100)),
    ("europa", 2.3, 0.10, 5.5, (200, 190, 170)),
    ("ganymede", 3.0, 0.16, 3.2, (160, 150, 140)),
    ("callisto", 3.8, 0.14, 1.8, (120, 110, 100)),
]

# Comets: (name, semi_major, eccentricity, speed, inclination_deg)
COMETS = [
    ("halley", 25.0, 0.85, 0.04, 20.0),
    ("swift", 18.0, 0.70, 0.07, -15.0),
    ("hale_bopp", 30.0, 0.80, 0.025, 30.0),
]

COMET_TRAIL_LEN = 80


def orbit_points(radius: float, n: int = 256) -> np.ndarray:
    """Generate a flat circle of points for an orbit ring."""
    t = np.linspace(0, 2 * np.pi, n)
    pts = np.zeros((n, 3), dtype=np.float32)
    pts[:, 0] = radius * np.cos(t)
    pts[:, 1] = radius * np.sin(t)
    return pts


def make_ring_mesh(inner: float, outer: float, tilt_deg: float = 25.0, n: int = 64):
    """Triangulated annular ring for Saturn-like planets, pre-tilted."""
    angles = np.linspace(0, 2 * np.pi, n, endpoint=False)
    verts = []
    for a in angles:
        c, s = np.cos(a), np.sin(a)
        verts.append([inner * c, inner * s, 0.0])
        verts.append([outer * c, outer * s, 0.0])
    verts = np.array(verts, dtype=np.float32)

    # Bake the tilt into the vertices
    R = Rotation.from_euler("x", tilt_deg, degrees=True).as_matrix().astype(np.float32)
    verts = verts @ R.T

    indices = []
    for i in range(n):
        i0 = 2 * i
        i1 = i0 + 1
        i2 = (i0 + 2) % (2 * n)
        i3 = (i0 + 3) % (2 * n)
        indices.extend([i0, i2, i1])
        indices.extend([i1, i2, i3])

    colors = np.full_like(verts, 0.75)  # pale grey-gold
    colors[:, 0] = 0.85
    colors[:, 1] = 0.78
    colors[:, 2] = 0.60
    return verts, colors, indices


def make_starfield(n: int = 8000, spread: float = 80.0) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    """Random background stars as a point cloud."""
    rng = np.random.default_rng(42)
    # Spherical shell
    directions = rng.normal(size=(n, 3)).astype(np.float32)
    directions /= np.linalg.norm(directions, axis=1, keepdims=True)
    radii_dist = spread * (0.6 + 0.4 * rng.random(n, dtype=np.float32))
    positions = directions * radii_dist[:, None]

    brightness = rng.uniform(0.5, 1.0, size=(n, 3)).astype(np.float32)
    # Slight color tint
    brightness[:, 0] *= rng.uniform(0.8, 1.0, size=n).astype(np.float32)
    brightness[:, 2] *= rng.uniform(0.9, 1.0, size=n).astype(np.float32)

    radii = rng.uniform(0.05, 0.18, size=n).astype(np.float32)
    return positions, brightness, radii


def make_asteroid_belt(n: int = 3000, r_inner: float = 14.0, r_outer: float = 15.5):
    """Asteroid belt between Mars and Jupiter."""
    rng = np.random.default_rng(7)
    angles = rng.uniform(0, 2 * np.pi, n).astype(np.float32)
    radii = rng.uniform(r_inner, r_outer, n).astype(np.float32)
    z = rng.normal(0, 0.3, n).astype(np.float32)

    positions = np.zeros((n, 3), dtype=np.float32)
    positions[:, 0] = radii * np.cos(angles)
    positions[:, 1] = radii * np.sin(angles)
    positions[:, 2] = z

    colors = rng.uniform(0.3, 0.6, (n, 3)).astype(np.float32)
    pt_radii = rng.uniform(0.04, 0.12, n).astype(np.float32)
    return positions, colors, pt_radii, angles, radii


def comet_position(t: float, semi_major: float, ecc: float, speed: float, incl_deg: float):
    """Elliptical orbit for a comet, returns (x, y, z)."""
    # Mean anomaly
    M = speed * t
    # Approximate eccentric anomaly (few Newton iterations)
    E = M
    for _ in range(5):
        E = M + ecc * np.sin(E)
    # True position in orbital plane
    x = semi_major * (np.cos(E) - ecc)
    y = semi_major * np.sqrt(1 - ecc**2) * np.sin(E)
    # Tilt out of ecliptic
    incl = np.radians(incl_deg)
    z = y * np.sin(incl)
    y = y * np.cos(incl)
    return float(x), float(y), float(z)


def pose_at(x: float, y: float, z: float = 0.0) -> np.ndarray:
    T = np.eye(4, dtype=np.float32)
    T[0, 3] = x
    T[1, 3] = y
    T[2, 3] = z
    return T


def main():
    vis = slamd.Visualizer("Solar System", port=6010)
    scene = vis.scene("3D View")
    canvas = slamd.Canvas()
    vis.add_canvas("Radar Map", canvas)

    # ── Sun ──────────────────────────────────────────────────────────────
    scene.set_object("/sun/sphere", slamd.geom.Sphere(1.8, (255, 220, 50)))

    # Sun "corona" arrows radiating outward
    n_rays = 16
    ray_angles = np.linspace(0, 2 * np.pi, n_rays, endpoint=False)
    starts = np.zeros((n_rays, 3), dtype=np.float32)
    ends = np.zeros((n_rays, 3), dtype=np.float32)
    for i, a in enumerate(ray_angles):
        starts[i] = [2.0 * np.cos(a), 2.0 * np.sin(a), 0.0]
        ends[i] = [3.0 * np.cos(a), 3.0 * np.sin(a), 0.0]
    scene.set_object("/sun/corona", slamd.geom.Arrows(starts, ends, (255, 180, 30), 0.06))

    # ── Ecliptic plane ──────────────────────────────────────────────────
    scene.set_object(
        "/ecliptic",
        slamd.geom.Plane(
            np.array([0, 0, 1], dtype=np.float32),
            np.array([0, 0, 0], dtype=np.float32),
            (40, 50, 80),
            radius=38.0,
            alpha=0.12,
        ),
    )

    # ── Starfield ────────────────────────────────────────────────────────
    star_pos, star_col, star_rad = make_starfield()
    star_base_rad = star_rad.copy()
    star_pc = slamd.geom.PointCloud(star_pos, star_col, star_rad, 1.0)
    scene.set_object("/stars", star_pc)

    # ── Asteroid belt ────────────────────────────────────────────────────
    ast_pos, ast_col, ast_rad, ast_angles, ast_orbit_r = make_asteroid_belt()
    asteroid_pc = slamd.geom.PointCloud(ast_pos, ast_col, ast_rad, 0.6)
    scene.set_object("/asteroids", asteroid_pc)

    # ── Planets ──────────────────────────────────────────────────────────
    for name, orbit_r, size, speed, color, has_ring in PLANETS:
        # Orbit path
        scene.set_object(
            f"/orbits/{name}",
            slamd.geom.PolyLine(orbit_points(orbit_r), 0.04, (60, 70, 100), 1.0),
        )
        # Planet sphere
        scene.set_object(f"/planets/{name}/body", slamd.geom.Sphere(size, color))
        # Small triad on each planet
        scene.set_object(f"/planets/{name}/triad", slamd.geom.Triad(scale=size * 0.8, thickness=0.5))

        if has_ring:
            ring_v, ring_c, ring_i = make_ring_mesh(size * 1.8, size * 3.0, tilt_deg=25.0, n=96)
            scene.set_object(f"/planets/{name}/ring", slamd.geom.Mesh(ring_v, ring_c, ring_i))

        # Earth gets a moon
        if name == "earth":
            scene.set_object(f"/planets/{name}/moon/body", slamd.geom.Sphere(MOON_SIZE, (200, 200, 200)))

        # Jupiter gets Galilean moons
        if name == "jupiter":
            for mname, morbit, msize, mspeed, mcolor in JUPITER_MOONS:
                scene.set_object(
                    f"/planets/{name}/{mname}/body",
                    slamd.geom.Sphere(msize, mcolor),
                )

    # ── Comets with tails ────────────────────────────────────────────────
    comet_trails: dict[str, list[tuple[float, float, float]]] = {}
    for cname, *_ in COMETS:
        scene.set_object(f"/comets/{cname}/head", slamd.geom.Sphere(0.15, (200, 220, 255)))
        comet_trails[cname] = []

    # ── Space probe with camera frustum ──────────────────────────────────
    K = np.array([[400, 0, 320], [0, 400, 240], [0, 0, 1]], dtype=np.float64)
    # Simple gradient image for the frustum
    img = np.zeros((480, 640, 3), dtype=np.uint8)
    img[:, :, 2] = np.linspace(40, 180, 480, dtype=np.uint8)[:, None]
    img[:, :, 1] = 20
    img[:240, :, 0] = 10

    frustum = slamd.geom.CameraFrustum(K, 640, 480, img, 0.5)
    scene.set_object("/probe/frustum", frustum)
    scene.set_object("/probe/triad", slamd.geom.Triad(scale=0.8))
    scene.set_object("/probe/body", slamd.geom.Sphere(0.2, (200, 200, 220)))

    # Probe looks inward: rotate so camera -Z points toward sun
    probe_cam_rot = np.eye(4, dtype=np.float32)
    probe_cam_rot[:3, :3] = Rotation.from_euler("x", -90, degrees=True).as_matrix().astype(np.float32)
    scene.set_transform("/probe/frustum", probe_cam_rot)

    # ── 2D radar canvas setup ────────────────────────────────────────────
    # Orbit rings on canvas
    for name, orbit_r, *_ in PLANETS:
        ring_pts_2d = np.zeros((128, 2), dtype=np.float32)
        a = np.linspace(0, 2 * np.pi, 128)
        scale_2d = 12.0  # pixels per AU-ish unit
        ring_pts_2d[:, 0] = orbit_r * scale_2d * np.cos(a)
        ring_pts_2d[:, 1] = orbit_r * scale_2d * np.sin(a)
        canvas.set_object(f"/orbits/{name}", slamd.geom2d.PolyLine(ring_pts_2d, (30, 45, 70), 1.0))

    # Sun dot on radar
    canvas.set_object("/sun", slamd.geom2d.Points(
        np.array([[0.0, 0.0]], dtype=np.float32),
        (255, 220, 50),
        8.0,
    ))

    # Planet dots (will update positions each frame)
    n_planets = len(PLANETS)
    planet_2d_pos = np.zeros((n_planets, 2), dtype=np.float32)
    planet_2d_colors = np.array([p[4] for p in PLANETS], dtype=np.float32) / 255.0
    planet_2d_radii = np.array([p[2] * 6.0 for p in PLANETS], dtype=np.float32)
    canvas.set_object("/planets", slamd.geom2d.Points(planet_2d_pos, planet_2d_colors, planet_2d_radii))

    # Probe dot on radar
    probe_2d = slamd.geom2d.Circles(
        np.array([[0.0, 0.0]], dtype=np.float32),
        np.array([[0.8, 0.8, 1.0]], dtype=np.float32),
        np.array([5.0], dtype=np.float32),
        0.3,
    )
    canvas.set_object("/probe", probe_2d)

    # Probe trail on radar
    trail_max = 300
    trail_2d_buf = np.zeros((trail_max, 2), dtype=np.float32)
    trail_idx = 0
    trail_line = None

    # ── Animation loop ───────────────────────────────────────────────────
    t0 = time.monotonic()
    scale_2d = 12.0

    while True:
        time.sleep(0.016)
        t = time.monotonic() - t0

        # Update planets
        for i, (name, orbit_r, size, speed, color, has_ring) in enumerate(PLANETS):
            angle = speed * t * 0.4
            x = orbit_r * np.cos(angle)
            y = orbit_r * np.sin(angle)
            scene.set_transform(f"/planets/{name}", pose_at(x, y))

            # 2D radar
            planet_2d_pos[i] = [x * scale_2d, y * scale_2d]

            # Earth moon
            if name == "earth":
                ma = MOON_SPEED * t
                mx = MOON_ORBIT * np.cos(ma)
                my = MOON_ORBIT * np.sin(ma)
                scene.set_transform(f"/planets/{name}/moon", pose_at(mx, my))

            # Jupiter moons
            if name == "jupiter":
                for mname, morbit, msize, mspeed, mcolor in JUPITER_MOONS:
                    ma = mspeed * t
                    mx = morbit * np.cos(ma)
                    my = morbit * np.sin(ma)
                    scene.set_transform(f"/planets/{name}/{mname}", pose_at(mx, my))

        canvas.set_object("/planets", slamd.geom2d.Points(
            planet_2d_pos.copy(), planet_2d_colors, planet_2d_radii,
        ))

        # Update comets
        for cname, semi_major, ecc, cspeed, incl in COMETS:
            cx, cy, cz = comet_position(t, semi_major, ecc, cspeed, incl)
            scene.set_transform(f"/comets/{cname}", pose_at(cx, cy, cz))

            # Build tail
            trail = comet_trails[cname]
            trail.append((cx, cy, cz))
            if len(trail) > COMET_TRAIL_LEN:
                trail.pop(0)
            if len(trail) > 2:
                pts = np.array(trail, dtype=np.float32)
                scene.set_object(
                    f"/comets/{cname}/tail",
                    slamd.geom.PolyLine(pts, 0.06, (150, 180, 255), 1.0),
                )

        # Slowly rotate asteroid belt
        ast_new_angles = ast_angles + t * 0.02
        ast_pos[:, 0] = ast_orbit_r * np.cos(ast_new_angles)
        ast_pos[:, 1] = ast_orbit_r * np.sin(ast_new_angles)
        asteroid_pc.update_positions(ast_pos)

        # Probe: elliptical orbit tilted out of ecliptic
        probe_r = 20.0 + 8.0 * np.sin(t * 0.07)
        probe_angle = t * 0.15
        px = probe_r * np.cos(probe_angle)
        py = probe_r * np.sin(probe_angle)
        pz = 4.0 * np.sin(t * 0.11)

        probe_T = np.eye(4, dtype=np.float32)
        probe_T[0, 3] = px
        probe_T[1, 3] = py
        probe_T[2, 3] = pz
        # Point probe toward sun
        fwd = -np.array([px, py, pz], dtype=np.float32)
        fwd /= np.linalg.norm(fwd) + 1e-8
        up = np.array([0, 0, 1], dtype=np.float32)
        right = np.cross(fwd, up)
        right /= np.linalg.norm(right) + 1e-8
        up = np.cross(right, fwd)
        probe_T[:3, 0] = right
        probe_T[:3, 1] = up
        probe_T[:3, 2] = -fwd
        scene.set_transform("/probe", probe_T)

        # Probe on radar
        canvas.set_object("/probe", slamd.geom2d.Circles(
            np.array([[px * scale_2d, py * scale_2d]], dtype=np.float32),
            np.array([[0.8, 0.8, 1.0]], dtype=np.float32),
            np.array([5.0], dtype=np.float32),
            0.3,
        ))

        # Probe trail on radar
        trail_2d_buf[trail_idx % trail_max] = [px * scale_2d, py * scale_2d]
        trail_idx += 1
        n_trail = min(trail_idx, trail_max)
        if n_trail > 2:
            # Reorder buffer so it's sequential
            if trail_idx <= trail_max:
                trail_pts = trail_2d_buf[:n_trail].copy()
            else:
                start = trail_idx % trail_max
                trail_pts = np.concatenate([trail_2d_buf[start:], trail_2d_buf[:start]])
            if trail_line is None:
                trail_line = True
            canvas.set_object("/probe_trail", slamd.geom2d.PolyLine(trail_pts, (120, 140, 200), 1.0))

        # Twinkling stars — subtle radius oscillation
        twinkle = 1.0 + 0.3 * np.sin(t * 3.0 + star_pos[:, 0] * 0.5)
        star_pc.update_radii(star_base_rad * twinkle)

        # Pulsing sun corona — rotate arrows
        rot_corona = np.eye(4, dtype=np.float32)
        rot_corona[:3, :3] = Rotation.from_euler("z", t * 0.3).as_matrix().astype(np.float32)
        scene.set_transform("/sun/corona", rot_corona)


if __name__ == "__main__":
    main()
