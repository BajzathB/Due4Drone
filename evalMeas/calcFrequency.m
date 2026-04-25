function frequency = calcFrequency(data, length)
%CALCFREQUENCY Calculate the FFT of a measured data

%In1
Yin1 = fft(data);
P2in1 = abs(Yin1/length);
P1in1 = P2in1(1:floor(length/2) + 1);
P1in1(2:end-1) = 2*P1in1(2:end-1);

frequency = P1in1;
end

