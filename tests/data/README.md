# tests/data
Raw test captures (CSV). Standard control logger header (10 Hz):

```
ms,setF,T_out_raw,T_out_filt,ratio,u,Kp,Ki,flow_lpm,link_ok
```

Keep files named by milestone/task (ex: `m2_closedloop_*.csv`, `setpoint_auto_*.csv`). Live serial captures from `m2_logger_live_plot.py` can be saved directly here for later plotting. If you add extra columns (e.g., additional sensors), document them in the test report.
