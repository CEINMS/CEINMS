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
* Cross-platform building: [CMake](http://www.cmake.org/) 2.8.12 or later
* Compiler: [Visual Studio](http://www.visualstudio.com) 2016 or later (Windows only), [gcc](http://gcc.gnu.org/) 8 or later (typically on Linux), or [Clang](http://clang.llvm.org/) 5 or later (typically on Mac, possibly through Xcode)
* XML parsing: [CodeSynthesis XSD](https://www.codesynthesis.com/products/xsd/) 4.0 or later
* Musculoskeletal modelling: [OpenSim](https://github.com/opensim-org/opensim-core) 4.0 or later
* Signal filtering: [Filter](https://github.com/RealTimeBiomechanics/Filter)
* Thread synchronisation: [Concurrency](https://github.com/RealTimeBiomechanics/Concurrency/tree/master)


Building from source
-------
### Windows:
1. Get the dependencies
    * You can obtain them individually or run the included "install_dependencies" script on Windows
2. Configure and generate the project files using CMake
    * Ensure CMake found all dependencies (e.g. if xerces_dll is not found it should still build but could have install error)
3. Open project solution in Visual Studio, build and install
4. Add install location to PATH

Acknowledgments
---------------
* Australian National Health and Medical Research Council (628850)
* Royal Society of NZ Marsden Fund (12-UOA-1221)
* US National Institutes of Health Grant (R01EB009351)
* Commission of the European Union Grant (IFP7-ICT-2013-10-611695)
* European Research Council (ERC) Advanced Grant DEMOVE (267888)
