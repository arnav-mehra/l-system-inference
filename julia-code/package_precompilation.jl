import Pkg
Pkg.add("PackageCompiler")

using PackageCompiler, Libdl
PackageCompiler.create_sysimage(
    ["CSV", "JuMP", "Juniper", "Ipopt"],
    sysimage_path = "customimage." * Libdl.dlext,
    precompile_execution_file = "hist_solver_jump.jl",
)