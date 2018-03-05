function [] = code1()

N = 1000;
th = linspace(0,2*pi,N);
r = exp(1i*th);
hold on;
%{
f = @(r) (r - 1) ./ r;
xi = f(r);
plot(real(xi),imag(xi), 'LineWidth', 2);

f = @(r) (3*r.^2 - 4*r + 1) ./ (2*r.^2);
xi = f(r);
plot(real(xi),imag(xi), 'LineWidth', 2);
%}
f = @(r) (11*r.^3 - 18*r.^2 + 9*r - 2) ./ (6*r.^3);
xi = f(r);
plot(real(xi),imag(xi), 'LineWidth', 2);
%{
f = @(r) (25*r.^4 - 48*r.^3 + 36*r.^2 - 16*r + 3) ./ (12*r.^4);
xi = f(r);
plot(real(xi),imag(xi), 'LineWidth', 2);

f = @(r) (137*r.^5 - 300*r.^4 + 300*r.^3 - 200*r.^2 + 75*r - 12) ./ (60*r.^5);
xi = f(r);
plot(real(xi),imag(xi), 'LineWidth', 2); 

f = @(r) (147*r.^6 - 360*r.^5 + 450*r.^4 - 400*r.^3 + 225*r.^2 - 72*r + 10) ./ (60*r.^6);
xi = f(r);
plot(real(xi),imag(xi), 'LineWidth', 2); 

Legend = cell(6,1)
Legend{1} = 'k = 1';
Legend{2} = 'k = 2';
Legend{3} = 'k = 3';
Legend{4} = 'k = 4';
Legend{5} = 'k = 5';
Legend{6} = 'k = 6';
%}
Legend = cell(1,1);
Legend{1} = 'k = 3';
legend(Legend);

set(findobj(gcf,'type','axes'),'FontName','Arial','FontSize',12,'FontWeight','Bold', 'LineWidth', 2);

title('Absolute Stability BDF (k = 3)');
ylabel('Imag(h\lambda)');
saveas(gcf, '5_2_2_1.png');