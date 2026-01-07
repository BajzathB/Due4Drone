clear all, clc

directory = "2026_01_05";
fileNumber = 8; %1 is ., 2 is ..

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


% % % plotting in time
% figure(1);
% clf(1);
% plot(meas.sysTime, [meas.GRawX meas.GPT1X meas.GPT2X]);
% % plot(meas.sysTime, [meas.GRawY meas.GPT1Y meas.GPT2Y]);
% xlabel('time');
% ylabel('rad/s');
% legend('raw','pt1','pt2');
% title('Gyro')

% figure(2);
% clf(2);
% % plot(meas.sysTime, [meas.ARawX meas.APT1X meas.APT2X]);
% % plot(meas.sysTime, [meas.ARawY meas.APT1Y meas.APT2Y]);
% plot(meas.sysTime, [meas.ARawZ meas.APT1Z meas.APT2Z]);
% xlabel('time');
% ylabel('m2/s');
% legend('raw','pt1','pt2');
% title('Acc')

% figure(3);
% clf(3);
% plot(meas.sysTime, [meas.aRawR meas.aPT2R meas.aKFRawR meas.aKFPT21R]);
% % plot(meas.sysTime, [meas.aPT2P meas.aKFPT21P]);
% xlabel('time');
% ylabel('°angle');
% legend('aRaw', 'apt2', 'aKFRaw', 'aKFPT21');
% title('Angle')

figure(4);
clf(4);
plot(meas.sysTime, [meas.aPT1R meas.aPT2R meas.aKFRawR meas.aKFPT10R meas.aKFPT11R]);
legend('aPT1', 'aPT2', 'aKFRaw', 'aKFPT10', 'aKFPT11');
usedParams = file + newline ...
    + sprintf(", GparamC: %.f", header.GparamC) ...
    + sprintf(", AparamC: %.f", header.AparamC) ...
    + sprintf(", KFQangle: %.6f", header.KFQAng) ...
    + sprintf(", KFQbias: %.6f", header.KFQbias) ...
    + sprintf(", KFRmeas: %.f", header.KFRmeas);
title(usedParams, 'Interpreter', 'none');


% %FFT
% In1 = transpose(meas.aPT2R);
% In2 = meas.aKFRawR;
% In3 = meas.aKFPT21R;
% L = length(meas.sysTime);  %length
% Fs = 100;   %sampling frequency
% f = Fs*(0:(L/2))/L;
% %In1
% Yin1 = fft(In1);
% P2in1 = abs(Yin1/L);
% P1in1 = P2in1(1:L/2+1);
% P1in1(2:end-1) = 2*P1in1(2:end-1);
% %In2
% Yin2 = fft(In2);
% P2in2 = abs(Yin2/L);
% P1in2 = P2in2(1:L/2+1);
% P1in2(2:end-1) = 2*P1in2(2:end-1);
% %In3
% Yin3 = fft(In3);
% P2in3 = abs(Yin3/L);
% P1in3 = P2in3(1:L/2+1);
% P1in3(2:end-1) = 2*P1in3(2:end-1);
% 
% figure(5)
% clf(5);
% plot(f,P1in1,f,P1in2,f,P1in3)
% legend('In1','In2','In3');
% ylim([0 10]);
% title('FFT');
% xlabel('f (Hz)')














