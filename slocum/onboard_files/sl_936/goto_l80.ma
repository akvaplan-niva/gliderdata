behavior_name=goto_list

# goto_l10.ma
# Flies the box in ashumet
# Each leg about 200m

<start:b_arg>
    b_arg: num_legs_to_run(nodim) -1 #! min = -2
				     # Number of waypoints to sequence thru:
				     #  1 to N exactly this many waypoints
				     #  0      illegal
				     # -1      loop forever
				     # -2      traverse list once (stop at last in list)
				     # <-2     illegal

    b_arg: initial_wpt(enum)      -1 #! min = -2; max = N-1
				     # Which waypoint to head for first
				     #  0 to N-1 the waypoint in the list
				     # -1 ==> one after last one achieved
				     # -2 ==> closest

    b_arg: start_when(enum)     0    # BAW_IMMEDIATELY

    b_arg: list_stop_when(enum) 7    # BAW_WHEN_WPT_DIST

    b_arg: list_when_wpt_dist(m) 10  #! min = 10.0
				     # used if list_stop_when == 7
				     # How close to get to the waypoint before it is 'achieved'

<end:b_arg>
<start:waypoints>
#  LON         LAT
-7032.0640   4138.1060
-7031.9200   4138.1090
-7031.9170   4138.0000
-7032.0610   4137.9980
<end:waypoints>

