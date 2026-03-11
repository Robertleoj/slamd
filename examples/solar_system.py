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

# Mars moons
MARS_MOONS = [
    ("phobos", 0.8, 0.08, 12.0, (160, 140, 120)),
    ("deimos", 1.3, 0.06, 6.0, (140, 130, 110)),
]

# Dwarf planets in the Kuiper belt
DWARF_PLANETS = [
    ("pluto", 36.0, 0.30, 0.003, (200, 180, 160)),
    ("eris", 42.0, 0.25, 0.001, (220, 220, 230)),
    ("makemake", 39.0, 0.22, 0.002, (190, 170, 150)),
]

# Space station orbiting Earth
ISS_ORBIT = 0.9
ISS_SPEED = 15.0

# Voyager escape trajectory
VOYAGER_SPEED = 0.12
VOYAGER_SPIRAL_RATE = 0.03

# Meteor shower config
N_METEORS = 200
METEOR_SPEED = 2.5

# Jupiter radiation belt
N_RADIATION_PARTICLES = 2000

# Black hole lurking in the outer system
BLACK_HOLE_POS = (50.0, -20.0, 5.0)
BLACK_HOLE_MASS = 3.0  # controls accretion disk size
N_ACCRETION = 3000

# Space jellyfish count
N_JELLYFISH = 4

# The Monolith (2001 reference) — orbits Jupiter
MONOLITH_ORBIT = 5.0
MONOLITH_SPEED = 0.7

# Alien ship
ALIEN_SPEED = 0.2

# Wormhole
WORMHOLE_ENTRY = (15.0, -25.0, 0.0)
WORMHOLE_EXIT = (-30.0, 10.0, 8.0)
N_WORMHOLE_PARTICLES = 600

# Space whale — cosmic leviathan
WHALE_SPEED = 0.04
WHALE_BREATH_INTERVAL = 8.0  # seconds between particle breaths
N_WHALE_BREATH = 150

# Pulsar — spinning neutron star with beams
PULSAR_POS = (-35.0, 30.0, 12.0)
PULSAR_SPIN = 6.0  # rotations per second
N_PULSAR_BEAM = 400

# Planet X (Nibiru) — rogue planet on crazy orbit
NIBIRU_SEMI_MAJOR = 55.0
NIBIRU_ECC = 0.92
NIBIRU_SPEED = 0.008
NIBIRU_INCL = 65.0  # extreme tilt

# Space Kraken — lurking behind Neptune
KRAKEN_POS = (35.0, 15.0, -8.0)
N_KRAKEN_TENTACLES = 8
KRAKEN_REACH = 12.0

# Dyson sphere construction
N_DYSON_ARCS = 12
DYSON_RADIUS = 3.5

# Quantum tunneling particles
N_QUANTUM = 100
QUANTUM_TELEPORT_RATE = 0.3  # probability per second of teleporting

# Cosmic spiderweb anchor points
SPIDER_SPEED = 0.05


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


def make_starfield(
    n: int = 8000, spread: float = 80.0
) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
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


def make_kuiper_belt(n: int = 4000, r_inner: float = 34.0, r_outer: float = 45.0):
    """Kuiper belt beyond Neptune."""
    rng = np.random.default_rng(99)
    angles = rng.uniform(0, 2 * np.pi, n).astype(np.float32)
    radii = rng.uniform(r_inner, r_outer, n).astype(np.float32)
    z = rng.normal(0, 0.8, n).astype(np.float32)

    positions = np.zeros((n, 3), dtype=np.float32)
    positions[:, 0] = radii * np.cos(angles)
    positions[:, 1] = radii * np.sin(angles)
    positions[:, 2] = z

    # Icy blue-grey colors
    colors = np.zeros((n, 3), dtype=np.float32)
    colors[:, 0] = rng.uniform(0.3, 0.5, n).astype(np.float32)
    colors[:, 1] = rng.uniform(0.35, 0.55, n).astype(np.float32)
    colors[:, 2] = rng.uniform(0.45, 0.65, n).astype(np.float32)
    pt_radii = rng.uniform(0.03, 0.10, n).astype(np.float32)
    return positions, colors, pt_radii, angles, radii


def comet_position(
    t: float, semi_major: float, ecc: float, speed: float, incl_deg: float
):
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


def make_radiation_belt(n: int = 2000, planet_r: float = 1.2):
    """Torus-shaped radiation belt around Jupiter."""
    rng = np.random.default_rng(33)
    # Torus parameters
    major_r = planet_r * 2.5  # distance from planet center to ring center
    minor_r = planet_r * 0.6  # tube radius

    theta = rng.uniform(0, 2 * np.pi, n).astype(np.float32)  # around the ring
    phi = rng.uniform(0, 2 * np.pi, n).astype(np.float32)  # around the tube
    tube_r = minor_r * np.sqrt(rng.uniform(0, 1, n).astype(np.float32))  # fill tube

    x = (major_r + tube_r * np.cos(phi)) * np.cos(theta)
    y = (major_r + tube_r * np.cos(phi)) * np.sin(theta)
    z = tube_r * np.sin(phi)

    positions = np.stack([x, y, z], axis=1).astype(np.float32)

    # Hot radiation colors: yellows, oranges, whites
    colors = np.zeros((n, 3), dtype=np.float32)
    heat = rng.uniform(0.5, 1.0, n).astype(np.float32)
    colors[:, 0] = heat
    colors[:, 1] = heat * rng.uniform(0.4, 0.9, n).astype(np.float32)
    colors[:, 2] = heat * rng.uniform(0.1, 0.4, n).astype(np.float32)

    radii = rng.uniform(0.02, 0.06, n).astype(np.float32)
    return positions, colors, radii, theta


def make_meteor_shower(n: int = 200):
    """Generate meteor positions and velocities."""
    rng = np.random.default_rng(77)
    # Start from a random direction, all heading roughly toward inner system
    source_angle = rng.uniform(0, 2 * np.pi)
    spread = 15.0

    positions = np.zeros((n, 3), dtype=np.float32)
    positions[:, 0] = rng.uniform(-spread, spread, n).astype(np.float32)
    positions[:, 1] = rng.uniform(-spread, spread, n).astype(np.float32)
    positions[:, 2] = rng.uniform(-3, 3, n).astype(np.float32)

    # Velocities point roughly inward with some spread
    velocities = np.zeros((n, 3), dtype=np.float32)
    velocities[:, 0] = -np.cos(source_angle) + rng.normal(0, 0.3, n).astype(np.float32)
    velocities[:, 1] = -np.sin(source_angle) + rng.normal(0, 0.3, n).astype(np.float32)
    velocities[:, 2] = rng.normal(0, 0.1, n).astype(np.float32)

    # Stagger start times so they don't all appear at once
    phase = rng.uniform(0, 30.0, n).astype(np.float32)

    colors = np.zeros((n, 3), dtype=np.float32)
    colors[:, 0] = rng.uniform(0.8, 1.0, n).astype(np.float32)
    colors[:, 1] = rng.uniform(0.6, 0.9, n).astype(np.float32)
    colors[:, 2] = rng.uniform(0.3, 0.6, n).astype(np.float32)

    radii = rng.uniform(0.03, 0.08, n).astype(np.float32)
    return positions, velocities, phase, colors, radii


