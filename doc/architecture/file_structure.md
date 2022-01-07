File structure
====
In this document we review the layout of the files in this library, and why they are grouped in the directories that they are in.

Main directory
----
In the main directory, we find the standard layout for Unix libraries and a few extra directories. These directories include:

* `include`: This contains the implementation of the library. The contents of this directory should be included for the compiler to use this library.
* `test`: This contains the tests for this library. They are not necessary to use the library.
* `doc`: Containing documentation, including this file.
* `cmake`: Containing CMake helper scripts and modules.
* `benchmarking`: A project that can be used to benchmark this library.

CMake file structure
----
To allow directories to be moved more easily, the `CMakeLists.txt` file in each directory should only concern files and subdirectories in the directory itself, not in subdirectories. For instance, the documentation folder should contain a CMake script that collects the documentation. The test folder should contain a CMake script that compiles the tests.

Within the include directory
----
The source code of the library is inside the `include` directory. Inside the include directory, there are two subdirectories:

* `operations`: This contains all of the [operations](operations.md) that can be performed on geometric objects. They are grouped together because they all have generally the same structure. They define variations of geometric algorithms.
* `detail`: This contains helper classes, functions and other things that should not be directly accessed by the user. The user doesn't need to know about the files in this folder. Everything in this folder should be defined in the `::apex::detail` namespace.

The rest of the files in the `include` directory are data structures that the user does need to access. In many cases, these data structures are the main way in which the user would use the library. For this reason, they can be placed directly in the `include` directory.

The `test` directory should follow the same directory structure as the `include` directory, but adds more files for its test cases and helpers.