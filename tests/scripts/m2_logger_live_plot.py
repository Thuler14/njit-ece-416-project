# ====================================================
# M2: Live Logger Plot
# Purpose: Read the control ESP32 CSV logger from serial in real time
#          and display live plots (temperatures, ratio/u, flow).
# Usage:
#   python3 tests/scripts/m2_logger_live_plot.py --port COM3
#   python3 tests/scripts/m2_logger_live_plot.py --port /dev/tty.usbserial-0001
#   python3 tests/scripts/m2_logger_live_plot.py --port /dev/tty.usbserial-0001 --window 120
# Optional: --outfile to save the streamed data to disk while plotting.
# ====================================================

from __future__ import annotations

import argparse
import sys
import time
from collections import deque
from pathlib import Path

import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

try:
  import serial  # type: ignore
except ImportError:  # pragma: no cover - hardware dependent
  serial = None


DEFAULT_HEADER = "ms,setF,T_out_raw,T_out_filt,ratio,u,Kp,Ki,flow_lpm,link_ok"


def parse_args() -> argparse.Namespace:
  base_dir = Path(__file__).resolve().parents[2]
  default_out = base_dir / "tests" / "data" / "m2_logger_live_capture.csv"

  p = argparse.ArgumentParser(description="Live-plot ESP32 control logger over serial.")
  p.add_argument("--port", required=True, help="Serial port (e.g. /dev/ttyUSB0)")
  p.add_argument("--baud", type=int, default=115200, help="Baud rate (default: 115200)")
  p.add_argument("--window", type=int, default=120, help="Time window to show (seconds)")
  p.add_argument("--timeout", type=float, default=0.2, help="Serial timeout (seconds)")
  p.add_argument("--outfile", type=Path, help="Optional CSV log file to save streamed data")
  return p.parse_args()


def open_serial(port: str, baud: int, timeout: float):
  if serial is None:
    print("pyserial not installed. Install with `pip install pyserial`.", file=sys.stderr)
    sys.exit(1)
  try:
    return serial.Serial(port, baudrate=baud, timeout=timeout)
  except Exception as exc:  # pragma: no cover
    print(f"Failed to open serial port {port}: {exc}", file=sys.stderr)
    sys.exit(2)


def is_data_line(line: str) -> bool:
  if not line:
    return False
  if line.startswith(DEFAULT_HEADER):
    return True
  first = line.split(",", 1)[0]
  return first.isdigit()


