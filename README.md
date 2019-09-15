This library should currently be considered defunct as code is being moved over from the Apex-RawCL repository. I'm doing a rewrite.

I'm mostly in this for learning how to use heterogenous computing and how to set up a library properly. As these things go, there is no guarantee that the library ever comes into a useable state. In fact, chances are pretty slim, though I am stubborn.

Apex
====
![Apex logo](apex.svg)

Apex is a geometric operations library aimed at heterogenous computing. It leverages different computational resources on your system (CPUs, GPUs and accelerators) to improve performance of your applications.

Requirements
----
Apex is a header-only library, meaning that there are no binaries to distribute. To use Apex, only the files in the `include` folder need to be included into your project. However some dependencies are required as well:
* A C++14-compliant compiler. Apex is tested using GCC and MinGW-w64.
* Boost.Compute is used to interact with OpenCL. This is also a header-only library.
* * Boost.Compute requires the headers of Boost itself.
* An installation of OpenCL should be present on the user's computer to leverage the OpenCL capabilities. If this is not present, Apex will fall back to host-only code at the cost of performance.

To build the tests, some additional dependencies are required:
* GTest, a C++ unit testing suite.

Usage
----
Please see the docs folder for usage examples and comprehensive documentation.