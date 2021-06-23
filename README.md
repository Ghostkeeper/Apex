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
* A C++17-compliant compiler. Apex is tested using GCC and MinGW-w64.
  * When using GCC, use at GCC-8 or later. In GCC-7.4 there are still bugs that prevent this library from compiling.
* OpenMP is used to distribute computation.

To build the tests, some additional dependencies are required:
* GTest, a C++ unit testing suite.

Usage
----
Please see the docs folder for usage examples and comprehensive documentation.