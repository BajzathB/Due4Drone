clear all, clc

directory = "2026_01_03";
fileNumber = 3; %1 is ., 2 is ..

%reading file in
files = dir(directory);
file = directory + "/" +files(fileNumber).name;
disp(['Opening ' + file]);

%reading header
fid = fopen(file,'r');
line1 = fgetl(fid);
line2 = fgetl(fid);
line3 = fgetl(fid);
fclose(fid);
headerText = strsplit(line2, ',');   % line 2
formatSpec = repmat('%f', 1, numel(headerText));
headerData = textscan(line3, formatSpec, 1, ...
    'Delimiter', ',', ...
    'CollectOutput', true);
header = array2table(headerData{1}, 'VariableNames', headerText);

%reading measurement data
opts = detectImportOptions(file, ...
    'NumHeaderLines', 3, ...
    'Delimiter', ',');
meas = readtable(file, opts);

%% plot gyro
% figure(1);
% clf(1);
% plot(meas.sysTime, [meas.GRawX meas.GPT1X]);
% % plot(meas.sysTime, [meas.GRawY meas.GPT1Y meas.GPT2Y]);
% xlabel('time');
% ylabel('rad/s');
% legend('raw','pt1','pt2');
% title('Gyro')

%% plot acc
% trim = 150;
% figure(2);
% clf(2);
% subplot(3,1,1);
% plot(meas.sysTime(trim:(end-trim)), [meas.ARawX(trim:(end-trim)) meas.APT1X(trim:(end-trim))]);
% title('Acc')
% subplot(3,1,2);
% plot(meas.sysTime(trim:(end-trim)), [meas.ARawY(trim:(end-trim)) meas.APT1Y(trim:(end-trim))]);
% ylabel('m2/s');
% legend('raw','pt1','pt2');
% subplot(3,1,3);
% plot(meas.sysTime(trim:(end-trim)), [meas.ARawZ(trim:(end-trim)) meas.APT1Z(trim:(end-trim))]);
% xlabel('time');
% title('Acc')

%% plot angle
% trim = 150;
% figure(3);
% clf(3);
% subplot(2,1,1);
% plot(meas.sysTime(trim:(end-trim)), [meas.aRawR(trim:(end-trim)) meas.aPT2R(trim:(end-trim)) meas.aKFRawR(trim:(end-trim))]);
% title('Angle')
% subplot(2,1,2);
% plot(meas.sysTime(trim:(end-trim)), [meas.aRawP(trim:(end-trim)) meas.aPT2P(trim:(end-trim)) meas.aKFRawP(trim:(end-trim))]);
% ylabel('°angle');
% legend('raw', 'pt2', 'kfraw');
% xlabel('time');

%% plot plot angle with parameters
% figure(4);
% clf(4);
% plot(meas.sysTime, [ meas.aPT1R meas.aKFRawR]);
% legend( 'aPT1R', 'aKFRawR', 'aKFPT10R');
% usedParams = file + newline ...
%     + sprintf("GparamC: %.f", header.GparamC) ...
%     + sprintf(", AparamC: %.f\n", header.AparamC) ...
%     + sprintf("KFQangle: %.6f", header.KFQAng) ...
%     + sprintf(", KFQbias: %.6f", header.KFQbias) ...
%     + sprintf(", KFRmeas: %.f\n", header.KFRmeas) ...
%     + sprintf("CPx: %.f", header.CPx) ...
%     + sprintf(", CIx: %.f", header.CIx) ...
%     + sprintf(", CPy: %.f", header.CPy) ...
%     + sprintf(", CIy: %.f", header.CIy);
% title(usedParams, 'Interpreter', 'none');

%% FFT acc axis

