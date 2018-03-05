function [] = code1()

N = 1000;
th = linspace(0,2*pi,N);
r = exp(1i*th);
z = r - 1;
hold on;

%{
f = @(r) (r);
xi = f(r);
plot(z ./ xi, 'LineWidth',2);

f = @(r) (1*r + 1) / 2;
xi = f(r);
plot(z ./ xi, 'LineWidth',2);

f = @(r) (5*r + 8 - 1 ./ r) / 12;
xi = f(r);
plot(z ./ xi, 'LineWidth',2);
%}
f = @(r) (9*r + 19 - 5 ./ r + 1 ./ (r.^2)) / 24;
xi = f(r);
plot(z ./ xi, 'LineWidth',2);
%{
f = @(r) (251*r + 646 - 264 ./ r + 106 ./ (r.^2) - 19 ./ (r.^3)) ./ 720;
xi = f(r);
plot(z ./ xi, 'LineWidth',2);

f = @(r) (475*r + 1427 - 798 ./ (r) + 482 ./ (r.^2) -173 ./ (r.^3) + 27 ./ (r.^4)) / 1440;
xi = f(r);
plot(z ./ xi, 'LineWidth',2);

Legend=cell(6,1)
Legend{1} = 'k = 1';
Legend{2} = 'k = 2';
Legend{3} = 'k = 3';
Legend{4} = 'k = 4';
Legend{5} = 'k = 5';
Legend{6} = 'k = 6';
legend(Legend);
%}

Legend=cell(1,1)
Legend{1} = 'k = 4';
legend(Legend);

xlim([-4 1])

title('Adams-Moulton Absolute Stability (k=4)');
xlabel('Real(h\lambda)');
ylabel('Imag(h\lambda)');
saveas(gcf, '5_2_5_single.png');