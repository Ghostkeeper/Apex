Strategies
====
Algorithms in this library often have multiple implementations. The library will choose which algorithm is most likely the most effective for the current situation.

Some algorithms scale well with large amounts of data, and some scale poorly. However some algorithms also have a lot of overhead, which is not effective for small amounts of data. As such, it is appropriate to choose an algorithm with little overhead when presented with small amounts of data. How well it scales is less important then. On the other hand, when presented with large amounts of data, it is better to choose an algorithm that scales well.

This is especially important for the objective to implement high-performance computing. This library is intended to handle a wide range of input sizes well. An implementation on the GPU is not always a good solution because it has a lot of overhead. However the implementation on the GPU most likely scales better due to the available processing power.

If no GPU hardware is available on the user's computer, the strategy pattern in this library also easily allows falling back on CPU implementations.

Choosing a strategy
----
The best strategy is currently chosen based on performance measurements on a single computer. This is not intended to be a permanent solution. It does not translate well to other computers, and it's unclear how to add better measurements.

In the future, we may use a strategy pattern library. [Nadir](https://github.com/Ghostkeeper/Nadir) was being developed for this purpose. Ideally a strategy pattern implementation allows for measurements to be statically stored in the binary, and it needs to be fast (on the order of a couple of integer comparisons). It may use the size of the input data as parameter, the available hardware on the computer, and even the current availability of compute devices (for instance to cause the CPU to take on a task that is better suited for a GPU if the GPU is busy).

Available strategies
----
The aim is for every algorithm to have at least two strategies available: One that runs on the CPU and one that runs on the GPU. This is important, because not all computers have a GPU and fewer still have a GPU that is properly supported by the bindings in compilers. This allows users without supported GPUs to fall back on the CPU implementation. On the other hand, the GPU implementation should also be present, even if it is hard to parallelise, because it allows the data to stay on the GPU, preventing the need to transport data back and forth between the CPU and GPU. While the strategy pattern is currently not clever enough to make these decisions, it could become so in the future. In many cases, a single-threaded and a multi-threaded implementation will be available for the CPU too.

If an algorithm is better in every case than another algorithm, the weaker algorithm should be removed to reduce maintenance.