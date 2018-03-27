clear all; close all; clc;
tic
meshSize = 1024 ;   % mesh size
patchSize = 2000  ; % patch size
direction=90; % wind direction
[windx , windy] = pol2cart( deg2rad(direction) , 1) ; 
windSpeed = 10;     % wind speed
timeStep = 1/10;    % time step between neighbor time slots
Times=1 ;        % total timeslots

rng(13);         % setting seed for random numbers
A = 1e-7;        % parameter
g = 9.81;        % gravitational constant
% practical significant wave height
swaveheight = windSpeed ^ 2 / g * 0.209 ; 
% In order to satisfy conjugate symmetric property which inverse fft will be real 
% meshSize should be an odd number. At last we will padding zeros and
% change meshSize back
meshSize = meshSize - 1 ;  

gridSize = meshSize * [1 1] ;
i = 1:meshSize; j = 1:meshSize;
meshLim = pi * meshSize / patchSize ;
N = linspace(-meshLim , meshLim , meshSize ) ;
M = linspace(-meshLim , meshLim , meshSize ) ;
[Kx,Ky] = meshgrid(N,M) ;

K = sqrt(Kx.^2 + Ky.^2); % vector K
W = sqrt(K .* g); % deep water frequencies

% phillips spectrum
P = zeros(size(gridSize));
for i = 1:meshSize
    for j = 1:meshSize
            k_sq = Kx(i,j)^2 + Ky(i,j)^2;
            if k_sq == 0
                P = 0;
            else
                L = windSpeed^2 / g;
                k = [Kx(i,j), Ky(i,j)] / sqrt(k_sq);
                WK = k(1) * windx + k(2) * windy;
                P(i,j) = A / k_sq^2 * exp(-1.0 / (k_sq * L^2)) * WK^2;
                if WK < 0
                    P(i,j) = 0;
                end
            end
    end
end

%mesh(P);
%simulated significant wave height
power=sum(sum(2*P))/meshSize^4;
sswh=4*sqrt(power);

% construct H0
H0 = 1/sqrt(2) .* (randn(gridSize) + 1i .* randn(gridSize)) .* sqrt(P);

%This parameter use to convert simulated height to practical height
ratio=swaveheight/sswh;
  
%infile
%file=fopen('ocean_data_1024_2000_15_0.10_500.txt','wt');
file=fopen('data.txt','wt');
fprintf(file,'%d\t',meshSize+1);
fprintf(file,'%d\t',patchSize);
fprintf(file,'%.5f\t',windSpeed);
fprintf(file,'%.5f\t',timeStep);
fprintf(file,'%d\n',Times);
 
%The reason we start from 500 is because the ocean wave is not
%fully oscillated at first.
 for t1 = 500+1:500+Times
     Ht = H0 .* exp(1i .* W .* (t1 * timeStep)) + ...
            conj(flip(flip(H0,1),2)) .* exp(-1i .* W .* (t1 * timeStep));
        
     padzeros1=zeros(1,meshSize+1);
     padzeros2=zeros(meshSize,1);
     Ht=[padzeros2 Ht];%pad zero on one row and one colomn
     Ht=[padzeros1; Ht];

     Output=real(ifft2(Ht));
     for i = 1:meshSize+1
         for j = 1:meshSize+1
             if mod(i+j,2) == 0
                sign = -1; % works fine if we change signs vice versa
             else
                sign = 1;
             end
             Output(i,j) = Output(i,j) * sign;
         end
     end

     for i=1:meshSize+1
         for j=1:meshSize+1
             if j==meshSize+1
                 fprintf(file,'%.5f\n',ratio*Output(i,j));
             else
                 fprintf(file,'%.5f\t',ratio*Output(i,j));
             end
         end
     end
 end

 fclose(file);
 