function [angle, gyro_bias] = complementaryV2(angle, gyro, acc_angle, dt, gyro_bias, Kp, Ki)
    error = acc_angle - angle;

    gyro_bias = gyro_bias + Ki * error * dt;
    gyro_corrected = gyro + Kp * error - gyro_bias;

    angle = angle + gyro_corrected * dt;
end