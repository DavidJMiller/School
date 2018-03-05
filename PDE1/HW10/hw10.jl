using Plots
using PyPlot
using Plotly

    graphPoints = 1001
    xx = linspace(0, 1, graphPoints)      #linspace creates vector of elements linspace(start, stop, dx)
    yy = linspace(0, 1, graphPoints)                                    # linspace(0,1,3) = {0, 0.5, 1}

    #ygrid = repmat(yy, graphPoints, 1)     #repmat(A,n,m) this makes a 2D array. Source array is repeated 
    #xgrid = repmat(xx, 1, graphPoints)         #A is repeated by n times in first dimension (rows), then
                                                #and then m times in second dimension (columns)

    N = 10;
    u = zeros(graphPoints,graphPoints)
    for tt in 0:3
        u = zeros(graphPoints,graphPoints)
        for x in 1:graphPoints
            for y in 1:graphPoints
                sum = 0.0
                for n in 1:N
                    sum = sum + 16/((2*n-1)^3*pi^3) * sinpi((2*n-1)*yy[y]) * exp(-5*((2*n-1)^2)*pi^2*(tt/100))
                end
                u[y,x] = sum
            end
        end
   surf(xx,yy,u', cmap=ColorMap("jet"), alpha=0.7, linewidth=0, antialiased=false, shade=false)
            xlabel("X")
            ylabel("Y")
            zlabel("u(x,y)")

    end