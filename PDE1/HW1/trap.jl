# trap.jl
# Numerically estimates integral via Trapezoidal quadrature
using Winston

hold(false)

N = 10
steps = zeros(N+1)
errors = zeros(N+1)

# Trapezoidal quadrature
function trap(ff::Vector{Float64}, dx::Float64)
	FF = ff[1:size(ff)[1]-1,1] + ff[2:size(ff)[1],1];
	return ( dx * sum(FF) / 2 )
end

# Trapezoidal quadrature with result and convergence test
function evaluate(f, L::Float64, R::Float64, exact::Float64)
	for i = 0:N
		dx = 2.0^(-i)
		xx = collect(L:dx:R)
		ff = f(xx)
		approx = trap(ff, dx)
		error = abs(exact - approx)
		steps[i+1,1] = dx
		errors[i+1,1] = error
	end
	figure()
	# Graph loglog of error vs spatial step size
	plot(loglog(steps, errors))
	hold(true)
	xx = collect(0:.001:1)
	# Graph line with slope 2
	plot(loglog(xx, 4*xx.^2, "r--", label = "Slope 2 Line"))
	# Pick two points to determine slope of loglog line
	order = log(errors[8]/errors[5]) / log(steps[8]/steps[5])
	display(string("Order of convergence is ", order))
	xlabel("log(dx)"); ylabel("log(error)")
	title("Convergence of Trapezoidal Quadrature")	
end

# Test Case 1
#=
exact = -0.958924274663138
evaluate(a -> cos(a) , 0.0, 5.0, exact)
=#

# Test Case 2
#=
exact = 0.840982822189277
evaluate(a -> sech(a) - sin(a) , 0.0, 5.0, exact)
=#

# Test Case 3
#=
exact = 29.16666666666666
evaluate(a -> a.^2 + 5.*a + 1 , -2.0, 3.0, exact)
=#

# Test Case 4
#=
exact = -8.75
evaluate(a -> -a.^3 + 3.*a , -2.0, 3.0, exact)
=#

# Test Case 5
#=
exact = 10.1344234012711
evaluate(a -> exp(a) + 2 , -7.0, -2.0, exact)
=#

# Test Case 6
#=
exact = 6.3231
evaluate(a -> exp(cos(a)) - .1*cos(a) , -7.0, -2.0, exact)
=#

# Test Case 7

exact = 37.9747275562544
evaluate(a -> cos(a).*exp(a) + a.^2 - a.*sin(a), -5.0, 5.0, exact)


# Test Case 8
#=
exact = 4.418195428799727
evaluate(a -> cos(exp(a)) - (a.^2).*sin(a) - a, -5.0, 5.0, exact)
=#