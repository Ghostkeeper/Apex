Apex
====
![Apex logo](doc/res/logo.svg)

Apex is a geometric operations library aimed at heterogenous computing. It leverages different computational resources on your system (CPUs, GPUs and accelerators) to improve performance of your applications.

Status
----
This library is not in a useable state.

I am properly overengineering this. I'm giving it a 95% chance that it will never really come into a useable state. That is not the aim of this project, either. The primary goals at the moment are:
* To practice setting up projects properly, using proper data structures, design patterns, tests and documentation.
* To have fun implementing interesting algorithms and inventing new ones.
* To specialise myself in high-performance computations, with a mix of practicality and science.

The goal of eventually maybe having a usable library is only secondary, but it serves as a motivator to keep working on the project.

Requirements
----
Apex is a header-only library, meaning that there are no binaries to distribute. To use Apex, only the files in the `include` folder need to be included into your project. However some dependencies are required as well:
* A C++20-compliant compiler. Apex is tested using Clang 12 and GCC 10.
  * GCC and MinGW don't sufficiently support OpenMP teams at the time of this writing to use GPUs or accelerators. I'm betting on that they will by the time this library is in a useable state. Currently these are limited to use only the CPU versions of algorithms.
  * To use GPU acceleration, you will need to build a compiler with bindings to the drivers of the graphics card you will be compiling for. This involves compiling your own compiler, such as described [here](https://hpc-wiki.info/hpc/Building_LLVM/Clang_with_OpenMP_Offloading_to_NVIDIA_GPUs). It is sadly not currently possible to compile one binary that works on all common graphics cards. You would have to load libraries dynamically based on the graphics card in use.
* OpenMP is used to distribute computation.

To build the tests and documentation, some additional dependencies are required:
* GTest, a C++ unit testing suite.
* Doxygen, a system to compile documentation into a human-readable form.

Usage
----
Please see the docs folder for usage examples and comprehensive documentation.