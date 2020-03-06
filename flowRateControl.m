period = 1;
numSteps = 100; 
maxFlowRate = 43;
shift = maxFlowRate/2;
phase = 0;
timePerStep = period/numSteps;
t = 0:timePerStep:period;
omega = pi; %rad/s
pos = shift + maxFlowRate*sin(omega*t + phase).^2;
speed = shift + maxFlowRate*sin((omega.*t)+phase).*cos((omega.*t)+phase);
acceleration = abs(maxFlowRate*((cos(omega.*t).^2) - (sin(omega.*t).^2)));
figure;
hold on;
plot(t, pos);
%plot(t, speed);
%plot(t, acceleration);
%legend({'position', 'speed', 'acceleration'});