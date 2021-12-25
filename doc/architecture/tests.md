Tests
====
The tests are structured around the files. The file structure of the tests folder should closely mimic the file structure of the implementation, with a few extra bits as helper classes and a few extra tests for integration.

Helpers
----
The tests have a few helper functions that can load data, automate certain common tests and general helper functions. These helper functions are compiled first into a library and then linked to each of the testing executables. This speeds up the compilation of these tests significantly, since it's not needed to compile each helper class separately for every executable then.

Test Cases
----
Test cases can be programmatically defined, but they can also be loaded from a file. Loading a test case from a file is preferred, because the file can be a file type that is easy to visualise for a maintainer. For instance, 2-dimensional shapes can be stored as SVG files so that an image viewer can easily visualise them.

To make running the tests easier, these test cases are compiled into the executable as a string. This does require some preparation in CMake, to turn the file into something that can be included in the test code.

Not all test cases are practical to store in an external file. For instance, a test case might have a large amount of data which would require parsing megabytes of files. Or it could be that there is no easily visualised file format available for the type of data that is tested, or that the available file formats are difficult to parse. The parsers need to be kept simple to reduce the chance of new bugs arising in the tests themselves. For all other cases, easily visualised external files should be the preferred way to store a test case.

Runners and Devices
----
The implementation should not depend on the device that it runs on. However due to the nature of the HPC industry, this is not always the case. It would be ideal to test this library on as many devices as possible, with various vendors of graphical devices in particular.

Currently this is not achievable. As of this writing, the library is tested on:
* A Github runner, which has no graphical accelerator.
* The desktop computer of Ghostkeeper, which has an NVidia GPU.