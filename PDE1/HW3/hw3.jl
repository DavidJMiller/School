#transport code
# using Winston

# p = FramedPlot(
# 		title = "1D Wave Equation (D'Alembert)",
# 		xlabel = "x",
# 		ylabel = "u")

# k = 0.5
# c = 100.0
# L = 10.0
# N = 20
# function initcond(xx::Vector{Float64}, tt::Float64)
#     u = 0.0;
#     u = 0.5*(-1*exp(-0.5*(xx + tt).^2) + exp(-0.5*(xx - tt).^2))
#     return u
# end

# dx = 0.01
# xx = collect(-L:dx:L)
# colors = ("red", "green", "blue", "orange", "black", "purple")
# for tt=0:5
# 	t = 1.0*tt
#     uu = initcond(xx, t)
#     c = Curve(xx, uu, color = colors[tt+1])
#     setattr(c, label = string("t = ", t))
#     l = Legend(.75, (.35 - tt * 0.05), [c])
# 	add(p, c, l)
# end
# figure();
# display(p);
# savefig(p, "HW3_1d.png")

using Winston

p = FramedPlot(
        title = "1D Wave Equation (D'Alembert)",
        xlabel = "x",
        ylabel = "u")

k = 0.5
C = 100.0
L = 5
N = 20
function initcond(xx::Vector{Float64}, tt::Float64)
    xx = xx - C*tt
    fg = (5/2)*xx- (1/2)*xx.^2
    if (tt >= 0) && ((tt % 2L) <= L)
        return fg;
    elseif (tt >= 0) && ((tt % 2L) > L)
        return -fg;
    elseif (tt < 0) && ((tt % 2l) > L)
        return -fg;
    else
        return fg;  
    end
end

dx = 0.01
xx = collect(0:dx:L)
colors = ("red", "green", "blue", "orange", "black", "purple")
for tt=0:5
    t = .01*tt
    uu = initcond(xx, t)
    c = Curve(xx, uu, color = colors[tt+1])
    setattr(c, label = string("t = ", t))
    l = Legend(.75, (.35 - tt * 0.05), [c])
    add(p, c, l)
end
figure();
display(p);
savefig(p, "HW3_1d.png")   