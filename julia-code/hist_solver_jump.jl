import Pkg
# Pkg.add("Ipopt")
# Pkg.add("CSV")
# Pkg.add("JuMP")
# Pkg.add("Juniper")
using CSV
using JuMP
using Juniper
using Ipopt

function read_inputs()
    target_hist = zeros(Int64, 0)
    depth = 0

    for row in CSV.File("../inData", header=false)
        for (idx, x) in enumerate(row)
            if idx == 1
                depth = x
            else
                append!(target_hist, x)
            end
        end
    end

    println(target_hist)
    println(depth)

    return (target_hist, depth)
end

function write_outputs(status, x, A)
    array = vcat(vec(x), vec(A))
    pushfirst!(array, status)
    array = round.(Int, array)

    open("../outData", "w") do file
        println(file, join(array, ","))
    end
end

function pow(M, p)
    if p == 1
        return M
    end

    half = pow(M, div(p, 2))
    if p % 2 == 0
        return half * half
    else
        return half * half * M
    end
end

(target_hist, D) = read_inputs()
println(target_hist, D)

N = length(target_hist)
L = sum(target_hist)
T = reshape(target_hist, 1, N)

nl_solver = optimizer_with_attributes(Ipopt.Optimizer, "print_level"=>0)
optimizer = optimizer_with_attributes(Juniper.Optimizer, "nl_solver"=>nl_solver, "print_level"=>0)
model = Model(optimizer)

@variable(model, A[1:N, 1:N] >= 0, Int)
@variable(model, x[1:1, 1:N] >= 0, Int)

for i in 1:N
    @constraint(model, sum(A[:,i]) >= 1)
    # @constraint(model, sum(A[:,i]) <= L)
    @constraint(model, sum(A[i,:]) >= 1)
    # @constraint(model, sum(A[:,i]) <= target_hist[i])
end

@constraint(model, x * pow(A, D) == T)
@objective(model, Min, sum(x[1,:]) + sum(A[:,:]))
# @objective(model, Min, (sum(x[1,:]) + sum(A[:,:]) - 6)^2)

# @variable(model, d[1:1,1:N], Int)
# @constraint(model, d == x * pow(A, D) - T)
# @objective(model, Min, sum(i^2 for i in d[1,:]))

# print(model)
optimize!(model)

if is_solved_and_feasible(model)
    write_outputs(1, value.(x), value.(A))
else
    write_outputs(0, [], [])
end