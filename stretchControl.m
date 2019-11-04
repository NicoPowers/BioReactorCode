f = 1; % Hz
period = (1/f); % s
numSteps = 100; 
timePerStep = period/numSteps;
t = 0:timePerStep:period/2;
steps = 3905.5;
omega = pi*f; %rad/s
pos = round(steps*sin(omega*t).^2);
speed = round(steps*sin(omega.*t).*cos(omega.*t));
acceleration = abs(round(steps*((cos(omega.*t).^2) - (sin(omega.*t).^2))));
figure;
hold on;
plot(t, pos);
plot(t, speed);
plot(t, acceleration);