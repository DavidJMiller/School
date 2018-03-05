#transport code
using Winston

p = FramedPlot(
        title = "1D Oxygen Equation",
        xlabel = "x",
        ylabel = "u")

k = 0.5
c = 100.0
L = 5.0
N = 10
function initcond(xx::Vector{Float64}, tt::Float64)
    u = 1 - .79 * xx / L
    for n = 1:N
        An = -1.58 / (n * pi)
        u = u + An * exp(-k * n^2 * pi^2 * tt / L^2) * sin(n * pi * xx / L)
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
savefig(p, "Q5_10.png")