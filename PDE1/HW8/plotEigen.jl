#Dirichlet Kernel code
using Winston
using LaTeXStrings

p = FramedPlot(
        title = "Eigen Functions",
        xlabel = L"x",
        ylabel = L"X_n(x)")

K = 1
L = 1.0
function eigenfunc(xx::Vector{Float64}, lambda::Float64)
    val = 0
    val = (cosh.((lambda)^(1/4).*xx) - cos.((lambda)^(1/4).*xx) -
    ((cosh((lambda)^(1/4)) + cos((lambda)^(1/4))) / 
    (sinh((lambda)^(1/4)) + sin((lambda)^(1/4)))) * 
    (sinh.((lambda)^(1/4).*xx) - sin.((lambda)^(1/4).*xx)))
    return val
end

dx = 0.001
xx = collect(0:dx:L)
colors = ("red", "green", "blue", "orange", "black", "purple", "yellow")
for nn=0:2
    lambda = (3807.0, 485.5, 12.36)
    lam = lambda[nn+1]
    uu = eigenfunc(xx, lam)
    c = Curve(xx, uu, color = colors[nn+1])
    setattr(c, label = string("n = ", 2 - nn))
    l = Legend(.1, (.2 - nn * 0.05), [c])
    add(p, c, l)
end
figure();
display(p); 
savefig(p, "Q1_EigenFunctions.png") 