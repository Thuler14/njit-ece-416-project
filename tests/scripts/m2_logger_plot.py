# ====================================================
# Logger CSV Plotter
# Purpose: Batch-plot control/flow telemetry CSVs from tests/data.
# Outputs: PNGs saved to tests/reports with the same stem as the CSV.
# ====================================================

from __future__ import annotations

import argparse
from pathlib import Path

import matplotlib
import pandas as pd


BASE_DIR = Path(__file__).resolve().parents[2]
DATA_DIR = BASE_DIR / "tests" / "data"
REPORT_DIR = BASE_DIR / "tests" / "reports"
DEFAULT_PATTERNS = ("setpoint_auto_*.csv", "setpoint_manual_*.csv", "setpoint_change*.csv")


def parse_args() -> argparse.Namespace:
  p = argparse.ArgumentParser(description="Plot control logger CSVs and save PNGs.")
  p.add_argument(
      "--pattern",
      action="append",
      help="Glob pattern(s) under tests/data (default: setpoint_auto_*, setpoint_manual_*, setpoint_change*)",
  )
  p.add_argument("--show", action="store_true", help="Show plots interactively after saving")
  return p.parse_args()


def main() -> None:
  args = parse_args()
  patterns = args.pattern if args.pattern else list(DEFAULT_PATTERNS)
  csv_files = sorted({p for pat in patterns for p in DATA_DIR.glob(pat)})
  if not csv_files:
    joined = ", ".join(patterns)
    print(f"No CSV files found in {DATA_DIR} matching: {joined}")
    return

  if not args.show:
    matplotlib.use("Agg")
  import matplotlib.pyplot as plt  # type: ignore

  REPORT_DIR.mkdir(parents=True, exist_ok=True)
  figs = []

  for csv_path in csv_files:
    fig = _plot_csv(csv_path, plt)

    out_path = REPORT_DIR / f"{csv_path.stem}.png"
    fig.savefig(out_path, dpi=150, bbox_inches="tight")

    rel_in = csv_path.relative_to(BASE_DIR)
    rel_out = out_path.relative_to(BASE_DIR)
    print(f"Saved {rel_in} -> {rel_out}")

    if args.show:
      figs.append(fig)
    else:
      plt.close(fig)

  if args.show:
    plt.show()


def _plot_csv(csv_path: Path, plt):
  df = pd.read_csv(csv_path)
  df["t_s"] = df["ms"] / 1000.0

  fig, (ax_temp, ax_ctrl) = plt.subplots(2, 1, figsize=(10, 6), sharex=True)

  # Outlet temperature vs setpoint
  ax_temp.plot(df["t_s"], df["T_out_filt"], label="Outlet (filtered)", color="#d08c00", linewidth=2.3)
  ax_temp.plot(df["t_s"], df["T_out_raw"], label="Outlet (raw)", color="#888888", linestyle="--", alpha=0.7)
  ax_temp.plot(df["t_s"], df["setF"], label="Setpoint", color="#1f78d1", linewidth=2)
  ax_temp.set_ylabel("Temperature (°F)")
  ax_temp.set_title(f"{csv_path.stem} (100 ms cadence)", pad=8)
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

  # Highlight link drops when the column is present
  if "link_ok" in df.columns:
    link_drop = df[df["link_ok"] == 0]
    if not link_drop.empty:
      spans = _spans_from_indices(link_drop.index)
      for start_idx, end_idx in spans:
        start_t = df.loc[start_idx, "t_s"]
        end_t = df.loc[end_idx, "t_s"]
        ax_ctrl.axvspan(
            start_t,
            end_t,
            color="#ffb3b3",
            alpha=0.4,
            label="Link Lost" if start_idx == spans[0][0] else None,
        )

  # Combine legends
  lines_ctrl, labels_ctrl = ax_ctrl.get_legend_handles_labels()
  lines_flow, labels_flow = ax_flow.get_legend_handles_labels()
  ax_ctrl.legend(lines_ctrl + lines_flow, labels_ctrl + labels_flow, loc="upper right")

  fig.tight_layout()
  return fig


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
