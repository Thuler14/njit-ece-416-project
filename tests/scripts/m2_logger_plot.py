# ====================================================
# M2: Logger Sample Plot
# Purpose: Visualize control + flow telemetry from m2_logger_sample.csv
# Data: tests/data/m2_logger_sample.csv
# ====================================================

from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd


def main() -> None:
  base_dir = Path(__file__).resolve().parents[2]
  data_path = base_dir / "tests" / "data" / "m2_logger_sample.csv"

  df = pd.read_csv(data_path)
  df["t_s"] = df["ms"] / 1000.0

  fig, (ax_temp, ax_ctrl) = plt.subplots(2, 1, figsize=(10, 6), sharex=True)

  # Outlet temperature vs setpoint
  ax_temp.plot(df["t_s"], df["T_out_filt"], label="Outlet (filtered)", color="#d08c00", linewidth=2.3)
  ax_temp.plot(df["t_s"], df["T_out_raw"], label="Outlet (raw)", color="#888888", linestyle="--", alpha=0.7)
  ax_temp.plot(df["t_s"], df["setF"], label="Setpoint", color="#1f78d1", linewidth=2)
  ax_temp.set_ylabel("Temperature (°F)")
  ax_temp.set_title("Control Logger Snapshot (100 ms cadence)", pad=8)
  ax_temp.grid(True, linestyle="--", linewidth=0.6, alpha=0.6)
  ax_temp.legend(loc="upper right")

  # Control effort (ratio/u) and flow
  ax_ctrl.plot(df["t_s"], df["ratio"], label="Mix Ratio", color="#00a36c", linewidth=2)
  ax_ctrl.plot(df["t_s"], df["u"], label="PI Output (u)", color="#9b4f96", linestyle="--", linewidth=2)
  ax_ctrl.set_xlabel("Time (s)")
  ax_ctrl.set_ylabel("Ratio / u")
  ax_ctrl.grid(True, linestyle="--", linewidth=0.6, alpha=0.6)

  ax_flow = ax_ctrl.twinx()
  ax_flow.plot(df["t_s"], df["flow_lpm"], label="Flow (L/min)", color="#e85d04", linewidth=2)
  ax_flow.set_ylabel("Flow (L/min)")

  # Highlight link drops
  link_drop = df[df["link_ok"] == 0]
  if not link_drop.empty:
    spans = _spans_from_indices(link_drop.index)
    for start_idx, end_idx in spans:
      start_t = df.loc[start_idx, "t_s"]
      end_t = df.loc[end_idx, "t_s"]
      ax_ctrl.axvspan(start_t, end_t, color="#ffb3b3", alpha=0.4, label="Link Lost" if start_idx == spans[0][0] else None)

  # Combine legends
  lines_ctrl, labels_ctrl = ax_ctrl.get_legend_handles_labels()
  lines_flow, labels_flow = ax_flow.get_legend_handles_labels()
  ax_ctrl.legend(lines_ctrl + lines_flow, labels_ctrl + labels_flow, loc="upper right")

  fig.tight_layout()
  try:
    plt.show()
  except KeyboardInterrupt:
    print("Closed by user (Ctrl+C)")


def _spans_from_indices(idxs):
  """Return inclusive index spans for contiguous ranges."""
  spans = []
  if len(idxs) == 0:
    return spans

  start = prev = idxs[0]
  for idx in idxs[1:]:
    if idx == prev + 1:
      prev = idx
      continue
    spans.append((start, prev))
    start = prev = idx
  spans.append((start, prev))
  return spans


if __name__ == "__main__":
  main()
