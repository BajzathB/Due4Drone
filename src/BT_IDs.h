#pragma once

//special commands
#define ID_update_global_time 1


//bool
#define ID_meas_2_card_sysTime 100
#define ID_meas_2_card_gyro_raw_X 101
#define ID_meas_2_card_gyro_raw_Y 102
#define ID_meas_2_card_gyro_raw_Z 103
#define ID_meas_2_card_gyro_PT1_X 104
#define ID_meas_2_card_gyro_PT1_Y 105
#define ID_meas_2_card_gyro_PT1_Z 106
#define ID_meas_2_card_gyro_PT2_X 107
#define ID_meas_2_card_gyro_PT2_Y 108
#define ID_meas_2_card_gyro_PT2_Z 109
#define ID_meas_2_card_acc_raw_X 110
#define ID_meas_2_card_acc_raw_Y 111
#define ID_meas_2_card_acc_raw_Z 112
#define ID_meas_2_card_acc_PT1_X 113
#define ID_meas_2_card_acc_PT1_Y 114
#define ID_meas_2_card_acc_PT1_Z 115
#define ID_meas_2_card_acc_PT2_X 116
#define ID_meas_2_card_acc_PT2_Y 117
#define ID_meas_2_card_acc_PT2_Z 118
#define ID_meas_2_card_angle_raw_roll 119
#define ID_meas_2_card_angle_raw_pitch 120
#define ID_meas_2_card_angle_PT1_roll 121
#define ID_meas_2_card_angle_PT1_pitch 122
#define ID_meas_2_card_angle_PT2_roll 123
#define ID_meas_2_card_angle_PT2_pitch 124
#define ID_meas_2_card_angle_KF_raw_roll 125
#define ID_meas_2_card_angle_KF_raw_pitch 126
#define ID_meas_2_card_angle_KF_PT10_roll 127
#define ID_meas_2_card_angle_KF_PT10_pitch 128
#define ID_meas_2_card_angle_KF_PT20_roll 129
#define ID_meas_2_card_angle_KF_PT20_pitch 130
#define ID_meas_2_card_angle_KF_PT11_roll 131
#define ID_meas_2_card_angle_KF_PT11_pitch 132
#define ID_meas_2_card_angle_KF_PT21_roll 133
#define ID_meas_2_card_angle_KF_PT21_pitch 134
#define ID_meas_2_card_angle_KF_PT22_roll 135
#define ID_meas_2_card_angle_KF_PT22_pitch 136
#define ID_meas_2_card_PID_refsig_X 137
#define ID_meas_2_card_PID_refsig_Y 138
#define ID_meas_2_card_PID_refsig_Z 139
#define ID_meas_2_card_PID_sensor_X 140
#define ID_meas_2_card_PID_sensor_Y 141
#define ID_meas_2_card_PID_sensor_Z 142
#define ID_meas_2_card_PID_Pout_X 143
#define ID_meas_2_card_PID_Pout_Y 144
#define ID_meas_2_card_PID_Pout_Z 145
#define ID_meas_2_card_PID_Iout_X 146
#define ID_meas_2_card_PID_Iout_Y 147
#define ID_meas_2_card_PID_Iout_Z 148
#define ID_meas_2_card_PID_Dout_X 149
#define ID_meas_2_card_PID_Dout_Y 150
#define ID_meas_2_card_PID_Dout_Z 151
#define ID_meas_2_card_PID_FFout_X 152
#define ID_meas_2_card_PID_FFout_Y 153
#define ID_meas_2_card_PID_FFout_Z 154
#define ID_meas_2_card_PID_U_X 155
#define ID_meas_2_card_PID_U_Y 156
#define ID_meas_2_card_PID_U_Z 157




//uint8
#define ID_SDCARD_MAINSTATE 1000
#define ID_SDCARD_RESET_MEASRUEMENT 1001
#define ID_SDCARD_REINIT_SDCARD 1002



//uint16




