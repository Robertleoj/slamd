"""
Double pendulum — chaotic motion with a fading trail.
"""

import slamd
import numpy as np
import time

G = 9.81
L1, L2 = 1.5, 1.5
M1, M2 = 1.0, 1.0


def derivs(state):
    t1, w1, t2, w2 = state
    delta = t1 - t2
    sd, cd = np.sin(delta), np.cos(delta)
    den = 2 * M1 + M2 - M2 * np.cos(2 * delta)

    a1 = (-G * (2 * M1 + M2) * np.sin(t1)
           - M2 * G * np.sin(t1 - 2 * t2)
           - 2 * sd * M2 * (w2**2 * L2 + w1**2 * L1 * cd)) / (L1 * den)

    a2 = (2 * sd * (w1**2 * L1 * (M1 + M2)
           + G * (M1 + M2) * np.cos(t1)
           + w2**2 * L2 * M2 * cd)) / (L2 * den)

    return np.array([w1, a1, w2, a2])


def rk4(state, dt):
    k1 = derivs(state)
    k2 = derivs(state + 0.5 * dt * k1)
    k3 = derivs(state + 0.5 * dt * k2)
    k4 = derivs(state + dt * k3)
    return state + (dt / 6.0) * (k1 + 2 * k2 + 2 * k3 + k4)


def angles_to_xy(t1, t2):
    x1 = L1 * np.sin(t1)
    y1 = -L1 * np.cos(t1)
    x2 = x1 + L2 * np.sin(t2)
    y2 = y1 - L2 * np.cos(t2)
    return (x1, y1), (x2, y2)


def main():
    vis = slamd.Visualizer("Double Pendulum", spawn=True, port=6001)
    scene = vis.scene("scene")

    state = np.array([2.8, 0.0, 3.0, 0.0])  # high angles -> chaos
    dt = 0.002
    trail = []
    max_trail = 200

    spheres = None
    arm_line = None
    trail_line = None

    while True:
        for _ in range(10):
            state = rk4(state, dt)

        p1, p2 = angles_to_xy(state[0], state[2])
        trail.append([p2[0], 0.0, p2[1]])
        if len(trail) > max_trail:
            trail = trail[-max_trail:]

        positions = np.array([[p1[0], 0, p1[1]], [p2[0], 0, p2[1]]], dtype=np.float32)
        colors = np.array([[1.0, 0.2, 0.1], [0.2, 0.6, 1.0]], dtype=np.float32)
        radii = np.array([0.12, 0.12], dtype=np.float32)

        if spheres is None:
            spheres = slamd.geom.Spheres(positions, colors, radii, 0.3)
            scene.set_object("/masses", spheres)
        else:
            spheres.update_positions(positions)

        arm_pts = np.array([[0, 0, 0], [p1[0], 0, p1[1]], [p2[0], 0, p2[1]]], dtype=np.float32)
        arm = slamd.geom.PolyLine(arm_pts, 0.04, np.array([0.9, 0.9, 0.9], dtype=np.float32), 0.5)
        scene.set_object("/arm", arm)

        if len(trail) > 2:
            pts = np.array(trail, dtype=np.float32)
            trail_obj = slamd.geom.PolyLine(pts, 0.02, np.array([1.0, 0.7, 0.1], dtype=np.float32), 0.3)
            scene.set_object("/trail", trail_obj)

        time.sleep(0.01)


if __name__ == "__main__":
    main()
