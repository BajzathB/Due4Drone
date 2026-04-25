clear all, clc

%reading files from folder
directory = "2026_04_06";
files = dir(directory);
files = files(3:end);
% fileList = strings(1, length(files));
% for i = 1:length(files)
%     fileList(i) = files(i).name;
% end
% disp(fileList(1:3));
% disp("...");
% disp(fileList(end-2:end));

%manually select files
fileList = [
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

% % "MEAS558.txt"
% % "MEAS559.txt"
"MEAS560.txt"
% % "MEAS561.txt"
% % "MEAS562.txt"

];


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

trim = 500;
figure(1)
clf(1)
% plot([meas{1,1}.PIDRefX(trim:end-trim) meas{1,1}.PIDSensX(trim:end-trim)]);
% plot([meas{1,1}.PIDRefY(trim:end-trim) meas{1,1}.PIDSensY(trim:end-trim)]);
plot([meas{1,1}.PIDRefZ(trim:end-trim) meas{1,1}.PIDSensZ(trim:end-trim)]);

measSize = size(meas);
for s = 1:measSize(2)
    stepResp{1,s} = calcStepResponse(meas{1,s}.PIDRefX(trim:end-trim), meas{1,s}.PIDSensX(trim:end-trim));
    stepResp{2,s} = calcStepResponse(meas{1,s}.PIDRefY, meas{1,s}.PIDSensY);
%     stepResp{3,s} = calcStepResponse(meas{1,s}.PIDRefZ, meas{1,s}.PIDSensZ);
end

%%
wnd = 1024;

figure(1); clf; hold on
legend('Location','eastoutside');
title("PID tuning ROLL");
yline(1,'--');
xlim([1 wnd]);
ylim([0 1.3]);

figure(1); clf;
ax1 = subplot(2,1,1);
hold(ax1, 'on');
title(ax1, "PID tuning" + newline + "ROLL");
xlim(ax1, [1 wnd]);
ylim(ax1, [0 1.3]);
ax2 = subplot(2,1,2);
hold(ax2, 'on');
title(ax2, "PITCH");
xlim(ax2, [1 wnd]);
ylim(ax2, [0 1.3]);
legend(ax1, 'Location','eastoutside');
legend(ax2, 'Location','eastoutside');

figure(2); clf; hold on
subplot(2,1,1);
legend('Ref', 'Sens');
subplot(2,1,2);
title("PITCH");
legend('Ref', 'Sens');

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
    
    plot(ax1, stepResp{1,p}, 'DisplayName', labelX);
    plot(ax2, stepResp{2,p}, 'DisplayName', labelY);
    
    figure(2)
    clf;
    subplot(2,1,1);
    plot([meas{1,p}.PIDRefX(trim:end-trim) meas{1,p}.PIDSensX(trim:end-trim)]);
    title("gyro" + newline + "ROLL");
    legend('Ref', 'Sens');
    subplot(2,1,2);
    plot([meas{1,p}.PIDRefY(trim:end-trim) meas{1,p}.PIDSensY(trim:end-trim)]);
    title("PITCH");
    legend('Ref', 'Sens');
end
figure(1); hold off
figure(2); hold off

%% Manual step windows for X

% manualWindow_560 = [
%     18 90
%     835 912
%     980 1045
%     1200 1290
%     1470 1530
%     1550 1615
%     2645 2720
%     2840 2940
%     3045 3099
%     3277 3370
%     4191 4311
%     4610 4715
%     7162 7380
%     7648 7898
%     8185 8351
%     8351 8495
%     8698 8845
% ];
% 
% figure(3); clf; hold on 
% ax1 = subplot(3,1,1);
% ax2 = subplot(3,1,2);
% hold(ax1, 'on');
% hold(ax2, 'on');
% for i = 1:size(manualWindow_560,1)
%     startIdx = manualWindow_560(i,1) + trim;
%     endIdx   = manualWindow_560(i,2) + trim;
%     
%     plot(ax1, [meas{1,1}.PIDRefX(startIdx:endIdx) meas{1,1}.PIDSensX(startIdx:endIdx)]);
%     
%     manualStepResp{i} = calcManualStepResponse(meas{1,1}.PIDRefX(startIdx:endIdx), meas{1,1}.PIDSensX(startIdx:endIdx));
%     plot(ax2, manualStepResp{i});
%     labels(i) = (startIdx-trim) + " - " + (endIdx-trim);
% end
% legend(ax2, labels, 'Location','eastoutside');
% 
% % cut the steps to the shortest
% minLen = min(cellfun(@length, manualStepResp));
% manualStepResp_truncated = cellfun(@(x) x(1:minLen), manualStepResp, 'UniformOutput', false);
% manualStepResp_matrix = cell2mat(manualStepResp_truncated(:));
% meanManualResp = mean(manualStepResp_matrix, 1);
% 
% figure(3)
% subplot(3,1,3);
% plot(meanManualResp);
% title("Manual mean step response X");
% ylim([0 1.2]);
% yline(1,'--');
% 
% figure(3); hold off

%% Manual step windows for Y

% manualWindow_560 = [
%     447 525
%     525 595
%     662 726
%     1772 1835
%     3195 3401
% ];
% 
% figure(3); clf; hold on 
% ax1 = subplot(3,1,1);
% ax2 = subplot(3,1,2);
% hold(ax1, 'on');
% hold(ax2, 'on');
% for i = 1:size(manualWindow_560,1)
%     startIdx = manualWindow_560(i,1) + trim;
%     endIdx   = manualWindow_560(i,2) + trim;
%     
%     plot(ax1, [meas{1,1}.PIDRefY(startIdx:endIdx) meas{1,1}.PIDSensY(startIdx:endIdx)]);
%     
%     manualStepResp{i} = calcManualStepResponse(meas{1,1}.PIDRefY(startIdx:endIdx), meas{1,1}.PIDSensY(startIdx:endIdx));
%     plot(ax2, manualStepResp{i});
%     labels(i) = (startIdx-trim) + " - " + (endIdx-trim);
% end
% legend(ax2, labels, 'Location','eastoutside');
% 
% % cut the steps to the shortest
% minLen = min(cellfun(@length, manualStepResp));
% manualStepResp_truncated = cellfun(@(x) x(1:minLen), manualStepResp, 'UniformOutput', false);
% manualStepResp_matrix = cell2mat(manualStepResp_truncated(:));
% meanManualResp = mean(manualStepResp_matrix, 1);
% 
% figure(3)
% subplot(3,1,3);
% plot(meanManualResp);
% title("Manual mean step response Y");
% ylim([0 1.3]);
% yline(1,'--');
% 
% figure(3); hold off