def make_solar_wind(n_shells: int = 5, particles_per_shell: int = 40):
    """Expanding shells of particles from the sun."""
    rng = np.random.default_rng(55)
    total = n_shells * particles_per_shell
    positions = np.zeros((total, 3), dtype=np.float32)

    # Random directions on sphere for each particle
    directions = rng.normal(size=(total, 3)).astype(np.float32)
    directions /= np.linalg.norm(directions, axis=1, keepdims=True)

    colors = np.zeros((total, 3), dtype=np.float32)
    colors[:, 0] = 1.0
    colors[:, 1] = rng.uniform(0.6, 0.9, total).astype(np.float32)
    colors[:, 2] = rng.uniform(0.1, 0.3, total).astype(np.float32)

    radii = np.full(total, 0.04, dtype=np.float32)
    return positions, directions, colors, radii


def make_accretion_disk(n: int = 3000, center: tuple = (0, 0, 0), mass: float = 3.0):
    """Swirling accretion disk around a black hole."""
    rng = np.random.default_rng(666)
    angles = rng.uniform(0, 2 * np.pi, n).astype(np.float32)
    # Particles closer in orbit faster, further out slower
    radii = (mass * 0.5 + mass * 2.0 * rng.power(0.5, n)).astype(np.float32)
    z = rng.normal(0, 0.15, n).astype(np.float32) * (radii / radii.max())

    positions = np.zeros((n, 3), dtype=np.float32)
    positions[:, 0] = center[0] + radii * np.cos(angles)
    positions[:, 1] = center[1] + radii * np.sin(angles)
    positions[:, 2] = center[2] + z

    # Hot inner disk to cool outer: white → orange → red → dark
    temp = 1.0 - (radii - radii.min()) / (radii.max() - radii.min() + 1e-8)
    colors = np.zeros((n, 3), dtype=np.float32)
    colors[:, 0] = np.clip(temp * 1.5, 0, 1)
    colors[:, 1] = np.clip(temp * 0.8 - 0.1, 0, 1)
    colors[:, 2] = np.clip(temp * 0.5 - 0.2, 0, 1)

    pt_radii = (0.03 + 0.05 * temp).astype(np.float32)
    # Angular velocity inversely proportional to sqrt(radius) (Kepler)
    angular_vel = 1.0 / np.sqrt(radii + 0.5)
    return positions, colors, pt_radii, angles, radii, angular_vel


def make_wormhole_particles(
    n: int = 600, entry: tuple = (0, 0, 0), exit_pt: tuple = (0, 0, 0)
):
    """Particles spiraling through a wormhole tunnel."""
    rng = np.random.default_rng(42)
    # Each particle has a phase along the tunnel [0, 1]
    phase = rng.uniform(0, 1, n).astype(np.float32)
    # Spiral angle
    spiral_angle = rng.uniform(0, 2 * np.pi, n).astype(np.float32)
    # Tunnel radius varies (wider at ends, narrow in middle)
    base_radius = rng.uniform(0.3, 1.5, n).astype(np.float32)

    colors = np.zeros((n, 3), dtype=np.float32)
    colors[:, 0] = rng.uniform(0.3, 0.8, n).astype(np.float32)
    colors[:, 1] = rng.uniform(0.0, 0.5, n).astype(np.float32)
    colors[:, 2] = rng.uniform(0.6, 1.0, n).astype(np.float32)

    radii = rng.uniform(0.03, 0.08, n).astype(np.float32)
    return phase, spiral_angle, base_radius, colors, radii


