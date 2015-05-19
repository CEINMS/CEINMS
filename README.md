CEINMS
======
The Calibrated EMG-Informed NMS Modelling Toolbox (CEINMS) permits to estimate muscle forces inside a human body during static and dynamic tasks. CEINMS implments all the transformations that take place from the onset of muscle excitation to the generation of muscle forces and resulting joint moments.

CEINMS allows automatically identification of a number of parameters that determine the way musculotendon units activate and contract, which vary non-linearly across individuals. This is done via an optimization-based calibration procedure that adjusts the internal parameters to best reflect the anatomy and physiology of an individual.

Read more about CEINMS at the [CEINMS project page](https://simtk.org/home/ceinms)

Quick Start
-----------
[Test data and examples](https://simtk.org/frs/download.php?file_id=4495)

Dependencies
------------
CEINMS depends on the following 
* cross-platform building: [CMake](http://www.cmake.org/) 2.8.8 or later
* compiler: [Visual Studio](http://www.visualstudio.com) 2013 or later (Windows only), [gcc](http://gcc.gnu.org/) 4.8.1 or later (typically on Linux), or [Clang](http://clang.llvm.org/) 3.4 or later (typically on Mac, possibly through Xcode)
* file managment: [Boost libraries](http://www.boost.org/), or [Boost libraries binaries](http://sourceforge.net/projects/boost/files/boost-binaries) (Windows only)
* XML parsing: [CodeSynthesis XSD 3.3](https://www.codesynthesis.com)


Install
-------
[CEINMS 0.9 Win32 installer](https://simtk.org/frs/download.php?file_id=4494) (does not require any external library).

Acknowledgments
---------------
* Australian National Health and Medical Research Council (628850)
* Royal Society of NZ Marsden Fund (12-UOA-1221)
* US National Institutes of Health Grant (R01EB009351)
* Commission of the European Union Grant (IFP7-ICT-2013-10-611695)
* European Research Council (ERC) Advanced Grant DEMOVE (267888)

<!---
Useful links:
[OpenSim 3.2 VC12P x86](https://drive.google.com/file/d/0BzMAK5L0QV2PSEkxSUVxSXd3NzA/edit?usp=sharing)

When compiling with OpenSim3.2 in Windows, the following errors may occur:
```
Error	1	error C3646: '__attribute__' : unknown override specifier	c:\opensim3.2\sdk\include\opensim\common\DebugUtilities.h	42	1	Calibration
Error	2	error C2065: 'noreturn' : undeclared identifier	c:\opensim3.2\sdk\include\opensim\common\DebugUtilities.h	42	1	Calibration
Error	3	error C2072: 'OpenSim::DebugUtilities::Fatal_Error' : initialization of a function	c:\opensim3.2\sdk\include\opensim\common\DebugUtilities.h	42	1	Calibration
Error	4	error C2146: syntax error : missing ';' before identifier 'WINAPI'	c:\opensim3.2\sdk\include\opensim\common\LoadOpenSimLibrary.h	43	1	Calibration
Error	5	error C4430: missing type specifier - int assumed. Note: C++ does not support default-int	c:\opensim3.2\sdk\include\opensim\common\LoadOpenSimLibrary.h	43	1	Calibration
Error	6	error C2146: syntax error : missing ';' before identifier 'LoadOpenSimLibrary'	c:\opensim3.2\sdk\include\opensim\common\LoadOpenSimLibrary.h	43	1	Calibration
```

If so, be sure that _WIN32 is used instead of WIN32 in the define directives in OpenSim include files DebugUtilities.h, LoadOpenSimLibrary.h, osimCommonDLL.h (i.e. look where the errors are and change WIN32 in _WIN32 everywhere) 
-->
