function step_response = calcStepResponse(setpoint_signal,gyro_signal)
%CALCSTEPRESPONSE Calculates PID step response based on setpoint and signal
%   code is based on pyPIDToolbox code from github
segment_length = 1024;
skipRate = 0.10;

step_responses = zeros(500, segment_length);
input_segments = zeros(500, segment_length);


seg_vector = [1:round(segment_length * skipRate):(length(gyro_signal) - segment_length)];

j = 1;

for i = seg_vector
    Xseg = setpoint_signal(i : i + segment_length - 1);

    if max(abs(Xseg)) < 20
        continue;
    end

    Yseg = gyro_signal(i : i + segment_length - 1);

    win = hamming(segment_length)';

    x_fft = fft(win .* Xseg);
    y_fft = fft(win .* Yseg);

    % Keep only positive frequencies (equivalent to rfft)
    x_fft = x_fft(1:floor(segment_length/2)+1);
    y_fft = y_fft(1:floor(segment_length/2)+1);

    H = x_fft / length(x_fft);
    G = y_fft / length(y_fft);

    Hcon = conj(H);

    % Build full spectrum for inverse FFT (irfft equivalent)
    num = (G .* Hcon) ./ (H .* Hcon + 1e-4);
    full_spec = [num, conj(num(end-1:-1:2))];

    imp = real(ifft(full_spec));

    % Smooth (moving average of length 21)
    imp_smooth = movmean(imp, 21);

    resp = cumsum(imp_smooth);
    resp = resp - resp(1);

    if var(resp) > 0.2
        continue;
    end

    step_responses(j, :) = resp;
    input_segments(j, :) = Xseg;

    j = j + 1;

    if j > 500
        break;
    end
end

valid_count = j - 1;
disp("Meas length: " + length(gyro_signal) + ", valid steps: " + valid_count);

step_response = mean(step_responses(1:valid_count, :), 1);
end

