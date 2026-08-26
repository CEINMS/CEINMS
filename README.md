CEINMS
======
[![Build and Test](https://github.com/CEINMS/CEINMS/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/CEINMS/CEINMS/actions/workflows/build.yml)

The Calibrated EMG-Informed NMS Modelling Toolbox (CEINMS) permits to estimate muscle forces inside a human body during static and dynamic tasks. CEINMS implements all the transformations that take place from the onset of muscle excitation to the generation of muscle forces and resulting joint moments.

CEINMS allows automatically identification of a number of parameters that determine the way musculotendon units activate and contract, which vary non-linearly across individuals. This is done via an optimization-based calibration procedure that adjusts the internal parameters to best reflect the anatomy and physiology of an individual.

Read more about CEINMS at the [CEINMS project page](https://simtk.org/projects/ceinms)


Installation
------------
Prebuilt Windows installers and the user guide are available from the Downloads section of the [CEINMS project page on SimTK](https://simtk.org/projects/ceinms).

The installer places the OpenSim libraries that CEINMS was built against next to the CEINMS executables, so no separate OpenSim installation is needed to run it. You will still need OpenSim to prepare CEINMS input data, such as inverse kinematics, inverse dynamics and muscle analysis results.

CEINMS is run from a terminal, so it is usually convenient to add the `bin` directory of the installation (by default `C:\Program Files\CEINMS 1.0\bin`) to your `PATH`. On Windows this is under Settings > System > About > Advanced system settings > Environment Variables. The installer does not offer to do this for you, because Windows PATH values are frequently longer than the installer framework can safely rewrite.

To build CEINMS yourself instead, for example on Linux or against a different OpenSim version, see [Building from source](#building-from-source) below.


Dependencies
------------
* Cross-platform building: [CMake](http://www.cmake.org/) 3.16 or later
* Compiler: [Visual Studio](http://www.visualstudio.com) 2019 or later (Windows only), [gcc](http://gcc.gnu.org/) 8 or later (typically on Linux), or [Clang](http://clang.llvm.org/) 5 or later (typically on Mac, possibly through Xcode)
* XML parsing: [CodeSynthesis XSD](https://www.codesynthesis.com/products/xsd/) 4.0
* Musculoskeletal modelling: [OpenSim](https://simtk.org/projects/opensim/) 4.0 or later
    * **CEINMS must be built against the same OpenSim version it is run with.** OpenSim ships identically named libraries (`osimSimulation.dll`, `osimCommon.dll`, ...) in every release, with no version information recorded in them, so on Windows the loader simply uses whichever copy appears first on your PATH. If that copy is not the one CEINMS was compiled against, CEINMS exits immediately on startup with no error message (exit code `0xC0000139`, `STATUS_ENTRYPOINT_NOT_FOUND`) — the failure happens while the process is loading, before any CEINMS code runs, so it cannot report anything more helpful. This is easy to hit if several OpenSim versions are installed.
    * Building the `install` target, or producing a package with CPack, copies the OpenSim libraries in beside the CEINMS executables. Windows searches an executable's own directory before PATH, so the installed copy always finds the right libraries regardless of what else is installed. **Prefer running the installed CEINMS over the one in the build tree**, which has no such protection. A consequence is that an installed CEINMS is tied to the OpenSim it was built against; to change OpenSim version, reconfigure and rebuild.
    * Known to work with OpenSim 4.0-4.5. OpenSim 4.6 moved force computation onto a new `ForceProducer` base class and has not been validated with CEINMS yet.
* Signal filtering: [Filter](https://github.com/RealTimeBiomechanics/Filter)
* Thread synchronisation: [Concurrency](https://github.com/RealTimeBiomechanics/Concurrency)


Building from source
-------
### Windows:
1. Get the dependencies
    * [CMake](https://cmake.org/download/): Follow the instruction on the website
    * [Visual Studio](http://www.visualstudio.com): Follow the instruction on the website
        * The `-G` generator name in the commands below must match the version you installed (`Visual Studio 17 2022`, `Visual Studio 16 2019`, and so on); `cmake --help` lists the generators available on your machine
    * [CodeSynthesis XSD](https://www.codesynthesis.com/products/xsd/download.xhtml): Download and run the .msi package which includes the precompiled Xerces-C++ libraries that are needed
    * [OpenSim](https://simtk.org/projects/opensim/): The instructions for building OpenSim from source are provided in the [README](https://github.com/opensim-org/opensim-core) on GitHub
        * If you have multiple versions of OpenSim installed, ensure that your PATH is pointing to the correct version directory
    * [Filter](https://github.com/RealTimeBiomechanics/Filter):
        Download the code and from the directory run:
        ```bash
        mkdir build
        cd build
        cmake -G "Visual Studio 17 2022" ..
        cmake --build . --config Release --target install
        ```
        After installation, create a Windows environment variable named `Filter_DIR` which points at the installation directory.
    * [Concurrency](https://github.com/RealTimeBiomechanics/Concurrency):
        Download the code from the develop (default) branch and from the directory run:
        ```bash
        mkdir build
        cd build
        cmake -G "Visual Studio 17 2022" ..
        cmake --build . --config Release --target install
        ```
        After installation, create a Windows environment variable named `Concurrency_DIR` which points at the installation directory. 
2. Download the CEINMS code
3. Configure and generate the project files using CMake
    * Open the CMake GUI
    * In the field labelled "Where is the source code:" enter the directory where the CEINMS source code is located
    * In the field labelled "Where to build the binaries:" enter a directory where you want the code to be built
    * Click Configure, specify the generator matching your Visual Studio version (e.g. Visual Studio 17 2022) and click finish
    * After CMake has finished configuring you can set the install directory in the "CMAKE_INSTALL_PREFIX" field and check that the dependencies (Concurrency, Filter, OpenSim, Simbody, Xerces and XSD) were successfully located. If a dependency is not found automatically then enter the correct directory where it was installed into the corresponding field
        * You can also choose whether CEINMS outputs information to the terminal during execution using the CEINMS_TERMINAL_LOGGING variable (NOTE: Enabling this can increase execution time and is therefore off by default). 
    * Click Configure again, all fields should change from red back to white, then click generate
    * Click on Open Project to open the generated project in Visual Studio
4. In Visual Studio set the Solution Configuration from the dropdown box at the top (e.g. RelWithDebInfo) then build ALL_BUILD
    * You can run tests to ensure CEINMS is working as intended by building RUN_TESTS
    * Build the INSTALL target to install the CEINMS files to the directory that you specified, and you can add the CEINMS install directory to your path to run the CEINMS executables from any directory


### Linux:
1.  Get the dependencies
    * [CMake](https://cmake.org/download/): Follow instructions from website or install from terminal:
        ```
        $ sudo apt-get install cmake-gui
        ```
    * [gcc](http://gcc.gnu.org/): Version 8 or later
    * [CodeSynthesis XSD](https://www.codesynthesis.com/products/xsd/download.xhtml): Download 64 bit .rpm file and extract it to obtain the precompiled files
        * You will also need to obtain [Xerces](https://xerces.apache.org/xerces-c/download.cgi) by downloading (e.g. xerces-c-3.1.1.tar.gz from https://archive.apache.org/dist/xerces/c/3/sources/), extracting and building following the provided instructions
    * [OpenSim](https://github.com/opensim-org/opensim-core): The instructions for building OpenSim from source are provided in the README on GitHub
    * [Filter](https://github.com/RealTimeBiomechanics/Filter):
        Download the code and from the directory run:
        ```bash
        mkdir build
        cd build
        cmake -DCMAKE_BUILD_TYPE=Release ..
        cmake --build . -j2
        make install
        ```
    * [Concurrency](https://github.com/RealTimeBiomechanics/Concurrency):
        Download the code from the develop (default) branch and from the directory run:
        ```bash
        mkdir build
        cd build
        cmake -DCMAKE_BUILD_TYPE=Release ..
        cmake --build . -j2
        make install
        ```
2. Download the CEINMS code
3. Configure and generate the project files using CMake
    * Method 1:
        * From the directory where you downloaded CEINMS run: 
            ```bash
            mkdir build
            cd build
            cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX="~/CEINMS/install" ..
            ```
            * Set BUILD_TYPE to your desired build type and INSTALL_PREFIX to the desired install directory
            * This requires the dependencies to be easily located (e.g. adding the locations to path)
    * Method 2:
        * Open the CMake GUI
        * In the field labelled "Where is the source code:" enter the directory where the CEINMS source code is located
        * In the field labelled "Where to build the binaries:" enter a directory where you want the code to be built
        * Click Configure, specify Unix Makefiles as the project generator and click finish
        * After CMake has finished configuring you can set the install directory in the "CMAKE_INSTALL_PREFIX" field, the build type in the "CMAKE_BUILD_TYPE" field and check that the dependencies were successfully located. If a dependency is not found automatically then enter the correct directory where it was installed into the corresponding field
            * You can also choose whether CEINMS outputs information to the terminal during execution using the CEINMS_TERMINAL_LOGGING variable (NOTE: Enabling this can increase execution time and is therefore off by default). 
        * Click Configure again, all fields should change from red back to white, then click generate
4. Open a terminal in the build directory and run the following commands to build and install to the specified location:
    ```bash
    make -j8
    make install -j8
    ```
    * You can run tests to ensure CEINMS is working by running the following command before installing:
        ```bash
        ctest -j8
        ```
    * If you have an error finding a library when running CEINMS from the install location, ensure the dependency library was correctly added to the path
    * You can add the chosen install directory to the path if you wish to run CEINMS from the terminal outside of the install directory
        e.g.
        ```bash
        $ echo 'export PATH=~CEINMS/install/bin:$PATH' >> ~/.bashrc
        ```


Notes
---------------
* Inverse kinematics MOT files generated using an OpenSim version below 4.0 have some coordinates incorrectly labelled as being Rotational rather than Coupled. Therefore, it is recommended to process your CEINMS input data using OpenSim 4.0+ to avoid potential issues. Alternatively, newer versions of OpenSim provide a function (updatePre40KinematicsFilesFor40MotionType()) that you can use to update inverse kinmatics MOT files from pre v4.0 to v4.0+, however using this function on v4.0+ files will cause your data to be altered incorrectly.


Acknowledgments
---------------
* Australian National Health and Medical Research Council (628850)
* Royal Society of NZ Marsden Fund (12-UOA-1221)
* US National Institutes of Health Grant (R01EB009351)
* Commission of the European Union Grant (IFP7-ICT-2013-10-611695)
* European Research Council (ERC) Advanced Grant DEMOVE (267888)
