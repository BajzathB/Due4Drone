function step_responses = calcManualStepResponse(Xseg,Yseg)
%CALCMANUALSTEPRESPONSE Calculates PID step response based on setpoint and
%signal based on manually selected window
%   code is based on pyPIDToolbox code from github

j = 1;

segment_length = length(Xseg);

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

step_responses(j, :) = resp;
input_segments(j, :) = Xseg;

end

