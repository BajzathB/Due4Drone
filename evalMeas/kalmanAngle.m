function [angle, P, angle_est, bias_est] = kalmanAngle(accAngle, gyroRate, dt, angle_est, bias_est, P, Q, R)
% Q = [Q_angle, Q_bias]
% R = measurement noise variance

    Q_angle = Q(1);
    Q_bias  = Q(2);
    R_measure = R;

    % ---------- Prediction ----------
    angle_est = angle_est + dt * (gyroRate - bias_est);

    P(1,1) = P(1,1) + dt*(dt*P(2,2) - P(1,2) - P(2,1) + Q_angle);
    P(1,2) = P(1,2) - dt*P(2,2);
    P(2,1) = P(2,1) - dt*P(2,2);
    P(2,2) = P(2,2) + Q_bias*dt;

    % ---------- Update ----------
    y = accAngle - angle_est;
    S = P(1,1) + R_measure;
    K = [P(1,1); P(2,1)] / S;

    angle_est = angle_est + K(1)*y;
    bias_est  = bias_est  + K(2)*y;

    P = P - K*[P(1,1) P(1,2)];

    angle = angle_est;
end
