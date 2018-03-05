function [] = code1()

N = 1000;
th = linspace(0,2*pi,N);
r = exp(1i*th);
hold on;

f = @(r) (r-1);
xi = f(r);
plot(real(xi), imag(xi), 'LineWidth',2);

f = @(r) 2*(r.^2-r)./(3*r-1);
xi = f(r);
plot(real(xi), imag(xi), 'LineWidth',2);

f = @(r) 12*(r.^3-r.^2)./(23*r.^2 - 16*r + 5);
xi = f(r);
plot(real(xi), imag(xi), 'LineWidth',2);


f = @(r) 24*(r.^4-r.^3)./(55*r.^3 - 59*r.^2 + 37*r - 9);
xi = f(r);
plot(real(xi), imag(xi), 'LineWidth',2);


Legend = cell(4,1)
Legend{1} = 'k = 1';
Legend{2} = 'k = 2';
Legend{3} = 'k = 3';
Legend{4} = 'k = 4';

%Legend = cell(1,1);
%Legend{1} = 'k = 4'
legend(Legend);

title('Adams-Bashforth Absolute Stability (k = 4)');
xlabel('Real(h\lambda)');
ylabel('Imag(h\lambda)');
saveas(gcf, '5_2_2.png');