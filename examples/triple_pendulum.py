"""
Triple pendulum — three-link chaotic motion via Lagrangian mechanics.

The equations of motion are derived from the Lagrangian L = T - V for three
point masses on rigid, massless rods.  The resulting 3x3 mass matrix M(q) is
assembled and solved at every step:  M * q_ddot = f(q, q_dot).
"""

import slamd
import numpy as np
import time

G = 9.81
L1, L2, L3 = 1.2, 1.0, 0.8
M1, M2, M3 = 1.0, 1.0, 1.0


def derivs(state):
    t1, w1, t2, w2, t3, w3 = state

    c12 = np.cos(t1 - t2)
    c13 = np.cos(t1 - t3)
    c23 = np.cos(t2 - t3)
    s12 = np.sin(t1 - t2)
    s13 = np.sin(t1 - t3)
    s23 = np.sin(t2 - t3)

    # Mass matrix M(q) from the Lagrangian.
    # M_ij = sum_k m_k * l_i * l_j * cos(t_i - t_j)  for links that affect mass k.
    m11 = (M1 + M2 + M3) * L1**2
    m12 = (M2 + M3) * L1 * L2 * c12
    m13 = M3 * L1 * L3 * c13
    m22 = (M2 + M3) * L2**2
    m23 = M3 * L2 * L3 * c23
    m33 = M3 * L3**2

    # Right-hand side: Coriolis / centrifugal + gravity terms.
    f1 = (-(M2 + M3) * L1 * L2 * s12 * w2**2
          - M3 * L1 * L3 * s13 * w3**2
          - (M1 + M2 + M3) * G * L1 * np.sin(t1))

    f2 = ((M2 + M3) * L1 * L2 * s12 * w1**2
          - M3 * L2 * L3 * s23 * w3**2
          - (M2 + M3) * G * L2 * np.sin(t2))

    f3 = (M3 * L1 * L3 * s13 * w1**2
          + M3 * L2 * L3 * s23 * w2**2
          - M3 * G * L3 * np.sin(t3))

    # Solve M * [a1, a2, a3]^T = [f1, f2, f3]^T
    M = np.array([
        [m11, m12, m13],
        [m12, m22, m23],
        [m13, m23, m33],
    ])
    f = np.array([f1, f2, f3])
    a1, a2, a3 = np.linalg.solve(M, f)

    return np.array([w1, a1, w2, a2, w3, a3])


def rk4(state, dt):
    k1 = derivs(state)
    k2 = derivs(state + 0.5 * dt * k1)
    k3 = derivs(state + 0.5 * dt * k2)
    k4 = derivs(state + dt * k3)
    return state + (dt / 6.0) * (k1 + 2 * k2 + 2 * k3 + k4)


def angles_to_xy(t1, t2, t3):
    x1 = L1 * np.sin(t1)
    y1 = -L1 * np.cos(t1)
    x2 = x1 + L2 * np.sin(t2)
    y2 = y1 - L2 * np.cos(t2)
    x3 = x2 + L3 * np.sin(t3)
    y3 = y2 - L3 * np.cos(t3)
    return (x1, y1), (x2, y2), (x3, y3)


def main():
    vis = slamd.Visualizer("Triple Pendulum", spawn=True, port=6001)
    scene = vis.scene("scene")

    # Start with large angles for chaotic behaviour.
    state = np.array([2.5, 0.0, 3.0, 0.0, 2.0, 0.0])
    dt = 0.001
    trail = []
    max_trail = 200

    spheres = None

    while True:
        for _ in range(15):
            state = rk4(state, dt)

        p1, p2, p3 = angles_to_xy(state[0], state[2], state[4])
        trail.append([p3[0], 0.0, p3[1]])
        if len(trail) > max_trail:
            trail = trail[-max_trail:]

        positions = np.array([
            [p1[0], 0, p1[1]],
            [p2[0], 0, p2[1]],
            [p3[0], 0, p3[1]],
        ], dtype=np.float32)
        colors = np.array([
            [1.0, 0.2, 0.1],
            [0.2, 0.6, 1.0],
            [0.1, 1.0, 0.3],
        ], dtype=np.float32)
        radii = np.array([0.10, 0.10, 0.10], dtype=np.float32)

        if spheres is None:
            spheres = slamd.geom.Spheres(positions, colors, radii, 0.3)
            scene.set_object("/masses", spheres)
        else:
            spheres.update_positions(positions)

        arm_pts = np.array([
            [0, 0, 0],
            [p1[0], 0, p1[1]],
            [p2[0], 0, p2[1]],
            [p3[0], 0, p3[1]],
        ], dtype=np.float32)
        arm = slamd.geom.PolyLine(arm_pts, 0.03, np.array([0.9, 0.9, 0.9], dtype=np.float32), 0.5)
        scene.set_object("/arm", arm)

        if len(trail) > 2:
            pts = np.array(trail, dtype=np.float32)
            trail_obj = slamd.geom.PolyLine(pts, 0.015, np.array([1.0, 0.7, 0.1], dtype=np.float32), 0.3)
            scene.set_object("/trail", trail_obj)

        time.sleep(0.01)


if __name__ == "__main__":
    main()
