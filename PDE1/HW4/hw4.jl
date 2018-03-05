#transport code
using Winston

p = FramedPlot(
		title = "1D Heat PDE (Left Boundary)",
		xlabel = "x",
		ylabel = "u")

k = 0.5
w = 100.0
L = 1.0
N = 100
function func(xx::Vector{Float64}, tt::Float64)
    u = 0.0
    u += ((L^2)/6)
    for n = 1:N
        n = 2*n;
        An = (-4*(L^2))/(n^2*pi^2)
        u += An*cos(n*pi*xx/L)*exp((-k*(n^2)*(pi^2)*tt)/(L^2))
    end 
    return u
end

dx = 0.001
xx = collect(0:dx:L/25)
colors = ("red", "green", "blue", "orange", "black", "purple")
for tt=0:100
	t = tt/10000.0
    uu = func(xx, t)
    c = Curve(xx, uu)
    #setattr(c, label = string("t = ", t))
    #l = Legend(.8, (.925 - tt * 0.05), [c])
	add(p, c)
end
figure();
display(p);
savefig(p, "1d_2.png")	