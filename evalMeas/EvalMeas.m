clear all, clc

directory = "2026_01_01";
fileNumber = 4; %1 is ., 2 is ..

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
% plot(meas.sysTime, [meas.GRawX meas.GPT1X meas.GPT2X]);
% % plot(meas.sysTime, [meas.GRawY meas.GPT1Y meas.GPT2Y]);
% xlabel('time');
% ylabel('rad/s');
% legend('raw','pt1','pt2');
% title('Gyro')

%% plot acc
figure(2);
clf(2);
plot(meas.sysTime, [meas.ARawX meas.APT1X meas.APT2X]);
% plot(meas.sysTime, [meas.ARawY meas.APT1Y meas.APT2Y]);
% plot(meas.sysTime, [meas.ARawZ meas.APT1Z meas.APT2Z]);
xlabel('time');
ylabel('m2/s');
legend('raw','pt1','pt2');
title('Acc')

%% plot angle
% figure(3);
% clf(3);
% plot(meas.sysTime, [meas.aRawR meas.aKFRawR meas.aKFPT10R meas.aKFPT11R]);
% % plot(meas.sysTime, [meas.aPT1R meas.aKFRawR meas.aKFPT21R]);
% % plot(meas.sysTime, [meas.aPT2P meas.aKFPT21P]);
% xlabel('time');
% ylabel('°angle');
% legend('aRaw', 'aKFRaw', 'aKFPT10', 'aKFPT11', 'aKFPT21');
% title('Angle')

%% plot plot angle with parameters
figure(4);
clf(4);
plot(meas.sysTime, [meas.aPT1R meas.aKFRawR meas.aKFPT10R meas.aKFPT11R]);
legend('aPT1', 'aKFRaw', 'aKFPT10', 'aKFPT11');
usedParams = file + newline ...
    + sprintf(", GparamC: %.f", header.GparamC) ...
    + sprintf(", AparamC: %.f\n", header.AparamC) ...
    + sprintf("KFQangle: %.6f", header.KFQAng) ...
    + sprintf(", KFQbias: %.6f", header.KFQbias) ...
    + sprintf(", KFRmeas: %.f", header.KFRmeas);
title(usedParams, 'Interpreter', 'none');

%% FFT
In1 = transpose(meas.aPT2R);
In2 = meas.aKFRawR;
In3 = meas.aKFPT21R;
L = length(meas.sysTime);  %length
Fs = 100;   %sampling frequency
f = Fs*(0:(L/2))/L;
%In1
Yin1 = fft(In1);
P2in1 = abs(Yin1/L);
P1in1 = P2in1(1:L/2+1);
P1in1(2:end-1) = 2*P1in1(2:end-1);
%In2
Yin2 = fft(In2);
P2in2 = abs(Yin2/L);
P1in2 = P2in2(1:L/2+1);
P1in2(2:end-1) = 2*P1in2(2:end-1);
%In3
Yin3 = fft(In3);
P2in3 = abs(Yin3/L);
P1in3 = P2in3(1:L/2+1);
P1in3(2:end-1) = 2*P1in3(2:end-1);

figure(5)
clf(5);
plot(f,P1in1,f,P1in2,f,P1in3)
legend('In1','In2','In3');
ylim([0 10]);
title('FFT');
xlabel('f (Hz)')

%% recalc KF

% Q = [0.0001  0.003];
% R = 20;
% P_aKFRawR = eye(2);
% P_aKFPT1R = eye(2);
% aKFRawR(1) = 1;
% aKFPT1R(1) = 1;
% angle_est_aKFRawR = meas.aRawR(1);
% angle_est_aKFPT1R = meas.aRawR(1);
% bias_est_aKFRawR = 0;
% bias_est_aKFPT1R = 0;
% 
% for i = 2:length(meas.sysTime)
%     dt =meas.sysTime(i) - meas.sysTime(i-1);
%     
%     [aKFRawR(i), P_aKFRawR, angle_est_aKFRawR, bias_est_aKFRawR] = kalmanAngle(meas.aRawR(i), meas.GRawX(i), dt, angle_est_aKFRawR, bias_est_aKFRawR, P_aKFRawR, Q, R);
%     [aKFPT1R(i), P_aKFPT1R, angle_est_aKFPT1R, bias_est_aKFPT1R] = kalmanAngle(meas.aRawR(i), meas.GRawX(i), dt, angle_est_aKFRawR, bias_est_aKFPT1R, P_aKFPT1R, Q, R);
%     
% end
% 
% figure(6);
% clf(6);
% % plot(meas.sysTime, [meas.aKFRawR aKFRawR']);
% % legend('OG\_aKFRawR', 'RE\_aKFRawR');
% plot(meas.sysTime, [meas.aKFRawR meas.aKFPT10R aKFRawR' aKFPT1R']);
% legend('OG\_aKFRawR', 'OG\_aKFPT1R', 'RE\_aKFRawR', 'RE\_aKFPT1R');
% title('Recalc');

%% complementary filter

compV1_angle(1) = meas.aRawR(1);
alpha = 0.995;

compV2_angle(1) = meas.aRawR(1);
gyro_bias = 0;
Kp = 0.01;
Ki = 0.001;

compV3_angle(1) = meas.aRawR(1);

for i = 2:length(meas.sysTime)
    dt =meas.sysTime(i) - meas.sysTime(i-1);
    
    compV1_angle(i) = complementaryV1(compV1_angle(i-1), meas.GRawX(i), meas.aRawR(i), dt, alpha);
    [compV2_angle(i), gyro_bias] = complementaryV2(compV2_angle(i-1), meas.GRawX(i), meas.aRawR(i), dt, gyro_bias, Kp, Ki);
    compV3_angle(i) = complementaryV3(compV1_angle(i-1), meas.GRawX(i), meas.aRawR(i), dt, meas.ARawX(i), meas.ARawY(i), meas.ARawZ(i), alpha);
    
    
end

figure(7);
clf(7);
plot(meas.sysTime, [meas.aPT2R compV1_angle' compV2_angle' compV3_angle']);
legend('OG\_aKFRawR', 'compV1R', 'compV2R', 'compV3R');
title('Recalc');



