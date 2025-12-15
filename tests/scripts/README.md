# tests/scripts
Python helpers to plot control logs and milestone test data.

## Setup
- `python3 -m pip install pandas matplotlib pyserial` (pyserial only needed for live logging).
- Run scripts from the repo root so relative paths to `tests/data/` and `tests/reports/` resolve.

## Scripts
- `m2_logger_live_plot.py` — stream the control unit CSV logger over serial, live-plot, and optionally save (`--port /dev/tty... --outfile tests/data/run.csv`).
- `m2_logger_plot.py` — batch-plot CSVs in `tests/data/` and save PNGs to `tests/reports/` (patterns via `--pattern`).
- `m2_flow_sensor_test_plot.py` — dual-axis plot for YF-S201 calibration captures.
- `m2_outlet_temp_test_plot.py` — raw vs filtered outlet temperature from the 10 Hz read loop.
- `m2_closed_loop_v1_plot.py` — PID step response plot (outlet vs setpoint).

Use `python3 tests/scripts/<script>.py --help` for arguments and expected input files.
