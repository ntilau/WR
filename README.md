# Heavy merging of all weighted residuals codes ongoing...
Ubuntu 24.04 gcc compilers toolset porting...

# wxFES
A wxWidgets + VTK experiment extending FES

# How-to
To get started, clone the repository. Then run Maker.bat in windows terminal and type:
- 'mingw32-make' to start compilation; 
- 'mingw32-make test' for testing 

# Release
- Download [x86_64-w64-mingw32](https://github.com/ntilau/wxFES/raw/master/bin/wxfes.exe) build

# Screenshots
![Koshiba](doc/koshiba.jpg)


# Summary
Finite Element code related to PhD reasearch activities

# Releases
- Download [x86_64-linux-gnu](https://github.com/ntilau/FES2/raw/master/bin/x86_64-linux-gnu/fes) build
- Download [x86_64-w64-mingw32](https://github.com/ntilau/FES2/raw/master/bin/x86_64-w64-mingw32/fes.exe) build


## Compute Weighted Residuals a.k.a. CoWR
Generalized version of FES to extend to any Weighted Residuals Methods framework
The goal is to have an as modular as possible implementation minimizing the need
for external libraries.
- Preprocessing and Postprocessing with VTK file formats
  + Mesh manipulations with TRIANGLE & TETGEN
- Wave Equation solvers in both FE or MOM formulations
  + BLAS, ARPACK & MUMPS solvers to be standardized on
- Low frequency stabilization to be addressed

# Revision history
To-do: 
- Shared libraries conversion
- Pre preprocessing information in modular fashion
- Modular post processing

# Releases
Download [x86_64-linux-gnu](https://github.com/ntilau/CoWR/raw/master/bin/x86_64-linux-gnu/core) build or type in your terminal:
```shell
wget https://github.com/ntilau/CoWR/raw/master/bin/x86_64-linux-gnu/core
```

# References
Zhizhang Chen and Michel Ney, [The Method of Weighted Residuals: A General Approach to Deriving Time- and Frequency-Domain Numerical Methods](https://www.cs.buap.mx/~sandoval/MetodosNumericos/MetNumCEROS/MarcoAntonioRamires.pdf), IEEE Antennas and Propagation Magazine, Vol. 51, No.1, February 2009
