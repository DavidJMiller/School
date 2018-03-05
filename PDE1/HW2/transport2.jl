#transport code
using Winston

p = FramedPlot(
		title = "1D Heat Equation",
		xlabel = "x",
		ylabel = "u")

k = 0.5
c = 100.0
L = 5.0
N = 20
function initcond(xx::Vector{Float64}, tt::Float64)
    u = 0.0
    for n = 1:N
        Bn = (8*L^3/(n^3*pi^3))
        Cn = 1.0*cos((100.0*n*pi*tt)/5.0)
        #u = u + Bn*sin(n*pi*xx/L)*exp(-1*n^2*pi^2*k*tt/L^2)
        u = u + Bn*Cn*sin((n*pi*xx)/L)
    end 
    return u
end

dx = 0.001
xx = collect(0:dx:L)
colors = ("red", "green", "blue", "orange", "black", "purple")
for tt=0:5
	t = 1.0*tt
    uu = initcond(xx, t)
    c = Curve(xx, uu, color = colors[tt+1])
    setattr(c, label = string("t = ", t))
    l = Legend(.8, (.925 - tt * 0.05), [c])
	add(p, c, l)
end
figure();
display(p);
savefig(p, "HW2_3a.png")