% trim = 150;
% L = length(meas.sysTime(trim:(end-100)));
% Fs = 1/mean(diff(meas.sysTime(trim:(end-trim))));   %sampling frequency
% f = Fs*(0:(L/2))/L;
% 
% freq1 = calcFrequency(meas.ARawX(trim:(end-trim)), L);
% freq2 = calcFrequency(meas.APT1X(trim:(end-trim)), L);
% freq3 = calcFrequency(meas.ARawY(trim:(end-trim)), L);
% freq4 = calcFrequency(meas.APT1Y(trim:(end-trim)), L);
% freq5 = calcFrequency(meas.ARawZ(trim:(end-trim)), L);
% freq6 = calcFrequency(meas.APT1Z(trim:(end-trim)), L);
% 
% figure(5)
% clf(5);
% subplot(3,1,1);
% plot(f, [freq1 freq2])
% subplot(3,1,2);
% plot(f, [freq3 freq4])
% subplot(3,1,3);
% plot(f, [freq5 freq6])
% legend('raw','pt1','pt2');
% ylim([0 2]);
% title('FFT');
% xlabel('f (Hz)')

%% FFT angle

% trim = 150;
% L = length(meas.sysTime(trim:(end-100)));
% Fs = 1/mean(diff(meas.sysTime(trim:(end-trim))));   %sampling frequency
% f = Fs*(0:(L/2))/L;
% 
% freq1 = calcFrequency(meas.aRawR(trim:(end-trim)), L);
% freq2 = calcFrequency(meas.aPT2R(trim:(end-trim)), L);
% freq3 = calcFrequency(meas.aKFRawR(trim:(end-trim)), L);
% freq4 = calcFrequency(meas.aRawP(trim:(end-trim)), L);
% freq5 = calcFrequency(meas.aPT2P(trim:(end-trim)), L);
% freq6 = calcFrequency(meas.aKFRawP(trim:(end-trim)), L);
% 
% figure(5)
% clf(5);
% subplot(2,2,1);
% plot([ meas.aPT2R(trim:(end-trim)) meas.aKFRawR(trim:(end-trim))]);
% title('Measurements');
% subplot(2,2,2);
% plot(f, [ freq2 freq3])
% ylim([0 2]);
% title('FFT');
% subplot(2,2,3);
% plot([ meas.aPT2P(trim:(end-trim)) meas.aKFRawP(trim:(end-trim))]);
% subplot(2,2,4);
% plot(f, [ freq5 freq6])
% legend('pt2','kfraw');
% ylim([0 2]);
% xlabel('f (Hz)')

%% recalc KF

