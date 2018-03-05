#Dirichlet Kernel code
using Winston
using LaTeXStrings

p = FramedPlot(
        title = "Beam Equation Evolution",
        xlabel = L"x",
        ylabel = L"u(x,t)")

K = 1
L = 1.0
function eigenfunc(xx::Vector{Float64}, tt::Float64)
    lambda = (3807.0, 485.5, 12.36)
    val = 0
    val = (cosh.((lambda[1])^(1/4).*xx) - cos.((lambda[1])^(1/4).*xx) - ((cosh((lambda[1])^(1/4)) + cos((lambda[1])^(1/4))) / (sinh((lambda[1])^(1/4)) + sin((lambda[1])^(1/4)))) * (sinh.((lambda[1])^(1/4).*xx) - sin.((lambda[1])^(1/4).*xx))).*0.25*cos(sqrt(lambda[1])*tt)+(cosh.((lambda[2])^(1/4).*xx) - cos.((lambda[2])^(1/4).*xx) -((cosh((lambda[2])^(1/4)) + cos((lambda[2])^(1/4))) / (sinh((lambda[2])^(1/4)) + sin((lambda[2])^(1/4)))) * (sinh.((lambda[2])^(1/4).*xx) - sin.((lambda[2])^(1/4).*xx))).*0.5*cos(sqrt(lambda[2])*tt)+(cosh.((lambda[3])^(1/4).*xx) - cos.((lambda[3])^(1/4).*xx) -((cosh((lambda[3])^(1/4)) + cos((lambda[3])^(1/4))) / (sinh((lambda[3])^(1/4)) + sin((lambda[3])^(1/4)))) * (sinh.((lambda[3])^(1/4).*xx) - sin.((lambda[3])^(1/4).*xx))).*cos(sqrt(lambda[3])*tt)
    return val
end

dx = 0.001
xx = collect(0:dx:L)
colors = ("red", "green", "blue", "orange", "black", "purple", "yellow")
for nn=0:5
    tt = nn / 100.0
    uu = eigenfunc(xx, tt)
    c = Curve(xx, uu, color = colors[nn+1])
    #c = Curve(xx, uu)
    setattr(c, label = string("t = ", tt))
    l = Legend(.75, (.3 - nn * 0.05), [c])
    add(p, c, l)
    #add(p, c)
end
figure();
display(p); 
savefig(p, "Q1_BeamEq.png") 