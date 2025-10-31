# ====================================================
# M2: Flow Sensor Test — Plot Script (Dual Y-Axis)
# Purpose: Visualize measured flow rate (Hz → L/min)
# Data: tests/data/m2_flow_sensor_test_data.csv
# ====================================================

import matplotlib.pyplot as plt
from pathlib import Path
import pandas as pd

# Path to your data file
base_dir = Path(__file__).resolve().parents[2]
path = base_dir / "tests" / "data" / "m2_flow_sensor_test_data.csv"
df = pd.read_csv(path)

# Convert milliseconds → seconds
df["t_s"] = df["t_ms"] / 1000.0

# Create figure and primary axis
fig, ax1 = plt.subplots(figsize=(8, 4))

# Left Y-axis: Frequency (Hz)
color1 = "tab:blue"
ax1.set_xlabel("Time (s)")
ax1.set_ylabel("Pulse Frequency (Hz)", color=color1)
ax1.plot(df["t_s"], df["Hz"], color=color1, label="Frequency (Hz)", alpha=0.7)
ax1.tick_params(axis="y", labelcolor=color1)
ax1.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.7)

# Right Y-axis: Flow Rate (L/min)
ax2 = ax1.twinx()
color2 = "tab:orange"
ax2.set_ylabel("Flow Rate (L/min)", color=color2)
ax2.plot(df["t_s"], df["L_per_min"], color=color2, label="Flow Rate (L/min)", linewidth=2)
ax2.tick_params(axis="y", labelcolor=color2)

# Title and layout
plt.title("Flow Sensor Output — Frequency and Flow Rate (Uncalibrated)")
fig.tight_layout()
plt.show()
