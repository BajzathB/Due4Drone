function angle = complementaryV1(angle, gyro, acc_angle, dt, alpha)
    angle = alpha * (angle + gyro * dt) + (1 - alpha) * acc_angle;
end