//float
#define ID_control_PID_rate_P_X 2000
#define ID_control_PID_rate_I_X 2001
#define ID_control_PID_rate_D_X 2002
#define ID_control_PID_rate_P_Y 2003
#define ID_control_PID_rate_I_Y 2004
#define ID_control_PID_rate_D_Y 2005
#define ID_control_PID_rate_P_Z 2006
#define ID_control_PID_rate_I_Z 2007
#define ID_control_PID_rate_D_Z 2008
#define ID_control_PID_rate_saturation_I 2009
#define ID_control_PID_rate_saturation_PID 2010
#define ID_control_PID_rate_Dterm_C 2011
#define ID_gyro_filter_paramC 2012
#define ID_gyro_kalman_filter_q 2013
#define ID_gyro_kalman_filter_r 2014
#define ID_acc_filter_paramC 2015
#define ID_acc_kalman_filter_q_angle 2016
#define ID_acc_kalman_filter_q_bias 2017
#define ID_acc_kalman_filter_r 2018
#define ID_spi_acc_offset_x 2019
#define ID_spi_acc_offset_y 2020
#define ID_spi_acc_offset_z 2021
#define ID_control_PID_cascade_P_X 2022
#define ID_control_PID_cascade_I_X 2023
#define ID_control_PID_cascade_D_X 2024
#define ID_control_PID_cascade_P_Y 2025
#define ID_control_PID_cascade_I_Y 2026
#define ID_control_PID_cascade_D_Y 2027
#define ID_control_PID_cascade_P_Z 2028
#define ID_control_PID_cascade_I_Z 2029
#define ID_control_PID_cascade_D_Z 2030
#define ID_control_PID_cascade_saturation_I 2031
#define ID_control_PID_cascade_saturation_PID 2032
#define ID_control_PID_cascade_Dterm_C 2033
#define ID_control_PID_rate_FF_X 2034
#define ID_control_PID_rate_FF_Y 2035
#define ID_control_PID_rate_FF_DX 2036
#define ID_control_PID_rate_FF_DY 2037
#define ID_control_PID_cascade_FF_X 2038
#define ID_control_PID_cascade_FF_Y 2039
#define ID_control_PID_cascade_FF_DX 2040
#define ID_control_PID_cascade_FF_DY 2041



//steaming flags 32 flags only!
#define ID_bitshift_substracter 3000
#define ID_pidRate_sensor_signal_X 3000
#define ID_pidRate_sensor_signal_Y 3001
#define ID_pidRate_sensor_signal_Z 3002
#define ID_gyro_signal_X 3003
#define ID_gyro_signal_Y 3004
#define ID_gyro_signal_Z 3005
#define ID_motorCommand_FL 3006
#define ID_motorCommand_FR 3007
#define ID_motorCommand_RL 3008
#define ID_motorCommand_RR 3009
#define ID_gyro_PT1_X 3010
#define ID_gyro_PT1_Y 3011
#define ID_gyro_PT1_Z 3012
#define ID_gyro_PT2_X 3013
#define ID_gyro_PT2_Y 3014
#define ID_gyro_PT2_Z 3015
#define ID_gyro_KALMAN_X 3016
#define ID_gyro_KALMAN_Y 3017
#define ID_gyro_KALMAN_Z 3018
#define ID_acc_signal_X 3019
#define ID_acc_signal_Y 3020
#define ID_acc_signal_Z 3021
#define ID_acc_PT1_X 3022
#define ID_acc_PT1_Y 3023
#define ID_acc_PT1_Z 3024
#define ID_acc_PT2_X 3025
#define ID_acc_PT2_Y 3026
#define ID_acc_PT2_Z 3027
#define ID_roll 3028
#define ID_pitch 3029
#define ID_roll_PT1 3030
#define ID_pitch_PT1 3031

#define ID_bitshift_substracter2 3032
#define ID_roll_PT2 3032
#define ID_pitch_PT2 3033
#define ID_roll_KF 3034
#define ID_pitch_KF 3035
#define ID_roll_KF_AccPT1 3036
#define ID_pitch_KF_AccPT1 3037
#define ID_roll_KF_AccPT2 3038
#define ID_pitch_KF_AccPT2 3039
#define ID_roll_KF_AccPT1_GyroPT1 3040
#define ID_pitch_KF_AccPT1_GyroPT1 3041
#define ID_roll_KF_AccPT2_GyroPT2 3042
#define ID_pitch_KF_AccPT2_GyroPT2 3043
#define ID_loop_time_ms 3044

