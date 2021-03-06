# Changelog

## Release 2.0.0

- Add support for internal coordinates
- Add interface to Gaussian
- Improve ORCA interface (and make compatible with ORCA 4.2.0)
- Add BFGS optimizer and G-DIIS convergence accelerator
- Improve Bofill transition state search algorithm
- Various bugfixes and improvements

## Release 1.0.1

Hotfix to allow compilation on OSX using Clang.

## Release 1.0.0

Initial release with all necessary functionality to support Sparrow and ReaDuct.
Among other things, this includes:

- Analytic evaluation of gadients
- Calculation of bond orders
- Interface to the ORCA quantum chemistry program
- Numerical Hessian calculator
- Optimizers to find minima and transition states on the PES
- Python bindings
- SCF algorithm (including convergence accelerators such as DIIS)
