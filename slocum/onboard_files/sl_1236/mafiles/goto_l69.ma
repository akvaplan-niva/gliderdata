behavior_name=goto_list
# Written by SFMC on UTC: 2025-06-10T20:31:58.396684
# goto_l69.ma

<start:b_arg>
	b_arg: num_legs_to_run(nodim) -1
	b_arg: start_when(enum) 0 # BAW_IMMEDIATELY
	b_arg: list_stop_when(enum) 7 # BAW_WHEN_WPT_DIST
	b_arg: list_when_wpt_dist(m) 1000  #! min = 10.0
	b_arg: initial_wpt(enum) 0
	b_arg: num_waypoints(nodim) 2
<end:b_arg>
<start:waypoints>
1745.63	6947.11
1503.41	7021.92
<end:waypoints>