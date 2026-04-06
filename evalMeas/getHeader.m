function header = getHeader(fileID)
%GETHEADER Reads in the header part of a file
%   Reads in the 1st, 2nd and 3rd line, then extracts parameters from it
line1 = fgetl(fileID);
line2 = fgetl(fileID);
line3 = fgetl(fileID);
headerText = strsplit(line2, ',');   % line 2
formatSpec = repmat('%f', 1, numel(headerText));
headerData = textscan(line3, formatSpec, 1, ...
    'Delimiter', ',', ...
    'CollectOutput', true);
header = array2table(headerData{1}, 'VariableNames', headerText);
end

