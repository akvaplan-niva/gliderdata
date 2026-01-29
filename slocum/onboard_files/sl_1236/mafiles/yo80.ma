behavior_name=yo


<start:b_arg>
    b_arg: start_when(enum)      2   # pitch idle (see doco below)
    b_arg: num_half_cycles_to_do(nodim) 2   # Number of dive/climbs to perform
                                     # <0 is infinite, i.e. never finishes

    # arguments for dive_to
    b_arg: d_target_depth(m)    100
    b_arg: d_target_altitude(m)   5

    b_arg: d_use_bpump(enum) 0     # 0  Autoballast/Speed control.  
    b_arg: d_bpump_value(X) 360.0 # use_bpump == 0   Total amt of ballast, stored as C_AUTOBALLAST_VOLUME

    b_arg: d_use_pitch(enum)      3   # 1:battpos  2:setonce  3:servo
                                      #   in        rad        rad, <0 dive
    b_arg: d_pitch_value(X)   -0.37     # -20 deg
    b_arg: d_stop_when_hover_for(sec) 600.0        # increased to accomodate slower depth rate for autoballast 
    b_arg: d_stop_when_stalled_for(sec) 660.0      # increased to accomodate slower depth rate for autoballast 
    b_arg: d_speed_min(m/s) 0.06      # minimum depth rate for dive


    # arguments for climb_to
    b_arg: c_target_depth(m)      5
    b_arg: c_target_altitude(m)  -1

    b_arg: c_use_bpump(enum) 0	  # 0  Autoballast/Speed control. 

    b_arg: c_use_pitch(enum)      3   # 1:battpos  2:setonce  3:servo
                                      #   in         rad        rad, >0 climb
    b_arg: c_pitch_value(X)     0.37     # 20 deg
    b_arg: c_stop_when_hover_for(sec) 600.0        # increased to accomodate slower depth rate for autoballast 
    b_arg: c_stop_when_stalled_for(sec) 660.0        # increased to accomodate slower depth rate for autoballast 
    b_arg: c_speed_min(m/s) -0.06      # minimum depth rate for climb

    b_arg: end_action(enum) 2     # 0-quit, 2 resume
<end:b_arg>


