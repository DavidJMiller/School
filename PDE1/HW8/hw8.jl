#Dirichlet Kernel code
using Winston
using LaTeXStrings

p = FramedPlot(
        title = "Dirichlet Kernel",
        xlabel = L"\theta",
        ylabel = L"K_n(\theta)")

k = 0.5
c = 100.0
L = 5.0
N = 10
function kernel(xx::Vector{Float64}, nn::Float64)
    val = 0
    for n = 1:N
        val = sin((nn+1/2).*xx)./sin(xx./2)
    end 
    return val
end

dx = 0.001
xx = collect(-pi:dx:pi)
colors = ("red", "green", "blue", "orange", "black", "purple", "yellow")
for nn=1:5
    n = 5000.0*nn
    uu = kernel(xx, n)
    c = Curve(xx, uu, color = colors[nn+1])
    setattr(c, label = string("n = ", n))
    l = Legend(.7, (.975 - nn * 0.05), [c])
    add(p, c, l)
end
figure();
display(p); 
savefig(p, "Q5_1.png") 