function [] = code1(theta)

N = 1000;
xylim = 5;
domain  = linspace(0,2*pi,N);
r = exp(1i * domain);

f = @(r,y) (r - 1) ./ (y * r + 1 - y);

for i = 0 : 10

  xi = f(r, .1 * i * theta);
  plot(real(xi), imag(xi), 'LineWidth', 2);
  hold on;

end

Legend=cell(11,1)
Legend{1} = '\theta = 0';
Legend{2} = '\theta = 0.1';
Legend{3} = '\theta = 0.2';
Legend{4} = '\theta = 0.3';
Legend{5} = '\theta = 0.4';
Legend{6} = '\theta = 0.5';
Legend{7} = '\theta = 0.6';
Legend{8} = '\theta = 0.7';
Legend{9} = '\theta = 0.8';
Legend{10} = '\theta = 0.9';
Legend{11} = '\theta = 1';
legend(Legend);

title('Absolute Stability');
xlabel('Real(h\lambda)');
ylabel('Imag(h\lambda)');
ylim([-xylim xylim])
xlim([-xylim xylim])

saveas(gcf, '5_2_3.png');