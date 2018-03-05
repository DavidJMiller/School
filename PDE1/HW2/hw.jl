#transport code
using Winston

p = FramedPlot(
		title = "1D Heat PDE",
		xlabel = "x",
		ylabel = "u")

k = 0.5
w = 100.0
L = 1.0
N = 10
function func(xx::Vector{Float64}, tt::Float64)
    u = 0.0
    u += (L^2/6)
    for n = 1:N
        An = (-4*L^2)/(n^2*pi^2)
        u += An*cos(n*pi*xx/L)*exp((-k*n^2*pi^2*tt)/L^2)
    end 
    return u
end

dx = 0.001
xx = collect(0:dx:L)
colors = ("red", "green", "blue", "orange", "black", "purple")
for tt=0:5
	t = tt/10.0
    uu = func(xx, t)
    c = Curve(xx, uu, color = colors[tt+1])
    setattr(c, label = string("t = ", t))
    l = Legend(.8, (.925 - tt * 0.05), [c])
	add(p, c, l)
end
figure();
display(p);
savefig(p, "HW2_3b2.png")