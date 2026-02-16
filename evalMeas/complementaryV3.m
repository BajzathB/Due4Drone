function angle = complementaryV3(angle, gyro, acc_angle, dt, ax, ay, az, alpha)
    acc_mag = sqrt(ax*ax + ay*ay + az*az);
    w = max(0, min(1, 1 - abs(acc_mag - 1)));
    angle = alpha * (angle + gyro * dt) + (1.0 - alpha) * w * acc_angle + (1.0 - w) * angle;
end