# ====================================================
# M2: Outlet Temperature Test — Plot Script
# Purpose: Visualize raw vs filtered outlet temperature
# Data: tests/data/m2_outlet_temp_test_data.csv
# ====================================================

import matplotlib.pyplot as plt
from pathlib import Path
import pandas as pd

# Path to your data file
base_dir = Path(__file__).resolve().parents[2]
path = base_dir / "tests" / "data" / "m2_outlet_temp_test_data.csv"
df = pd.read_csv(path)

# Convert milliseconds → seconds
df["t_s"] = df["t_ms"] / 1000.0

# Plot
plt.figure(figsize=(8, 4))
plt.plot(df["t_s"], df["T_out_raw"], label="Raw (°F)", alpha=0.6)
plt.plot(df["t_s"], df["T_out_filt"], label="Filtered (°F)", linewidth=2)
plt.xlabel("Time (s)")
plt.ylabel("Temperature (°F)")
plt.title("Outlet Temperature Read Loop + EMA Filter")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()
