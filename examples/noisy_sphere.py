import slamd
import time
import numpy as np


def uv_sphere(subdivisions: int = 5) -> tuple[np.ndarray, list[int]]:
    n_lat = 2 ** (subdivisions + 1)
    n_lon = 2 * n_lat

    vertices = []
    indices = []

    for i in range(n_lat + 1):
        theta = np.pi * i / n_lat
        for j in range(n_lon + 1):
            phi = 2.0 * np.pi * j / n_lon
            x = np.sin(theta) * np.cos(phi)
            y = np.sin(theta) * np.sin(phi)
            z = np.cos(theta)
            vertices.append([x, y, z])

    for i in range(n_lat):
        for j in range(n_lon):
            p0 = i * (n_lon + 1) + j
            p1 = p0 + 1
            p2 = p0 + (n_lon + 1)
            p3 = p2 + 1
            indices.extend([p0, p2, p1])
            indices.extend([p1, p2, p3])

    return np.array(vertices, dtype=np.float32), indices


def snoise(
    positions: np.ndarray, t: float, freq: float, speed: float, phase: float = 0.0
) -> np.ndarray:
    x, y, z = positions[:, 0], positions[:, 1], positions[:, 2]
    return (
        np.sin(freq * x + speed * t + phase)
        * np.sin(freq * y - 0.7 * speed * t + phase * 1.3)
        * np.cos(freq * z + 0.3 * speed * t + phase * 0.7)
    )


def main():
    vis = slamd.Visualizer("noisy sphere", port=6000)

    scene = slamd.Scene()
    vis.add_scene("scene", scene)

    base_verts, indices = uv_sphere(subdivisions=5)
    radius = 5.0
    normals = base_verts.copy()

    # Precompute spherical coords for color effects
    theta = np.arccos(np.clip(base_verts[:, 2], -1, 1))
    phi = np.arctan2(base_verts[:, 1], base_verts[:, 0])

    mesh = None
    t0 = time.monotonic()

    while True:
        time.sleep(0.01)
        t = time.monotonic() - t0

        # Breathing base radius
        breath = 1.0 + 0.15 * np.sin(0.5 * t)

        # Displacement: many octaves with drifting phases for chaotic feel
        d = (
            0.50 * snoise(base_verts, t, freq=2.0, speed=0.8)
            + 0.30 * snoise(base_verts, t, freq=4.0, speed=1.3, phase=np.sin(0.2 * t))
            + 0.20 * snoise(base_verts, t, freq=7.0, speed=1.9, phase=np.cos(0.3 * t))
            + 0.10 * snoise(base_verts, t, freq=13.0, speed=2.7, phase=t * 0.1)
            + 0.05 * snoise(base_verts, t, freq=23.0, speed=3.5)
        )

        # Warp: let displacement feed back into itself for gnarly topology
        warp = 0.4 * np.sin(3.0 * d + 1.5 * t)
        d_final = d + warp

        points = (radius * breath + d_final[:, None]) * normals

        # --- Psychedelic color ---
        nd = d_final / (np.abs(d_final).max() + 1e-8)

        # Rotating hue based on spherical position + time
        hue_shift = phi + 0.8 * t + 2.0 * nd
        saturation_wave = theta * 2.0 + 0.5 * t

        # HSV-ish via sine waves = trippy rainbow
        r = 0.5 + 0.5 * np.sin(hue_shift)
        g = 0.5 + 0.5 * np.sin(hue_shift + 2.094)  # +120 deg
        b = 0.5 + 0.5 * np.sin(hue_shift + 4.189)  # +240 deg

        # Iridescent shimmer tied to displacement peaks
        shimmer = 0.4 * np.sin(10.0 * nd + 3.0 * t) * np.sin(saturation_wave)
        r += shimmer
        g += shimmer * 0.6
        b -= shimmer * 0.3

        # Deep glow in the valleys, hot highlights on peaks
        glow = np.exp(-2.0 * (nd + 0.5) ** 2)  # gaussian centered on valleys
        r += 0.3 * glow * np.sin(t * 1.1)
        g += 0.1 * glow
        b += 0.5 * glow * np.cos(t * 0.7)

        # Pulsing veins along latitude lines
        veins = 0.3 * (np.sin(12.0 * theta + 4.0 * d + 1.5 * t) ** 8)
        g += veins
        r += veins * 0.3

        colors = np.stack([r, g, b], axis=1).astype(np.float32)
        np.clip(colors, 0.0, 1.0, out=colors)

        if mesh is None:
            mesh = slamd.geom.Mesh(points, colors, indices)
            scene.set_object("/sphere", mesh)
        else:
            mesh.update_positions(points)
            mesh.update_colors(colors)


if __name__ == "__main__":
    main()
