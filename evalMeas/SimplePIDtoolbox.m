clear all, clc

%reading files from folder
directory = "2026_04_06";
files = dir(directory);
files = files(3:end);
fileList = strings(1, length(files));
for i = 1:length(files)
    fileList(i) = files(i).name;
end
disp(fileList(1:3));
disp("...");
disp(fileList(end-2:end));

%manually select files
% fileList = [
% % % "MEAS486.txt"
% % % "MEAS487.txt"
% % "MEAS488.txt"
% % %P-450
% % % "MEAS491.txt"
% % % "MEAS493.txt"
% % "MEAS494.txt"
% % % %P-45
% % % "MEAS501.txt"
% % % "MEAS502.txt"
% % % "MEAS504.txt"
% % % %P-43
% % "MEAS512.txt"
% % % "MEAS513.txt"
% % % "MEAS515.txt"
% % % P-38
% % % "MEAS517.txt"
% % "MEAS518.txt"
% % % "MEAS519.txt"
% %P-35
% % "MEAS521.txt"
% "MEAS522.txt"
% % "MEAS523.txt"
% %P-30
% "MEAS526.txt"
% "MEAS528.txt"
% % "MEAS529.txt"
% %P-28
% % "MEAS530.txt"
% "MEAS531.txt"
% % "MEAS533.txt"
% % "MEAS534.txt"
% % %P-25
% % % "MEAS535.txt"
% % "MEAS536.txt"
% % % "MEAS537.txt"
%pitch
% %P-50
% "MEAS545.txt"
% "MEAS546.txt"
% "MEAS548.txt"
%P-45
% "MEAS552.txt"
% "MEAS553.txt"
% %P-43
% "MEAS555.txt"

% ];


for n = 1:length(fileList)
    file = directory + "/" + fileList(n);
    %reading meas header
    fileID = fopen(file,'r');
    headers{n} = getHeader(fileID);
    fclose(fileID);
    %reading meas data
    opts = detectImportOptions(file, ...
        'NumHeaderLines', 3, ...
        'Delimiter', ',');
    meas{n} = readtable(file, opts);
end

measSize = size(meas);
trim = 500;
for s = 1:measSize(2)
    stepResp{1,s} = calcStepResponse(meas{1,s}.PIDRefX(trim:end-trim), meas{1,s}.PIDSensX(trim:end-trim));
    stepResp{2,s} = calcStepResponse(meas{1,s}.PIDRefY, meas{1,s}.PIDSensY);
%     stepResp{3,s} = calcStepResponse(meas{1,s}.PIDRefZ, meas{1,s}.PIDSensZ);
end
%%
wnd = 1024;                 % 500 ms step response window

figure(1); clf; hold on
legend('Location','eastoutside');
title("PID tuning ROLL");
yline(1,'--');
xlim([1 wnd]);
ylim([0 1.3]);

figure(1); clf;
ax1 = subplot(2,1,1); % top plot (X)
hold(ax1, 'on');
title(ax1, "PID tuning" + newline + "ROLL");
xlim(ax1, [1 wnd]);
ylim(ax1, [0 1.3]);
ax2 = subplot(2,1,2); % bottom plot (Y)
hold(ax2, 'on');
title(ax2, "PITCH");
xlim(ax2, [1 wnd]);
ylim(ax2, [0 1.3]);
legend(ax1, 'Location','eastoutside');
legend(ax2, 'Location','eastoutside');

figure(2); clf; hold on
title("gyro ROLL");
legend('Ref', 'Sens', 'Location','eastoutside');

for p = 1:measSize(2)
    fileName = erase(fileList(p), ".txt");
    
    labelX = sprintf('%s(%g,%g,%g)', ...
        fileName, ...
        headers{1,p}.Px, ...
        headers{1,p}.Ix, ...
        headers{1,p}.Dx);
    labelY = sprintf('%s(%g,%g,%g)', ...
        fileName, ...
        headers{1,p}.Py, ...
        headers{1,p}.Iy, ...
        headers{1,p}.Dy);
    
%     figure(1)
%     plot(stepResp{1,p}, 'DisplayName', labelX);
    
    plot(ax1, stepResp{1,p}, 'DisplayName', labelX);
    plot(ax2, stepResp{2,p}, 'DisplayName', labelY);
    
    figure(2)
    clf;
    plot([meas{1,p}.PIDRefX(trim:end-trim) meas{1,p}.PIDSensX(trim:end-trim)]);
end
figure(1); hold off
figure(2); hold off