def main() -> None:
  args = parse_args()
  ser = open_serial(args.port, args.baud, args.timeout)

  if args.outfile:
    args.outfile.parent.mkdir(parents=True, exist_ok=True)
    args.outfile.write_text(DEFAULT_HEADER + "\n", encoding="utf-8")
    log_f = args.outfile.open("a", encoding="utf-8")
  else:
    log_f = None

  # Buffers
  window_ms = args.window * 1000
  times = deque()  # ms
  setpoints = deque()
  t_raw = deque()
  t_filt = deque()
  ratios = deque()
  u_vals = deque()
  flows = deque()
  link_ok = deque()

  fig, (ax_temp, ax_ctrl) = plt.subplots(2, 1, figsize=(10, 6), sharex=True)
  (line_filt,) = ax_temp.plot([], [], label="Outlet (filtered)", color="#d08c00", linewidth=2.3)
  (line_raw,) = ax_temp.plot([], [], label="Outlet (raw)", color="#888888", linestyle="--", alpha=0.7)
  (line_set,) = ax_temp.plot([], [], label="Setpoint", color="#1f78d1", linewidth=2)
  ax_temp.set_ylabel("Temperature (°F)")
  ax_temp.set_title("Live Control Logger (100 ms cadence)", pad=8)
  ax_temp.grid(True, linestyle="--", linewidth=0.6, alpha=0.6)
  ax_temp.legend(loc="upper left")

  (line_ratio,) = ax_ctrl.plot([], [], label="Mix Ratio", color="#00a36c", linewidth=2)
  (line_u,) = ax_ctrl.plot([], [], label="PI Output (u)", color="#9b4f96", linestyle="--", linewidth=2)
  ax_flow = ax_ctrl.twinx()
  (line_flow,) = ax_flow.plot([], [], label="Flow (L/min)", color="#e85d04", linewidth=2)
  ax_ctrl.set_xlabel("Time (s)")
  ax_ctrl.set_ylabel("Ratio / u")
  ax_flow.set_ylabel("Flow (L/min)")
  ax_ctrl.grid(True, linestyle="--", linewidth=0.6, alpha=0.6)


  def trim_window():
    if not times:
      return
    cutoff = times[-1] - window_ms
    while times and times[0] < cutoff:
      times.popleft()
      setpoints.popleft()
      t_raw.popleft()
      t_filt.popleft()
      ratios.popleft()
      u_vals.popleft()
      flows.popleft()
      link_ok.popleft()

  def update_plot(_frame):
    # Drain serial buffer
    while ser.in_waiting:
      raw = ser.readline()
      if not raw:
        break
      try:
        line = raw.decode("utf-8", errors="ignore").strip()
      except Exception:
        continue
      if not is_data_line(line):
        continue
      if line.startswith(DEFAULT_HEADER):
        continue

      parts = line.split(",")
      if len(parts) < 10:
        continue
      try:
        t_ms = float(parts[0])
        setF = float(parts[1])
        raw_f = float(parts[2])
        filt_f = float(parts[3])
        ratio = float(parts[4])
        u = float(parts[5])
        flow = float(parts[8])
        link = int(parts[9])
      except ValueError:
        continue

      times.append(t_ms)
      setpoints.append(setF)
      t_raw.append(raw_f)
      t_filt.append(filt_f)
      ratios.append(ratio)
      u_vals.append(u)
      flows.append(flow)
      link_ok.append(link)

      if log_f:
        log_f.write(line + "\n")
        log_f.flush()

    trim_window()
    if not times:
      return line_filt, line_raw, line_set, line_ratio, line_u, line_flow

    t_s = [ms / 1000.0 for ms in times]
    line_filt.set_data(t_s, t_filt)
    line_raw.set_data(t_s, t_raw)
    line_set.set_data(t_s, setpoints)
    line_ratio.set_data(t_s, ratios)
    line_u.set_data(t_s, u_vals)
    line_flow.set_data(t_s, flows)

    # Update axes limits
    # Keep a sliding window on the x-axis to avoid tick label disappearance
    t_max = t_s[-1]
    t_min = max(0.0, t_max - args.window)
    ax_temp.set_xlim(t_min, t_max)

    ax_ctrl.set_xlabel("Time (s)")

    ax_temp.relim()
    ax_temp.autoscale_view(scaley=True)
    ax_ctrl.relim()
    ax_ctrl.autoscale_view(scaley=True)
    ax_flow.relim()
    ax_flow.autoscale_view(scaley=True)

    # Shade link-loss regions
    [p.remove() for p in getattr(ax_ctrl, "_link_patches", [])] if hasattr(ax_ctrl, "_link_patches") else None
    patches = []
    first_span = True
    if link_ok and any(v == 0 for v in link_ok):
      start = None
      t0 = 0.0
      for idx, val in enumerate(link_ok):
        if val == 0 and start is None:
          start = idx
        if val == 1 and start is not None:
          patches.append(
              ax_ctrl.axvspan(
                  times[start] / 1000.0,
                  times[idx - 1] / 1000.0,
                  color="#ffb3b3",
                  alpha=0.4,
                  label="Link Lost" if first_span else None,
              )
          )
          first_span = False
          start = None
      if start is not None:
        patches.append(
            ax_ctrl.axvspan(
                times[start] / 1000.0,
                times[-1] / 1000.0,
                color="#ffb3b3",
                alpha=0.4,
                label="Link Lost" if first_span else None,
            )
        )
    ax_ctrl._link_patches = patches

    # Combine legends like static plot
    lines_ctrl, labels_ctrl = ax_ctrl.get_legend_handles_labels()
    lines_flow, labels_flow = ax_flow.get_legend_handles_labels()
    ax_ctrl.legend(lines_ctrl + lines_flow, labels_ctrl + labels_flow, loc="upper left")

    return line_filt, line_raw, line_set, line_ratio, line_u, line_flow

  print(f"[m2_logger_live_plot] Listening on {args.port} @ {args.baud} (window={args.window}s)")
  if args.outfile:
    print(f"Logging to {args.outfile}")

  ani = FuncAnimation(fig, update_plot, interval=200, blit=False, cache_frame_data=False)
  try:
    plt.show()
  except KeyboardInterrupt:
    print("\nInterrupted by user, closing...")
  finally:
    try:
      ser.close()
    except Exception:
      pass
    if log_f:
      try:
        log_f.close()
      except Exception:
        pass
    print("Closed serial connection.")


if __name__ == "__main__":
  main()
