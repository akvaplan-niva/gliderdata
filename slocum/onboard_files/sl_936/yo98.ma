behavior_name=yo
# Written by Havard on UTC: 2024-02-19-12:27 for planA
# yo98.ma

<start:b_arg>
	b_arg: start_when(enum)       2   # pitch idle (see doco below)
	b_arg: num_half_cycles_to_do(nodim) 2   # Number of dive/climbs to perform
	#b_arg: alt_time(s)				0# time spacing for altimeter pings
	# arguments for dive_to
	b_arg: d_target_depth(m)     900.0
	b_arg: d_use_pitch(enum)      3   # 1:battpos  2:setonce  3:servo
									#   in         rad        rad, <0 dive
	b_arg: d_pitch_value(X)   -0.4538 # -26.0 deg
	b_arg: d_use_bpump(enum)      0 # Autoballast/Speed control
	b_arg: d_bpump_value(X)   400 # Speed control total ballast
	b_arg: d_speed_min(m/s)   0.2 # Minimum vertical depth rate for dive
	b_arg: d_target_altitude(m)   -1 # <0 disables
    b_arg: d_use_thruster(enum)   0  # 4  Command input power, watts
    b_arg: d_thruster_value(X)  7 # desired input power, between [1, 9]

	# arguments for climb_to
	b_arg: c_target_depth(m)     8.0
	b_arg: c_target_altitude(m)  -1
	b_arg: c_use_pitch(enum)      3   # 1:battpos  2:setonce  3:servo
									#   in         rad        rad, >0 climb
	b_arg: c_pitch_value(X)     0.4538 # 26.0 deg
	b_arg: c_use_bpump(enum)      0 # Autoballast/Speed control
	b_arg: c_bpump_value(X)    400 # Speed control total ballast
	b_arg: c_speed_min(m/s)   -0.2 # Minimum vertical depth rate for climb
    b_arg: c_use_thruster(enum)   0  #  3=Command depth rate. 2=command percentage in (x)
    b_arg: c_thruster_value(X)   7 # use_thruster == 3  m/s, desired depth rate. <0 for climb

	b_arg: end_action(enum) 2     # 0-quit, 2 resume
<end:b_arg>
