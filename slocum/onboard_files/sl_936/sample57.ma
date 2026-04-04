behavior_name=sample
# Written by SFMC on UTC: 2022-03-01T10:28:02.028
# sample57.ma

<start:b_arg>
	b_arg: sensor_type(enum) 0 # if 0 ALL
	b_arg: state_to_sample(enum) 1 # if 7 Diving, climbing, hovering
	b_arg: intersample_time(s) 0 # if 0 As fast as possible
	b_arg: intersample_depth(m) -1 # if -1 Use intersample_time
	b_arg: nth_yo_to_sample(nodim) 1
	b_arg: min_depth(m) 0
	b_arg: max_depth(m) 1000
<end:b_arg>

