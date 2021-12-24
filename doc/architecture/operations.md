Operations
====
This library consists of a number of data structures, and a number of operations that work on those data structures. This document describes where the operations are defined and why they are structured the way they are.

Free Functions
----
Each operation is defined as a number of free functions (not part of any class). The operation has a number of versions and overloads which are located in the same file, in order to keep the scope of a file small and make the code easier to follow. They may also re-use some helper functions, although they would need to be placed in the detail sub-namespace.

For ease of use, if there is an operation that works on a data structure, the data structure should also have a member that calls that operation on itself. This makes these operations easier to discover, especially with tools like an IDE which list the operations that can be performed on an object.

This structure of free functions is chosen for two reasons:

* It is simple and extensible. There are no problems with template classes or inheritance.
* The individual versions and overloads can separately be called from a test without needing to construct anything unnecessary.

Versions
----
Each operation may have a number of different versions. For instance, you may have a very efficient version that has a lot of overhead, and a less scalable version but with little overhead. Or there may be a version that runs on the CPU, one that runs multi-threaded on the CPU, and one that runs on the GPU. These versions are named the same as the original operation, but with an underscore and a distinguishing name at the end. The real function then decides which version to delegate to based on metadata of the input and the environment.

For example, a `sort` operation may have the following functions:

```
void sort(std::vector<int>& data); //The public version the user should call.
namespace detail {
	void sort_selection(std::vector<int>& data); //Low overhead.
	void sort_merge(std::vector<int>& data); //Scales well.
	void sort_merge_gpu(std::vector<int>& data); //Scales very well, on GPU.
}
```

In this case, there are really three different versions: A selection sort, a merge sort and a merge sort that works on the GPU. The public `sort` function chooses the version to use based on the metadata of the input, such as:

```
void sort(std::vector<int>& data) {
	if(data.size() < 100) detail::sort_selection(data);
	else if(data.size() < 5000) detail::sort_merge(data);
	else detail::sort_merge_gpu(data);
}
```

Overloads
----
Aside from having different versions, an operation may also have different overloads. You could compute the surface area of any 2-dimensional shape, so there could be an overload for the area of a polygon as well as the area of a circle.

Often an operation will also have an overload for batches of objects. These operations can then scale better, by delegating the work to the GPU or simply by improving the cache locality. The batch overloads should return a batch of answers. Note that there is a difference there between a batch and a vector. For instance, you may have an operation that checks for collisions with a batch of lines and an operation that checks for collisions with a vector of lines. The operation that takes a batch of lines should also give a batch of collision results, while the operation that takes a vector of lines might just give a single boolean as answer of whether any of the lines collide.