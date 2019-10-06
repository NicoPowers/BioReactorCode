f = 1; % Hz
distance = 10; % mm
period = (1/f)*1000; % ms
stepRes = 10; %ms
t = round(0:stepRes:period);
steps = 3905.5;
omega = pi*f/1000; %rad/ms
pos = round(steps*sin(omega*t).^2);
speed = round(steps*sin(omega.*t).*cos(omega.*t));
acceleration = (round(steps*((cos(omega.*t).^2) - (sin(omega.*t).^2))));
figure;
hold on;
plot(t, pos);
plot(t, speed);
plot(t, acceleration);