% Q = [0.0001  0.003];
% R = 100;
% P_aKFRawR = eye(2);
% P_aKFPT10R = eye(2);
% P_aKFPT11R = eye(2);
% aKFRawR(1) = 1;
% aKFPT10R(1) = 1;
% aKFPT11R(1) = 1;
% angle_est_aKFRawR = meas.aRawR(1);
% angle_est_aKFPT10R = meas.aRawR(1);
% angle_est_aKFPT11R = meas.aRawR(1);
% bias_est_aKFRawR = 0;
% bias_est_aKFPT10R = 0;
% bias_est_aKFPT11R = 0;
% 
% for i = 2:length(meas.sysTime)
%     dt =meas.sysTime(i) - meas.sysTime(i-1);
%     
% %     [aKFRawR(i), P_aKFRawR, angle_est_aKFRawR, bias_est_aKFRawR] = kalmanAngle(meas.aRawR(i), meas.GRawX(i), dt, angle_est_aKFRawR, bias_est_aKFRawR, P_aKFRawR, Q, R);
% %     [aKFPT10R(i), P_aKFPT10R, angle_est_aKFPT10R, bias_est_aKFPT10R] = kalmanAngle(meas.aPT1R(i), meas.GRawX(i), dt, angle_est_aKFPT10R, bias_est_aKFPT10R, P_aKFPT10R, Q, R);
%     [aKFPT11R(i), P_aKFPT11R, angle_est_aKFPT11R, bias_est_aKFPT11R] = kalmanAngle(-meas.aPT1R(i), meas.GPT1X(i), dt, angle_est_aKFPT11R, bias_est_aKFPT11R, P_aKFPT11R, Q, R);
%     
% end
% 
% figure(6);
% clf(6);
% % plot(meas.sysTime, [meas.aKFRawR aKFRawR']);
% % legend('OG\_aKFRawR', 'RE\_aKFRawR');
% plot(meas.sysTime, [meas.GPT1X meas.aPT1R meas.aKFPT11R aKFPT11R']);
% legend('OG\_GPT1X', 'OG\_aPT1R', 'OG\_aKFPT11R', 'RE\_aKFPT11R');
% title('Recalc');

%% complementary filter calc

% compV1_angle(1) = meas.aRawR(1);
% alpha = 0.995;
% 
% compV2_angle(1) = meas.aRawR(1);
% gyro_bias = 0;
% Kp = 0.01;
% Ki = 0.001;
% 
% compV3_angle(1) = meas.aRawR(1);
% 
% for i = 2:length(meas.sysTime)
%     dt =meas.sysTime(i) - meas.sysTime(i-1);
%     
%     compV1_angle(i) = complementaryV1(compV1_angle(i-1), meas.GRawX(i), meas.aRawR(i), dt, alpha);
%     [compV2_angle(i), gyro_bias] = complementaryV2(compV2_angle(i-1), meas.GRawX(i), meas.aRawR(i), dt, gyro_bias, Kp, Ki);
%     compV3_angle(i) = complementaryV3(compV1_angle(i-1), meas.GRawX(i), meas.aRawR(i), dt, meas.ARawX(i), meas.ARawY(i), meas.ARawZ(i), alpha);
%     
%     
% end
% 
% figure(7);
% clf(7);
% plot(meas.sysTime, [meas.aPT2R compV1_angle' compV2_angle' compV3_angle']);
% legend('OG\_aKFRawR', 'compV1R', 'compV2R', 'compV3R');
% title('Recalc');

%% complementary filter

% figure(8);
% clf(8);
% % plot(meas.sysTime, [meas.aKFRawR meas.aKFPT10R meas.aKFPT11R meas.aCFRawR meas.aCFPT10R meas.aCFPT11R]);
% % legend('aKFRawR', 'aKFPT10R', 'aKFPT11R', 'aCFRawR', 'aCFPT10R', 'aCFPT11R');
% plot(meas.sysTime, [meas.aKFRawR meas.aCFRawR meas.aCFPT10R]);
% legend('aKFRawR', 'aCFRawR', 'aCFPT10R');
% title('Complementary compare');

%% rate PID analysis

% refXRate(1) = 0;
% refXRatePT1(1) = 0;
% 
% paramC = 750/75;
% setPointThreshold = 300;
% errorThreshold = 100;
% 
% for i = 2:length(meas.sysTime)
%     dt =meas.sysTime(i) - meas.sysTime(i-1);
%     
%     refXRate(i) = (meas.PIDRefX(i) - meas.PIDRefX(i-1))/dt;
%     
%     refXRatePT1(i) = (refXRate(i) + paramC * (refXRatePT1(i-1))) / (paramC + 1);
%     
% end
% 
% figure(9);
% clf(9);
% subplot(2,1,1);
% hold on
% % plot(meas.sysTime, [meas.PIDRefX/10 meas.PIDSensX/10 -meas.PIDUX]);
% plot(meas.sysTime, [ refXRatePT1']);
% yline(setPointThreshold,'g-');
% yline(-setPointThreshold,'g-');
% ylim([-1500 1500]);
% legend('refXRatePT1');
% usedParams = "Rate PID analysis" + newline ...
%     + sprintf("Px: %.f", header.Px) ...
%     + sprintf(", Ix: %.f", header.Ix) ...
%     + sprintf(", Dx: %.f", header.Dx) ...
%     + sprintf(", FFrx: %.f\n", header.FFrx) ...
%     + sprintf("satI: %.f", header.satI) ...
%     + sprintf(", satPID: %.f", header.satPID) ...
%     + sprintf(", DTermC: %.f", header.DTermC) ...
%     + sprintf(", GparamC: %.f", header.GparamC);
% title(usedParams, 'Interpreter', 'none');
% hold off
% subplot(2,1,2);
% hold on
% plot(meas.sysTime, [meas.PIDPoutX/(header.Px/10000) ]);
% yline(errorThreshold,'r:');
% yline(-errorThreshold,'r:');
% ylim([-400 400]);
% legend('error');
% hold off

%% pyPIDtoolbox snippet
% segment_length = 1024;
% wnd = 1024;                 % 500 ms step response window
% StepRespDuration_ms = 500;  % max duration in ms for plotting
% rateHighThreshold = 500;    % deg/s
% skipRate = 0.10;
% 
% %take subset of meas
% % t_start = 320;
% % t_end = 355;
% % idx = (meas.sysTime >= t_start) & (meas.sysTime <= t_end);
% % gyro_signal = meas.PIDSensX(idx);
% % setpoint_signal = meas.PIDRefX(idx);
% gyro_signal = meas.PIDSensX;
% setpoint_signal = meas.PIDRefX;
% 
% step_response = zeros(500, segment_length);
% input_segments = zeros(500, segment_length);
% 
% disp("Meas length: " + length(gyro_signal));
% 
% seg_vector = [1:round(segment_length * skipRate):(length(gyro_signal) - segment_length)];
% 
% j = 1;
% 
% for i = seg_vector
%     Xseg = setpoint_signal(i : i + segment_length - 1);
% 
%     if max(abs(Xseg)) < 20
%         continue;
%     end
% 
%     Yseg = gyro_signal(i : i + segment_length - 1);
% 
%     win = hamming(segment_length)';
% 
%     x_fft = fft(win .* Xseg);
%     y_fft = fft(win .* Yseg);
% 
%     % Keep only positive frequencies (equivalent to rfft)
%     x_fft = x_fft(1:floor(segment_length/2)+1);
%     y_fft = y_fft(1:floor(segment_length/2)+1);
% 
%     H = x_fft / length(x_fft);
%     G = y_fft / length(y_fft);
% 
%     Hcon = conj(H);
% 
%     % Build full spectrum for inverse FFT (irfft equivalent)
%     num = (G .* Hcon) ./ (H .* Hcon + 1e-4);
%     full_spec = [num, conj(num(end-1:-1:2))];
% 
%     imp = real(ifft(full_spec));
% 
%     % Smooth (moving average of length 21)
%     imp_smooth = movmean(imp, 21);
% 
%     resp = cumsum(imp_smooth);
%     resp = resp - resp(1);
% 
%     if var(resp) > 0.2
%         continue;
%     end
% 
%     step_response(j, :) = resp;
%     input_segments(j, :) = Xseg;
% 
%     j = j + 1;
% 
%     if j > 500
%         break;
%     end
% end
% 
% valid_count = j - 1;
% disp("Valid steps: " + valid_count);
% 
% figure(10);
% clf(10);
% subplot(2,1,1);
% % xline(t_start);
% % xline(t_end);
% hold on
% plot(meas.sysTime, [meas.PIDRefX meas.PIDSensX]);
% ylim([-400 400]);
% usedParams = "PIDtoolbox" + newline ...
%     + sprintf("Px: %.f", header.Px) ...
%     + sprintf(", Ix: %.f", header.Ix) ...
%     + sprintf(", Dx: %.f", header.Dx) ...
%     + sprintf(", FFrx: %.f\n", header.FFrx) ...
%     + sprintf("satI: %.f", header.satI) ...
%     + sprintf(", satPID: %.f", header.satPID) ...
%     + sprintf(", DTermC: %.f", header.DTermC) ...
%     + sprintf(", GparamC: %.f", header.GparamC);
% title(usedParams, 'Interpreter', 'none');
% legend('PIDRefX', 'PIDSensX');
% hold off
% subplot(2,1,2);
% hold on
% plot(mean(step_response(1:valid_count, :), 1));
% yline(1,'--');
% xlim([1 wnd]);
% ylim([0 1.3]);
% hold off

