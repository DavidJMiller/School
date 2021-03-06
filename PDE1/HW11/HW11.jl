#transport code
using Winston

p = FramedPlot(
        title = "1D Heat PDE",
        xlabel = "x",
        ylabel = "u")

dx = 0.1
L = 10.0
xx = collect(0:dx:L)
truncation = 20

function bessel(xx::Vector{Float64}, nn::Float64)
    u = 0.0
    for i = 0:truncation
        u = u + (((-1.0)^i) ./ (factorial(i) * factorial(nn + i))) .* (0.5 .* xx) .^ (nn + 2*i)
    end 
    return u;
end

function approx(xx::Vector{Float64}, nn::Float64)
    u = 0.0
    u = sqrt(2.0 ./ (pi * xx)) .* cos(xx - pi / 4.0 - (nn * pi) / 2.0)
    return u;
end

colors = ("red", "green", "blue", "orange", "black", "purple")
for nn=0:1
    nn = 1.0 * nn
    uu = bessel(xx, nn)
    c = Curve(xx, xx, color = colors[nn + 1])
    setattr(c, label = string("Bessel n = ", nn))
    l = Legend(.8, (.925 - nn * 0.1), [c])
    add(p, c, l)
    #uu = approx(xx, nn)
    #c = Curve(xx, uu, color = colors[2 * nn + 2])
    #setattr(c, label = string("Approx (n = ", nn, ")"))
    #l = Legend(.8, (.925 - (2 * nn + 3) * 0.05), [c])
    #dd(p, c, l)
end

figure();
display(p);
savefig(p, "HW2_3b2.png")