def make_pulsar_beam(n: int = 400):
    """Two opposing beams of particles for a pulsar."""
    rng = np.random.default_rng(111)
    # Particles along beam axis with some spread
    beam_len = rng.uniform(0.5, 8.0, n).astype(np.float32)
    spread = rng.normal(0, 0.15, (n, 2)).astype(np.float32)
    # Half go up, half go down
    direction = np.ones(n, dtype=np.float32)
    direction[n // 2 :] = -1.0

    positions = np.zeros((n, 3), dtype=np.float32)
    positions[:, 2] = beam_len * direction
    positions[:, 0] = spread[:, 0] * (1.0 + beam_len * 0.1)
    positions[:, 1] = spread[:, 1] * (1.0 + beam_len * 0.1)

    # Cyan-white hot beam colors
    colors = np.zeros((n, 3), dtype=np.float32)
    intensity = np.clip(1.0 - beam_len / 8.0, 0.2, 1.0)
    colors[:, 0] = intensity * 0.5
    colors[:, 1] = intensity * 0.9
    colors[:, 2] = intensity * 1.0

    radii = (0.04 + 0.08 * (1.0 - beam_len / 8.0)).astype(np.float32)
    return positions, colors, radii, beam_len, direction


def make_quantum_particles(n: int = 100):
    """Particles that teleport around randomly."""
    rng = np.random.default_rng(999)
    positions = np.zeros((n, 3), dtype=np.float32)
    positions[:, 0] = rng.uniform(-30, 30, n).astype(np.float32)
    positions[:, 1] = rng.uniform(-30, 30, n).astype(np.float32)
    positions[:, 2] = rng.uniform(-5, 5, n).astype(np.float32)

    # Eerie green-blue quantum colors
    colors = np.zeros((n, 3), dtype=np.float32)
    colors[:, 0] = rng.uniform(0.0, 0.3, n).astype(np.float32)
    colors[:, 1] = rng.uniform(0.6, 1.0, n).astype(np.float32)
    colors[:, 2] = rng.uniform(0.4, 0.8, n).astype(np.float32)

    radii = rng.uniform(0.05, 0.12, n).astype(np.float32)
    return positions, colors, radii


def make_kraken_tentacle(n_pts: int = 20):
    """Single kraken tentacle — a wavy arm."""
    t_param = np.linspace(0, 1, n_pts, dtype=np.float32)
    return t_param


def make_dyson_arc(arc_idx: int, n_arcs: int, radius: float, n_pts: int = 64):
    """One arc of the Dyson sphere skeleton."""
    # Great circle at a specific tilt
    tilt = arc_idx * np.pi / n_arcs
    t = np.linspace(0, 2 * np.pi, n_pts, dtype=np.float32)
    pts = np.zeros((n_pts, 3), dtype=np.float32)
    pts[:, 0] = radius * np.cos(t)
    pts[:, 1] = radius * np.sin(t) * np.cos(tilt)
    pts[:, 2] = radius * np.sin(t) * np.sin(tilt)
    return pts


def pose_at(x: float, y: float, z: float = 0.0) -> np.ndarray:
    T = np.eye(4, dtype=np.float32)
    T[0, 3] = x
    T[1, 3] = y
    T[2, 3] = z
    return T


def main():
    vis = slamd.Visualizer("Solar System", port=6011)
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
    scene.set_object(
        "/sun/corona", slamd.geom.Arrows(starts, ends, (255, 180, 30), 0.06)
    )

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
        scene.set_object(
            f"/planets/{name}/triad", slamd.geom.Triad(scale=size * 0.8, thickness=0.5)
        )

        if has_ring:
            ring_v, ring_c, ring_i = make_ring_mesh(
                size * 1.8, size * 3.0, tilt_deg=25.0, n=96
            )
            scene.set_object(
                f"/planets/{name}/ring", slamd.geom.Mesh(ring_v, ring_c, ring_i)
            )

        # Earth gets a moon
        if name == "earth":
            scene.set_object(
                f"/planets/{name}/moon/body",
                slamd.geom.Sphere(MOON_SIZE, (200, 200, 200)),
            )

        # Jupiter gets Galilean moons
        if name == "jupiter":
            for mname, morbit, msize, mspeed, mcolor in JUPITER_MOONS:
                scene.set_object(
                    f"/planets/{name}/{mname}/body",
                    slamd.geom.Sphere(msize, mcolor),
                )

        # Mars gets Phobos and Deimos
        if name == "mars":
            for mname, morbit, msize, mspeed, mcolor in MARS_MOONS:
                scene.set_object(
                    f"/planets/{name}/{mname}/body",
                    slamd.geom.Sphere(msize, mcolor),
                )

    # ── Kuiper belt ──────────────────────────────────────────────────────
    kui_pos, kui_col, kui_rad, kui_angles, kui_orbit_r = make_kuiper_belt()
    kuiper_pc = slamd.geom.PointCloud(kui_pos, kui_col, kui_rad, 0.5)
    scene.set_object("/kuiper_belt", kuiper_pc)

    # ── Dwarf planets ────────────────────────────────────────────────────
    for dname, dorbit, dsize, dspeed, dcolor in DWARF_PLANETS:
        scene.set_object(
            f"/dwarf_planets/{dname}/body", slamd.geom.Sphere(dsize, dcolor)
        )
        scene.set_object(
            f"/orbits/{dname}",
            slamd.geom.PolyLine(orbit_points(dorbit), 0.03, (50, 55, 70), 1.0),
        )

    # ── Space station (ISS) orbiting Earth ─────────────────────────────
    scene.set_object("/planets/earth/iss/body", slamd.geom.Box())
    iss_scale = np.eye(4, dtype=np.float32)
    iss_scale[0, 0] = 0.08  # long solar panels
    iss_scale[1, 1] = 0.25
    iss_scale[2, 2] = 0.03
    scene.set_transform("/planets/earth/iss/body", iss_scale)

    # ── Jupiter radiation belts ──────────────────────────────────────────
    rad_pos, rad_col, rad_rad, rad_theta = make_radiation_belt(N_RADIATION_PARTICLES)
    radiation_pc = slamd.geom.PointCloud(rad_pos, rad_col, rad_rad, 1.0)
    scene.set_object("/planets/jupiter/radiation", radiation_pc)

    # ── Voyager probe — escaping the solar system ────────────────────────
    scene.set_object("/voyager/body", slamd.geom.Sphere(0.12, (180, 180, 200)))
    scene.set_object("/voyager/triad", slamd.geom.Triad(scale=0.5, thickness=0.4))
    voyager_trail: list[tuple[float, float, float]] = []

    # ── Meteor shower ────────────────────────────────────────────────────
    met_pos, met_vel, met_phase, met_col, met_rad = make_meteor_shower(N_METEORS)
    met_base_pos = met_pos.copy()
    meteor_pc = slamd.geom.PointCloud(met_pos.copy(), met_col, met_rad, 1.0)
    scene.set_object("/meteors", meteor_pc)

    # ── Solar wind particles ─────────────────────────────────────────────
    sw_pos, sw_dirs, sw_col, sw_rad = make_solar_wind()
    solar_wind_pc = slamd.geom.PointCloud(sw_pos.copy(), sw_col, sw_rad, 1.0)
    scene.set_object("/sun/solar_wind", solar_wind_pc)

    # ── Black hole with accretion disk ─────────────────────────────────
    scene.set_object("/black_hole/singularity", slamd.geom.Sphere(0.5, (5, 5, 5)))
    bh_pos = BLACK_HOLE_POS
    scene.set_transform("/black_hole", pose_at(*bh_pos))
    acc_pos, acc_col, acc_rad, acc_angles, acc_radii, acc_vel = make_accretion_disk(
        N_ACCRETION, (0, 0, 0), BLACK_HOLE_MASS
    )
    accretion_pc = slamd.geom.PointCloud(acc_pos, acc_col, acc_rad, 1.0)
    scene.set_object("/black_hole/accretion", accretion_pc)

    # Gravitational lensing ring — a circle of bright points around the BH
    n_lens = 80
    lens_angles = np.linspace(0, 2 * np.pi, n_lens, endpoint=False)
    lens_r = BLACK_HOLE_MASS * 1.2
    lens_pos = np.zeros((n_lens, 3), dtype=np.float32)
    lens_pos[:, 0] = lens_r * np.cos(lens_angles)
    lens_pos[:, 1] = lens_r * np.sin(lens_angles)
    lens_col = np.ones((n_lens, 3), dtype=np.float32) * 0.9  # bright white
    lens_rad = np.full(n_lens, 0.06, dtype=np.float32)
    lens_pc = slamd.geom.PointCloud(lens_pos, lens_col, lens_rad, 1.0)
    scene.set_object("/black_hole/lensing", lens_pc)

    # ── The Monolith — mysterious black box orbiting Jupiter ─────────
    scene.set_object("/planets/jupiter/monolith/body", slamd.geom.Box())
    mono_scale = np.eye(4, dtype=np.float32)
    mono_scale[0, 0] = 0.05  # 1:4:9 ratio, like the movie
    mono_scale[1, 1] = 0.20
    mono_scale[2, 2] = 0.45
    scene.set_transform("/planets/jupiter/monolith/body", mono_scale)

    # ── Wormhole ─────────────────────────────────────────────────────
    wh_phase, wh_spiral, wh_base_r, wh_col, wh_rad = make_wormhole_particles(
        N_WORMHOLE_PARTICLES, WORMHOLE_ENTRY, WORMHOLE_EXIT
    )
    wh_pos = np.zeros((N_WORMHOLE_PARTICLES, 3), dtype=np.float32)
    wormhole_pc = slamd.geom.PointCloud(wh_pos, wh_col, wh_rad, 1.0)
    scene.set_object("/wormhole", wormhole_pc)
    # Wormhole mouth markers
    scene.set_object("/wormhole_entry", slamd.geom.Sphere(1.0, (80, 40, 180)))
    scene.set_transform("/wormhole_entry", pose_at(*WORMHOLE_ENTRY))
    scene.set_object("/wormhole_exit", slamd.geom.Sphere(1.0, (180, 40, 80)))
    scene.set_transform("/wormhole_exit", pose_at(*WORMHOLE_EXIT))

    # ── Alien ship — doing impossible maneuvers ──────────────────────
    scene.set_object("/alien/body", slamd.geom.Sphere(0.3, (0, 255, 100)))
    scene.set_object("/alien/glow", slamd.geom.Sphere(0.5, (0, 180, 60)))
    alien_trail: list[tuple[float, float, float]] = []

    # ── Space jellyfish — bioluminescent creatures ───────────────────
    for ji in range(N_JELLYFISH):
        jcolor = [(255, 50, 200), (50, 200, 255), (200, 255, 50), (255, 150, 50)][ji]
        scene.set_object(f"/jellyfish/{ji}/head", slamd.geom.Sphere(0.4, jcolor))

    # ── Space Whale — cosmic leviathan gliding through the void ─────────
    # Body is a big sphere, with smaller spheres for eyes and a "blowhole"
    scene.set_object("/whale/body", slamd.geom.Sphere(2.0, (60, 80, 120)))
    scene.set_object("/whale/eye_left", slamd.geom.Sphere(0.25, (255, 255, 200)))
    scene.set_object("/whale/eye_right", slamd.geom.Sphere(0.25, (255, 255, 200)))
    scene.set_transform("/whale/eye_left", pose_at(1.2, 0.8, 0.8))
    scene.set_transform("/whale/eye_right", pose_at(1.2, -0.8, 0.8))
    scene.set_object("/whale/tail", slamd.geom.Sphere(0.8, (50, 70, 110)))
    scene.set_transform("/whale/tail", pose_at(-2.5, 0, 0.3))
    # Whale breath particles
    whale_breath_pos = np.zeros((N_WHALE_BREATH, 3), dtype=np.float32)
    whale_breath_col = np.ones((N_WHALE_BREATH, 3), dtype=np.float32) * 0.7
    whale_breath_col[:, 2] = 1.0  # bluish
    whale_breath_rad = np.full(N_WHALE_BREATH, 0.08, dtype=np.float32)
    whale_breath_pc = slamd.geom.PointCloud(
        whale_breath_pos.copy(), whale_breath_col, whale_breath_rad, 1.0
    )
    scene.set_object("/whale/breath", whale_breath_pc)
    rng_whale = np.random.default_rng(88)
    whale_breath_dirs = rng_whale.normal(size=(N_WHALE_BREATH, 3)).astype(np.float32)
    whale_breath_dirs[:, 2] = np.abs(whale_breath_dirs[:, 2])  # upward
    whale_breath_dirs /= np.linalg.norm(whale_breath_dirs, axis=1, keepdims=True)
    whale_breath_phase = rng_whale.uniform(
        0, WHALE_BREATH_INTERVAL, N_WHALE_BREATH
    ).astype(np.float32)

    # ── Pulsar — spinning neutron star with particle beams ────────────
    scene.set_object("/pulsar/core", slamd.geom.Sphere(0.4, (200, 230, 255)))
    scene.set_transform("/pulsar", pose_at(*PULSAR_POS))
    pul_pos, pul_col, pul_rad, pul_beam_len, pul_dir = make_pulsar_beam(N_PULSAR_BEAM)
    pulsar_pc = slamd.geom.PointCloud(pul_pos.copy(), pul_col, pul_rad, 1.0)
    scene.set_object("/pulsar/beams", pulsar_pc)

    # ── Planet X (Nibiru) — rogue planet on extreme orbit ─────────────
    scene.set_object("/nibiru/body", slamd.geom.Sphere(0.6, (180, 30, 30)))
    scene.set_object("/nibiru/aura", slamd.geom.Sphere(0.9, (120, 20, 20)))
    nibiru_trail: list[tuple[float, float, float]] = []

    # ── Space Kraken — eldritch horror lurking in the outer system ────
    scene.set_object("/kraken/head", slamd.geom.Sphere(1.5, (40, 15, 50)))
    scene.set_object("/kraken/eye", slamd.geom.Sphere(0.6, (255, 0, 0)))
    scene.set_transform("/kraken/eye", pose_at(0.8, 0, 0.5))
    scene.set_transform("/kraken", pose_at(*KRAKEN_POS))
    kraken_t_param = make_kraken_tentacle(20)

    # ── Dyson Sphere skeleton — megastructure under construction ──────
    dyson_construction_progress = 0.0  # 0 to 1, grows over time
    for ai in range(N_DYSON_ARCS):
        arc_pts = make_dyson_arc(ai, N_DYSON_ARCS, DYSON_RADIUS)
        scene.set_object(
            f"/dyson/arc_{ai}",
            slamd.geom.PolyLine(arc_pts, 0.03, (180, 160, 60), 1.0),
        )

    # ── Quantum Tunneling Particles — teleporting randomly ────────────
    q_pos, q_col, q_rad = make_quantum_particles(N_QUANTUM)
    quantum_pc = slamd.geom.PointCloud(q_pos.copy(), q_col, q_rad, 1.0)
    scene.set_object("/quantum", quantum_pc)
    rng_quantum = np.random.default_rng(1337)
    q_last_teleport = np.zeros(N_QUANTUM, dtype=np.float32)

    # ── Cosmic Spiderweb — polylines connecting nearby bodies ─────────
    # Spider itself
    scene.set_object("/spider/body", slamd.geom.Sphere(0.35, (60, 60, 60)))
    scene.set_object("/spider/abdomen", slamd.geom.Sphere(0.5, (50, 45, 40)))
    scene.set_transform("/spider/abdomen", pose_at(-0.6, 0, 0))
    # 8 legs
    for leg_i in range(8):
        leg_pts = np.zeros((4, 3), dtype=np.float32)
        scene.set_object(
            f"/spider/leg_{leg_i}",
            slamd.geom.PolyLine(leg_pts, 0.02, (80, 70, 60), 1.0),
        )
    spider_web_anchors: list[tuple[float, float, float]] = []

    # ── Comets with tails ────────────────────────────────────────────────
    comet_trails: dict[str, list[tuple[float, float, float]]] = {}
    for cname, *_ in COMETS:
        scene.set_object(
            f"/comets/{cname}/head", slamd.geom.Sphere(0.15, (200, 220, 255))
        )
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
    probe_cam_rot[:3, :3] = (
        Rotation.from_euler("x", -90, degrees=True).as_matrix().astype(np.float32)
    )
    scene.set_transform("/probe/frustum", probe_cam_rot)

    # ── 2D radar canvas setup ────────────────────────────────────────────
    # Orbit rings on canvas
    for name, orbit_r, *_ in PLANETS:
        ring_pts_2d = np.zeros((128, 2), dtype=np.float32)
        a = np.linspace(0, 2 * np.pi, 128)
        scale_2d = 12.0  # pixels per AU-ish unit
        ring_pts_2d[:, 0] = orbit_r * scale_2d * np.cos(a)
        ring_pts_2d[:, 1] = orbit_r * scale_2d * np.sin(a)
        canvas.set_object(
            f"/orbits/{name}", slamd.geom2d.PolyLine(ring_pts_2d, (30, 45, 70), 1.0)
        )

    # Sun dot on radar
    canvas.set_object(
        "/sun",
        slamd.geom2d.Points(
            np.array([[0.0, 0.0]], dtype=np.float32),
            (255, 220, 50),
            8.0,
        ),
    )

    # Planet dots (will update positions each frame)
    n_planets = len(PLANETS)
    planet_2d_pos = np.zeros((n_planets, 2), dtype=np.float32)
    planet_2d_colors = np.array([p[4] for p in PLANETS], dtype=np.float32) / 255.0
    planet_2d_radii = np.array([p[2] * 6.0 for p in PLANETS], dtype=np.float32)
    canvas.set_object(
        "/planets",
        slamd.geom2d.Points(planet_2d_pos, planet_2d_colors, planet_2d_radii),
    )

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

            # Mars moons
            if name == "mars":
                for mname, morbit, msize, mspeed, mcolor in MARS_MOONS:
                    ma = mspeed * t
                    mx = morbit * np.cos(ma)
                    my = morbit * np.sin(ma)
                    scene.set_transform(f"/planets/{name}/{mname}", pose_at(mx, my))

        canvas.set_object(
            "/planets",
            slamd.geom2d.Points(
                planet_2d_pos.copy(),
                planet_2d_colors,
                planet_2d_radii,
            ),
        )

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

        # Update dwarf planets
        for dname, dorbit, dsize, dspeed, dcolor in DWARF_PLANETS:
            angle = dspeed * t * 0.4
            dx = dorbit * np.cos(angle)
            dy = dorbit * np.sin(angle)
            scene.set_transform(f"/dwarf_planets/{dname}", pose_at(dx, dy))

        # Slowly rotate Kuiper belt
        kui_new_angles = kui_angles + t * 0.005
        kui_pos[:, 0] = kui_orbit_r * np.cos(kui_new_angles)
        kui_pos[:, 1] = kui_orbit_r * np.sin(kui_new_angles)
        kuiper_pc.update_positions(kui_pos)

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
        canvas.set_object(
            "/probe",
            slamd.geom2d.Circles(
                np.array([[px * scale_2d, py * scale_2d]], dtype=np.float32),
                np.array([[0.8, 0.8, 1.0]], dtype=np.float32),
                np.array([5.0], dtype=np.float32),
                0.3,
            ),
        )

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
            canvas.set_object(
                "/probe_trail", slamd.geom2d.PolyLine(trail_pts, (120, 140, 200), 1.0)
            )

        # Comets on radar
        n_comets = len(COMETS)
        comet_2d_pos = np.zeros((n_comets, 2), dtype=np.float32)
        for ci, (cname, semi_major, ecc, cspeed, incl) in enumerate(COMETS):
            cx, cy, cz = comet_position(t, semi_major, ecc, cspeed, incl)
            comet_2d_pos[ci] = [cx * scale_2d, cy * scale_2d]
        canvas.set_object(
            "/comets_radar",
            slamd.geom2d.Points(
                comet_2d_pos,
                (150, 180, 255),
                3.0,
            ),
        )

        # Dwarf planets on radar
        n_dwarfs = len(DWARF_PLANETS)
        dwarf_2d_pos = np.zeros((n_dwarfs, 2), dtype=np.float32)
        for di, (dname, dorbit, dsize, dspeed, dcolor) in enumerate(DWARF_PLANETS):
            angle = dspeed * t * 0.4
            dwarf_2d_pos[di] = [
                dorbit * np.cos(angle) * scale_2d,
                dorbit * np.sin(angle) * scale_2d,
            ]
        canvas.set_object(
            "/dwarfs_radar",
            slamd.geom2d.Points(
                dwarf_2d_pos,
                np.array([d[4] for d in DWARF_PLANETS], dtype=np.float32) / 255.0,
                3.0,
            ),
        )

        # ISS orbiting Earth (in Earth's local frame)
        iss_angle = ISS_SPEED * t
        iss_x = ISS_ORBIT * np.cos(iss_angle)
        iss_y = ISS_ORBIT * np.sin(iss_angle)
        iss_z = 0.15 * np.sin(iss_angle * 3)  # slight wobble
        scene.set_transform("/planets/earth/iss", pose_at(iss_x, iss_y, iss_z))

        # Jupiter radiation belt — swirl the particles
        rad_new_theta = rad_theta + t * 0.8
        new_major_r = 1.2 * 2.5
        rad_pos[:, 0] = new_major_r * np.cos(rad_new_theta) + 0.3 * np.sin(
            t * 2 + rad_theta * 3
        )
        rad_pos[:, 1] = new_major_r * np.sin(rad_new_theta) + 0.3 * np.cos(
            t * 2 + rad_theta * 3
        )
        rad_pos[:, 2] = 0.7 * np.sin(rad_theta * 2 + t * 1.5)
        radiation_pc.update_positions(rad_pos)

        # Voyager — spiraling outward, escaping the system
        voy_r = 5.0 + VOYAGER_SPEED * t
        voy_angle = VOYAGER_SPIRAL_RATE * t
        voy_x = voy_r * np.cos(voy_angle)
        voy_y = voy_r * np.sin(voy_angle)
        voy_z = 2.0 * np.sin(t * 0.05)
        scene.set_transform("/voyager", pose_at(voy_x, voy_y, voy_z))

        # Voyager trail
        voyager_trail.append((voy_x, voy_y, voy_z))
        if len(voyager_trail) > 500:
            voyager_trail.pop(0)
        if len(voyager_trail) > 2:
            vt_pts = np.array(voyager_trail, dtype=np.float32)
            scene.set_object(
                "/voyager/trail",
                slamd.geom.PolyLine(vt_pts, 0.03, (100, 180, 100), 1.0),
            )

        # Voyager on radar
        canvas.set_object(
            "/voyager_radar",
            slamd.geom2d.Points(
                np.array([[voy_x * scale_2d, voy_y * scale_2d]], dtype=np.float32),
                (100, 180, 100),
                4.0,
            ),
        )

        # Meteor shower — streaking particles that cycle
        cycle = 15.0  # seconds per full cycle
        met_t = (t + met_phase) % cycle
        met_pos[:, 0] = met_base_pos[:, 0] + met_vel[:, 0] * met_t * METEOR_SPEED
        met_pos[:, 1] = met_base_pos[:, 1] + met_vel[:, 1] * met_t * METEOR_SPEED
        met_pos[:, 2] = met_base_pos[:, 2] + met_vel[:, 2] * met_t * METEOR_SPEED
        # Fade out as they travel
        fade = np.clip(1.0 - met_t / cycle, 0.0, 1.0).astype(np.float32)
        meteor_pc.update_positions(met_pos)
        meteor_pc.update_radii(met_rad * fade)

        # Solar wind — expanding shells from the sun
        n_shells = 5
        ppsh = 40
        max_r = 6.0
        shell_period = 3.0  # seconds per shell cycle
        for si in range(n_shells):
            shell_t = (t + si * shell_period / n_shells) % shell_period
            r = 2.0 + (max_r - 2.0) * (shell_t / shell_period)
            fade_sw = 1.0 - shell_t / shell_period
            idx_start = si * ppsh
            idx_end = idx_start + ppsh
            sw_pos[idx_start:idx_end] = sw_dirs[idx_start:idx_end] * r
            sw_rad[idx_start:idx_end] = 0.04 * fade_sw
        solar_wind_pc.update_positions(sw_pos)
        solar_wind_pc.update_radii(sw_rad)

        # Black hole accretion disk — Keplerian rotation
        new_acc_angles = acc_angles + acc_vel * t * 2.0
        acc_pos[:, 0] = acc_radii * np.cos(new_acc_angles)
        acc_pos[:, 1] = acc_radii * np.sin(new_acc_angles)
        # Wobble the disk plane
        acc_pos[:, 2] = (
            0.15 * np.sin(new_acc_angles * 3 + t) * (acc_radii / acc_radii.max())
        )
        accretion_pc.update_positions(acc_pos)

        # Lensing ring shimmer
        shimmer = 0.7 + 0.3 * np.sin(t * 5.0 + lens_angles * 4)
        lens_rad[:] = 0.06 * shimmer.astype(np.float32)
        lens_pc.update_radii(lens_rad)

        # The Monolith — slowly rotating, orbiting Jupiter
        mono_angle = MONOLITH_SPEED * t
        mx = MONOLITH_ORBIT * np.cos(mono_angle)
        my = MONOLITH_ORBIT * np.sin(mono_angle)
        mz = 1.5 * np.sin(mono_angle * 0.3)
        mono_T = pose_at(mx, my, mz)
        # Tumble rotation
        tumble = (
            Rotation.from_euler("xyz", [t * 0.5, t * 0.3, t * 0.7])
            .as_matrix()
            .astype(np.float32)
        )
        mono_T[:3, :3] = tumble
        scene.set_transform("/planets/jupiter/monolith", mono_T)

        # Wormhole — particles spiraling through a tunnel between two points
        entry = np.array(WORMHOLE_ENTRY, dtype=np.float32)
        exit_pt = np.array(WORMHOLE_EXIT, dtype=np.float32)
        wh_t = (wh_phase + t * 0.15) % 1.0  # each particle slides along tunnel
        # Interpolate position along the tunnel
        midpoint = (entry + exit_pt) / 2 + np.array([0, 0, 12], dtype=np.float32)
        # Bezier curve: entry → midpoint → exit
        p0 = entry[None, :]
        p1 = midpoint[None, :]
        p2 = exit_pt[None, :]
        tt = wh_t[:, None]
        bezier = (1 - tt) ** 2 * p0 + 2 * (1 - tt) * tt * p1 + tt**2 * p2
        # Add spiral around the path
        tunnel_r = wh_base_r * (0.3 + 0.7 * np.sin(np.pi * wh_t))  # wide at middle
        spiral_a = wh_spiral + t * 4.0 + wh_t * 20.0
        wh_pos[:, 0] = bezier[:, 0] + tunnel_r * np.cos(spiral_a)
        wh_pos[:, 1] = bezier[:, 1] + tunnel_r * np.sin(spiral_a)
        wh_pos[:, 2] = bezier[:, 2] + tunnel_r * np.sin(spiral_a * 0.7)
        wormhole_pc.update_positions(wh_pos)
        # Pulse the wormhole colors
        pulse = 0.6 + 0.4 * np.sin(t * 3.0 + wh_t * 10)
        wh_rad[:] = 0.05 * pulse.astype(np.float32)
        wormhole_pc.update_radii(wh_rad)

        # Alien ship — chaotic Lissajous maneuvers
        ax = 20.0 * np.sin(ALIEN_SPEED * t * 2.3 + 1.0) * np.cos(ALIEN_SPEED * t * 0.7)
        ay = 18.0 * np.cos(ALIEN_SPEED * t * 1.7) * np.sin(ALIEN_SPEED * t * 1.1 + 2.0)
        az = 8.0 * np.sin(ALIEN_SPEED * t * 3.1 + 0.5) * np.cos(ALIEN_SPEED * t * 0.9)
        scene.set_transform("/alien", pose_at(ax, ay, az))
        # Trail
        alien_trail.append((ax, ay, az))
        if len(alien_trail) > 200:
            alien_trail.pop(0)
        if len(alien_trail) > 2:
            at_pts = np.array(alien_trail, dtype=np.float32)
            scene.set_object(
                "/alien/trail", slamd.geom.PolyLine(at_pts, 0.04, (0, 255, 100), 1.0)
            )

        # Space jellyfish — drifting and pulsating
        for ji in range(N_JELLYFISH):
            jt = t * 0.1 + ji * 7.0
            jx = 25.0 * np.sin(jt * 0.3 + ji * 1.5)
            jy = 20.0 * np.cos(jt * 0.2 + ji * 2.3)
            jz = 8.0 * np.sin(jt * 0.15 + ji) + 5.0
            scene.set_transform(f"/jellyfish/{ji}", pose_at(jx, jy, jz))
            # Tentacles — wavy polylines dangling down
            n_tentacles = 5
            for ti in range(n_tentacles):
                tent_pts = np.zeros((12, 3), dtype=np.float32)
                for si in range(12):
                    frac = si / 11.0
                    tent_pts[si, 0] = 0.8 * np.sin(t * 2.0 + frac * 4 + ti * 1.2) * frac
                    tent_pts[si, 1] = 0.8 * np.cos(t * 1.5 + frac * 3 + ti * 0.9) * frac
                    tent_pts[si, 2] = -frac * (2.0 + 0.5 * np.sin(t * 1.8 + ti))
                jcolor_line = [
                    (255, 50, 200),
                    (50, 200, 255),
                    (200, 255, 50),
                    (255, 150, 50),
                ][ji]
                scene.set_object(
                    f"/jellyfish/{ji}/tentacle_{ti}",
                    slamd.geom.PolyLine(tent_pts, 0.03, jcolor_line, 1.0),
                )

        # ── Space Whale — gliding through the cosmos ────────────────────
        whale_t = t * WHALE_SPEED
        wx = 30.0 * np.sin(whale_t * 0.7)
        wy = 25.0 * np.cos(whale_t * 0.5)
        wz = 10.0 * np.sin(whale_t * 0.3) + 6.0
        whale_T = pose_at(wx, wy, wz)
        # Whale faces its direction of travel
        whale_fwd = np.array(
            [
                30.0 * 0.7 * np.cos(whale_t * 0.7),
                -25.0 * 0.5 * np.sin(whale_t * 0.5),
                10.0 * 0.3 * np.cos(whale_t * 0.3),
            ],
            dtype=np.float32,
        )
        whale_fwd /= np.linalg.norm(whale_fwd) + 1e-8
        w_up = np.array([0, 0, 1], dtype=np.float32)
        w_right = np.cross(whale_fwd, w_up)
        w_right /= np.linalg.norm(w_right) + 1e-8
        w_up = np.cross(w_right, whale_fwd)
        whale_T[:3, 0] = whale_fwd
        whale_T[:3, 1] = w_right
        whale_T[:3, 2] = w_up
        scene.set_transform("/whale", whale_T)
        # Whale breathing particles — periodic bursts upward
        breath_cycle = t % WHALE_BREATH_INTERVAL
        breath_t = (breath_cycle + whale_breath_phase) % WHALE_BREATH_INTERVAL
        breath_active = breath_t < 3.0  # burst lasts 3 seconds
        breath_frac = np.where(breath_active, breath_t / 3.0, 0.0).astype(np.float32)
        whale_breath_pos[:, 0] = whale_breath_dirs[:, 0] * breath_frac * 4.0
        whale_breath_pos[:, 1] = whale_breath_dirs[:, 1] * breath_frac * 4.0
        whale_breath_pos[:, 2] = whale_breath_dirs[:, 2] * breath_frac * 6.0 + 1.5
        whale_breath_rad[:] = np.where(
            breath_active, 0.08 * (1.0 - breath_frac), 0.0
        ).astype(np.float32)
        whale_breath_pc.update_positions(whale_breath_pos)
        whale_breath_pc.update_radii(whale_breath_rad)

        # ── Pulsar — spinning beams ──────────────────────────────────────
        spin_angle = PULSAR_SPIN * t * 2 * np.pi
        cos_s, sin_s = np.cos(spin_angle), np.sin(spin_angle)
        # Rotate beam around Z axis
        pul_pos[:, 0] = (-pul_beam_len * 0.15 * np.cos(pul_beam_len * 0.5)) * cos_s
        pul_pos[:, 1] = (-pul_beam_len * 0.15 * np.cos(pul_beam_len * 0.5)) * sin_s
        pul_pos[:, 2] = pul_beam_len * pul_dir
        # Wobble the beam axis over time
        wobble = 0.3 * np.sin(t * 1.5)
        pul_pos[:, 0] += pul_dir * pul_beam_len * wobble * 0.1
        pulsar_pc.update_positions(pul_pos)
        # Pulsar core on radar
        canvas.set_object(
            "/pulsar_radar",
            slamd.geom2d.Points(
                np.array(
                    [[PULSAR_POS[0] * scale_2d, PULSAR_POS[1] * scale_2d]],
                    dtype=np.float32,
                ),
                (200, 230, 255),
                4.0,
            ),
        )

        # ── Planet X (Nibiru) — extreme elliptical orbit ─────────────────
        nib_x, nib_y, nib_z = comet_position(
            t, NIBIRU_SEMI_MAJOR, NIBIRU_ECC, NIBIRU_SPEED, NIBIRU_INCL
        )
        scene.set_transform("/nibiru", pose_at(nib_x, nib_y, nib_z))
        nibiru_trail.append((nib_x, nib_y, nib_z))
        if len(nibiru_trail) > 600:
            nibiru_trail.pop(0)
        if len(nibiru_trail) > 2:
            nib_pts = np.array(nibiru_trail, dtype=np.float32)
            scene.set_object(
                "/nibiru/trail", slamd.geom.PolyLine(nib_pts, 0.05, (180, 30, 30), 1.0)
            )

        # ── Space Kraken — writhing tentacles ────────────────────────────
        # Kraken head bobs menacingly
        kraken_bob = np.sin(t * 0.5) * 2.0
        scene.set_transform(
            "/kraken", pose_at(KRAKEN_POS[0], KRAKEN_POS[1], KRAKEN_POS[2] + kraken_bob)
        )
        for ti in range(N_KRAKEN_TENTACLES):
            tent_angle = ti * 2 * np.pi / N_KRAKEN_TENTACLES
            n_seg = 20
            tent_pts = np.zeros((n_seg, 3), dtype=np.float32)
            for si in range(n_seg):
                frac = si / (n_seg - 1)
                reach = KRAKEN_REACH * frac
                # Each tentacle waves independently
                wave1 = np.sin(t * 1.2 + frac * 5.0 + ti * 0.8) * frac * 2.0
                wave2 = np.cos(t * 0.9 + frac * 4.0 + ti * 1.1) * frac * 1.5
                base_x = reach * np.cos(tent_angle)
                base_y = reach * np.sin(tent_angle)
                tent_pts[si, 0] = base_x + wave1 * np.sin(tent_angle)
                tent_pts[si, 1] = base_y + wave1 * np.cos(tent_angle)
                tent_pts[si, 2] = -frac * 3.0 + wave2
            # Tentacles get redder at the tips
            tip_r = int(120 + 80 * np.sin(t + ti))
            scene.set_object(
                f"/kraken/tentacle_{ti}",
                slamd.geom.PolyLine(tent_pts, 0.08, (tip_r, 20, 60), 1.0),
            )

        # ── Dyson Sphere — slowly rotating construction ──────────────────
        dyson_construction_progress = min(1.0, t / 300.0)  # takes 5 min to "complete"
        dyson_rot = np.eye(4, dtype=np.float32)
        dyson_rot[:3, :3] = (
            Rotation.from_euler("xyz", [t * 0.02, t * 0.015, t * 0.01])
            .as_matrix()
            .astype(np.float32)
        )
        scene.set_transform("/dyson", dyson_rot)
        # Update visible arcs based on construction progress
        n_visible = max(1, int(N_DYSON_ARCS * dyson_construction_progress))
        for ai in range(N_DYSON_ARCS):
            if ai < n_visible:
                # Shimmer the arcs that are "complete"
                shimmer_color = int(160 + 40 * np.sin(t * 2.0 + ai * 0.5))
                arc_pts = make_dyson_arc(ai, N_DYSON_ARCS, DYSON_RADIUS)
                scene.set_object(
                    f"/dyson/arc_{ai}",
                    slamd.geom.PolyLine(
                        arc_pts, 0.03, (shimmer_color, shimmer_color - 20, 60), 1.0
                    ),
                )

        # ── Quantum Particles — random teleportation ─────────────────────
        # Each frame, some particles quantum-tunnel to new locations
        teleport_mask = rng_quantum.random(N_QUANTUM) < (QUANTUM_TELEPORT_RATE * 0.016)
        n_teleport = teleport_mask.sum()
        if n_teleport > 0:
            q_pos[teleport_mask, 0] = rng_quantum.uniform(-30, 30, n_teleport).astype(
                np.float32
            )
            q_pos[teleport_mask, 1] = rng_quantum.uniform(-30, 30, n_teleport).astype(
                np.float32
            )
            q_pos[teleport_mask, 2] = rng_quantum.uniform(-5, 5, n_teleport).astype(
                np.float32
            )
        # Non-teleporting particles vibrate in place (uncertainty principle)
        q_pos[:, 0] += rng_quantum.normal(0, 0.02, N_QUANTUM).astype(np.float32)
        q_pos[:, 1] += rng_quantum.normal(0, 0.02, N_QUANTUM).astype(np.float32)
        q_pos[:, 2] += rng_quantum.normal(0, 0.01, N_QUANTUM).astype(np.float32)
        # Flash particles that just teleported
        quantum_pc.update_positions(q_pos)
        flash_rad = q_rad.copy()
        flash_rad[teleport_mask] *= 3.0  # brief flash on teleport
        quantum_pc.update_radii(flash_rad)

        # ── Cosmic Spider — weaving between planets ──────────────────────
        spider_t = t * SPIDER_SPEED
        # Spider orbits in the asteroid belt region, weaving up and down
        sp_r = 14.5 + 1.0 * np.sin(spider_t * 3.0)
        sp_x = sp_r * np.cos(spider_t)
        sp_y = sp_r * np.sin(spider_t)
        sp_z = 3.0 * np.sin(spider_t * 2.3)
        scene.set_transform("/spider", pose_at(sp_x, sp_y, sp_z))
        # Animate legs — each leg is a jointed polyline
        for leg_i in range(8):
            leg_side = 1.0 if leg_i < 4 else -1.0
            leg_idx = leg_i % 4
            leg_angle = (leg_idx - 1.5) * 0.6  # spread along body
            leg_pts = np.zeros((4, 3), dtype=np.float32)
            # Base
            leg_pts[0] = [0.0, leg_side * 0.2, 0.0]
            # Shoulder
            walk_phase = np.sin(t * 4.0 + leg_i * 0.8)
            leg_pts[1] = [
                0.4 * np.sin(leg_angle),
                leg_side * 0.8,
                0.3 * walk_phase,
            ]
            # Knee
            leg_pts[2] = [
                0.6 * np.sin(leg_angle),
                leg_side * 1.2,
                -0.3 + 0.2 * walk_phase,
            ]
            # Foot
            leg_pts[3] = [
                0.5 * np.sin(leg_angle),
                leg_side * 1.4,
                -0.6,
            ]
            scene.set_object(
                f"/spider/leg_{leg_i}",
                slamd.geom.PolyLine(leg_pts, 0.02, (80, 70, 60), 1.0),
            )

        # Spider leaves a web trail
        spider_web_anchors.append((sp_x, sp_y, sp_z))
        if len(spider_web_anchors) > 800:
            spider_web_anchors.pop(0)
        if len(spider_web_anchors) > 2 and len(spider_web_anchors) % 5 == 0:
            web_pts = np.array(spider_web_anchors[-200:], dtype=np.float32)
            scene.set_object(
                "/spider/web", slamd.geom.PolyLine(web_pts, 0.01, (200, 200, 200), 1.0)
            )

        # Twinkling stars — subtle radius oscillation
        twinkle = 1.0 + 0.3 * np.sin(t * 3.0 + star_pos[:, 0] * 0.5)
        star_pc.update_radii(star_base_rad * twinkle)

        # Pulsing sun corona — rotate arrows
        rot_corona = np.eye(4, dtype=np.float32)
        rot_corona[:3, :3] = (
            Rotation.from_euler("z", t * 0.3).as_matrix().astype(np.float32)
        )
        scene.set_transform("/sun/corona", rot_corona)

        # Nibiru & Kraken on radar
        canvas.set_object(
            "/nibiru_radar",
            slamd.geom2d.Points(
                np.array([[nib_x * scale_2d, nib_y * scale_2d]], dtype=np.float32),
                (180, 30, 30),
                5.0,
            ),
        )
        canvas.set_object(
            "/whale_radar",
            slamd.geom2d.Points(
                np.array([[wx * scale_2d, wy * scale_2d]], dtype=np.float32),
                (60, 80, 120),
                6.0,
            ),
        )
        canvas.set_object(
            "/spider_radar",
            slamd.geom2d.Points(
                np.array([[sp_x * scale_2d, sp_y * scale_2d]], dtype=np.float32),
                (60, 60, 60),
                3.0,
            ),
        )


if __name__ == "__main__":
    main()
