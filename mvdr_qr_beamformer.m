%% QR Decomposition-Based MVDR Beamformer
% Full MATLAB Reference Implementation
% Compatible with Versal FPGA fixed-point mapping
%
% Array Model: ULA (Uniform Linear Array)
% Algorithm  : MVDR via QR -> R-matrix -> back-substitution
% -------------------------------------------------------------------------

clear; clc; close all;

%% ============================
%  PARAMETERS
%  ============================
M          = 8;          % Number of array elements
N_snap     = 64;         % Number of snapshots (must be >= M)
f0         = 3e9;        % Carrier frequency (Hz)
c          = 3e8;        % Speed of light
lambda     = c / f0;     % Wavelength
d          = lambda / 2; % Element spacing (half-wavelength)

theta_s    = 30;         % Signal-of-interest (SOI) angle (degrees)
theta_j    = [-20, 50];  % Jammer angles (degrees)
SNR_dB     = 10;         % Signal SNR (dB)
JNR_dB     = 30;         % Jammer-to-noise ratio (dB)
noise_var  = 1.0;        % Noise variance (sigma^2)

%% ============================
%  STEERING VECTOR FUNCTION
%  ============================
sv = @(theta) exp(1j * 2*pi * d/lambda * (0:M-1)' * sind(theta));

%% ============================
%  GENERATE RECEIVED DATA
%  ============================
% Steering vectors
a_s  = sv(theta_s);            % SOI
a_j1 = sv(theta_j(1));         % Jammer 1
a_j2 = sv(theta_j(2));         % Jammer 2

% Signal amplitudes
sig_amp   = sqrt(10^(SNR_dB/10)  * noise_var);
jammer_amp= sqrt(10^(JNR_dB/10) * noise_var);

% Generate snapshots
s  = sig_amp    * (randn(1,N_snap) + 1j*randn(1,N_snap)) / sqrt(2);
j1 = jammer_amp * (randn(1,N_snap) + 1j*randn(1,N_snap)) / sqrt(2);
j2 = jammer_amp * (randn(1,N_snap) + 1j*randn(1,N_snap)) / sqrt(2);
n  = sqrt(noise_var/2) * (randn(M,N_snap) + 1j*randn(M,N_snap));

% Received array data  X: M x N_snap
X = a_s*s + a_j1*j1 + a_j2*j2 + n;

%% ============================
%  SAMPLE COVARIANCE MATRIX
%  ============================
R = (X * X') / N_snap;    % M x M Hermitian PD matrix

%% ============================
%  CHOLESKY FOR REFERENCE
%  (MVDR via direct inversion)
%  ============================
w_direct = (R \ a_s) / (a_s' * (R \ a_s));   % classical MVDR weights

%% ============================
%  QR-BASED MVDR
%  ============================
% Step 1: QR decomposition of X^H (N x M matrix)
%   X^H = Q * R_qr   =>   R_qr is upper triangular
%   X * X^H = R_qr^H * R_qr  (scaled by N_snap)
%
[Q_mat, R_qr] = qr(X' / sqrt(N_snap), 0);  % economy QR, R_qr is MxM upper-triangular

% Step 2: Forward solve R_qr^H * y = a_s  =>  y = R_qr^{-H} * a_s
%         (solves lower-triangular system)
y = R_qr' \ a_s;           % M x 1 complex vector

% Step 3: Back-solve  R_qr * z = y  =>  z = R_qr^{-1} * y = R^{-1} * a_s
z = R_qr \ y;              % M x 1

% Step 4: Normalize to satisfy distortionless constraint
%         w = z / (a_s^H * z)
denom = a_s' * z;
w_qr  = z / denom;         % MVDR weight vector via QR

%% ============================
%  VERIFY RESULTS
%  ============================
fprintf('\n=== Weight Vector Comparison ===\n');
fprintf('Max |w_direct - w_qr| = %.6e\n', max(abs(w_direct - w_qr)));

%% ============================
%  BEAMPATTERN
%  ============================
theta_scan = -90:0.5:90;
nTheta     = length(theta_scan);
BP_direct  = zeros(1, nTheta);
BP_qr      = zeros(1, nTheta);

for k = 1:nTheta
    a_k         = sv(theta_scan(k));
    BP_direct(k) = 20*log10(abs(w_direct' * a_k) + eps);
    BP_qr(k)     = 20*log10(abs(w_qr'     * a_k) + eps);
end

figure('Name','MVDR Beampattern');
plot(theta_scan, BP_direct, 'b-',  'LineWidth', 1.5); hold on;
plot(theta_scan, BP_qr,     'r--', 'LineWidth', 1.5);
xline(theta_s,   'g',  'LineWidth', 1.5, 'Label', 'SOI');
xline(theta_j(1),'m',  'LineWidth', 1.5, 'Label', 'J1');
xline(theta_j(2),'m',  'LineWidth', 1.5, 'Label', 'J2');
grid on; ylim([-80 10]);
xlabel('Angle (degrees)'); ylabel('Array Gain (dB)');
title('QR-MVDR vs Direct-MVDR Beampattern');
legend('Direct MVDR','QR MVDR','Location','Best');

%% ============================
%  SINR COMPUTATION
%  ============================
P_signal_direct = abs(w_direct' * a_s)^2 * sig_amp^2;
P_noise_direct  = noise_var * norm(w_direct)^2;
SINR_direct_dB  = 10*log10(P_signal_direct / P_noise_direct);

P_signal_qr     = abs(w_qr' * a_s)^2 * sig_amp^2;
P_noise_qr      = noise_var * norm(w_qr)^2;
SINR_qr_dB      = 10*log10(P_signal_qr / P_noise_qr);

fprintf('\n=== SINR ===\n');
fprintf('Direct MVDR SINR : %.2f dB\n', SINR_direct_dB);
fprintf('QR     MVDR SINR : %.2f dB\n', SINR_qr_dB);

%% ============================
%  EXPORT FIXED-POINT TEST DATA
%  (for C/HLS verification)
%  ============================
% Scale and quantize to Q1.15 (16-bit fixed-point)
scale    = 2^14;
X_fp     = round(real(X) * scale) + 1j * round(imag(X) * scale);
a_s_fp   = round(real(a_s) * scale) + 1j * round(imag(a_s) * scale);

% Write binary files for HLS testbench
fid = fopen('X_data.bin', 'wb');
fwrite(fid, [real(X_fp(:)); imag(X_fp(:))], 'int16');
fclose(fid);

fid = fopen('a_steer.bin', 'wb');
fwrite(fid, [real(a_s_fp); imag(a_s_fp)], 'int16');
fclose(fid);

% Write weights for golden reference
fid = fopen('w_golden.bin', 'wb');
fwrite(fid, [real(w_qr); imag(w_qr)], 'double');
fclose(fid);

fprintf('\nFixed-point test data exported.\n');

%% ============================
%  FIXED-POINT SIMULATION
%  (matches HLS word lengths)
%  ============================
mvdr_qr_fixed_point(X, a_s, M, N_snap);

%% ============================
%  HELPER: Fixed-Point MVDR-QR
%  ============================
function w_fp = mvdr_qr_fixed_point(X, a_s, M, N_snap)
    WL  = 16;   % word length
    IWL = 2;    % integer word length (Q1.14 approx)
    FWL = WL - IWL;

    % Quantize inputs
    q   = @(x) round(x * 2^FWL) / 2^FWL;

    X_q   = q(real(X))   + 1j*q(imag(X));
    a_q   = q(real(a_s)) + 1j*q(imag(a_s));

    % QR decomp on quantized data
    [~, R_qr] = qr(X_q' / sqrt(N_snap), 0);
    R_qr      = q(real(R_qr)) + 1j*q(imag(R_qr));

    % Forward-backward solve
    y    = q(real(R_qr' \ a_q)) + 1j*q(imag(R_qr' \ a_q));
    z    = q(real(R_qr  \ y))   + 1j*q(imag(R_qr  \ y));
    w_fp = z / (a_q' * z);

    fprintf('\n=== Fixed-Point QR MVDR ===\n');
    fprintf('||w_fp|| = %.6f\n', norm(w_fp));
end
