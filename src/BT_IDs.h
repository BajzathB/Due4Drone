#pragma once

//bool
#define ID_meas_2_card_sysTime 1
#define ID_meas_2_card_gyro_raw_X 2
#define ID_meas_2_card_gyro_raw_Y 3
#define ID_meas_2_card_gyro_raw_Z 4
#define ID_meas_2_card_gyro_PT1_X 5
#define ID_meas_2_card_gyro_PT1_Y 6
#define ID_meas_2_card_gyro_PT1_Z 7
#define ID_meas_2_card_gyro_PT2_X 8
#define ID_meas_2_card_gyro_PT2_Y 9
#define ID_meas_2_card_gyro_PT2_Z 10
#define ID_meas_2_card_acc_raw_X 11
#define ID_meas_2_card_acc_raw_Y 12
#define ID_meas_2_card_acc_raw_Z 13
#define ID_meas_2_card_acc_PT1_X 14
#define ID_meas_2_card_acc_PT1_Y 15
#define ID_meas_2_card_acc_PT1_Z 16
#define ID_meas_2_card_acc_PT2_X 17
#define ID_meas_2_card_acc_PT2_Y 18
#define ID_meas_2_card_acc_PT2_Z 19
#define ID_meas_2_card_angle_raw_roll 20
#define ID_meas_2_card_angle_raw_pitch 21
#define ID_meas_2_card_angle_PT1_roll 22
#define ID_meas_2_card_angle_PT1_pitch 23
#define ID_meas_2_card_angle_PT2_roll 24
#define ID_meas_2_card_angle_PT2_pitch 25
#define ID_meas_2_card_angle_KF_raw_roll 26
#define ID_meas_2_card_angle_KF_raw_pitch 27
#define ID_meas_2_card_angle_KF_PT10_roll 28
#define ID_meas_2_card_angle_KF_PT10_pitch 29
#define ID_meas_2_card_angle_KF_PT20_roll 30
#define ID_meas_2_card_angle_KF_PT20_pitch 31
#define ID_meas_2_card_angle_KF_PT11_roll 32
#define ID_meas_2_card_angle_KF_PT11_pitch 33
#define ID_meas_2_card_angle_KF_PT21_roll 34
#define ID_meas_2_card_angle_KF_PT21_pitch 35
#define ID_meas_2_card_angle_KF_PT22_roll 36
#define ID_meas_2_card_angle_KF_PT22_pitch 37
#define ID_meas_2_card_PID_refsig_X 38
#define ID_meas_2_card_PID_refsig_Y 39
#define ID_meas_2_card_PID_refsig_Z 40
#define ID_meas_2_card_PID_sensor_X 41
#define ID_meas_2_card_PID_sensor_Y 42
#define ID_meas_2_card_PID_sensor_Z 43
#define ID_meas_2_card_PID_Pout_X 44
#define ID_meas_2_card_PID_Pout_Y 45
#define ID_meas_2_card_PID_Pout_Z 46
#define ID_meas_2_card_PID_Iout_X 47
#define ID_meas_2_card_PID_Iout_Y 48
#define ID_meas_2_card_PID_Iout_Z 49
#define ID_meas_2_card_PID_Dout_X 50
#define ID_meas_2_card_PID_Dout_Y 51
#define ID_meas_2_card_PID_Dout_Z 52
#define ID_meas_2_card_PID_FFout_X 53
#define ID_meas_2_card_PID_FFout_Y 54
#define ID_meas_2_card_PID_FFout_Z 55
#define ID_meas_2_card_PID_U_X 56
#define ID_meas_2_card_PID_U_Y 57
#define ID_meas_2_card_PID_U_Z 58




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

