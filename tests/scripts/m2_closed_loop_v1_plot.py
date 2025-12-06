# ====================================================
# M2: Closed Loop V1 — PID Response Plot
# Purpose: Visualize PID response (outlet vs set temperature)
# Data: tests/data/m2_closed_loop_v1_data.csv
# ====================================================

from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd


base_dir = Path(__file__).resolve().parents[2]
data_path = base_dir / "tests" / "data" / "m2_closed_loop_v1_data.csv"

df = pd.read_csv(data_path)
df["t_s"] = df["t_ms"] / 1000.0  # milliseconds → seconds

fig, ax = plt.subplots(figsize=(12, 6))

# Outlet temperature trajectory (PID response)
ax.plot(
    df["t_s"],
    df["out_f"],
    label="Outlet Temperature",
    color="#d08c00",
    linewidth=2.5,
)

# Set temperature (constant step)
ax.plot(
    df["t_s"],
    df["set_f"],
    label="Set Temperature",
    color="#3a8df4",
    linewidth=2,
)

ax.set_xlabel("Time (s)", fontsize=16)
ax.set_ylabel("Temperature (°F)", fontsize=16)
ax.set_title("PID Response (Kp=0.1, Ki=0.08, Kd=0)", fontsize=24, pad=16)
ax.grid(True, linestyle="--", linewidth=0.8, alpha=0.6)
ax.legend(loc="upper right", frameon=True, framealpha=0.9)
ax.tick_params(labelsize=12)

fig.tight_layout()
plt.show()
