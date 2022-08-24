CEINMS
======
The Calibrated EMG-Informed NMS Modelling Toolbox (CEINMS) permits to estimate muscle forces inside a human body during static and dynamic tasks. CEINMS implements all the transformations that take place from the onset of muscle excitation to the generation of muscle forces and resulting joint moments.

CEINMS allows automatically identification of a number of parameters that determine the way musculotendon units activate and contract, which vary non-linearly across individuals. This is done via an optimization-based calibration procedure that adjusts the internal parameters to best reflect the anatomy and physiology of an individual.

Read more about CEINMS at the [CEINMS project page](https://simtk.org/home/ceinms)


Dependencies
------------
* Cross-platform building: [CMake](http://www.cmake.org/) 3.8 or later
* Compiler: [Visual Studio](http://www.visualstudio.com) 2019 or later (Windows only), [gcc](http://gcc.gnu.org/) 8 or later (typically on Linux), or [Clang](http://clang.llvm.org/) 5 or later (typically on Mac, possibly through Xcode)
* XML parsing: [CodeSynthesis XSD](https://www.codesynthesis.com/products/xsd/) 4.0
* Musculoskeletal modelling: [OpenSim](https://github.com/opensim-org/opensim-core) 4.0 or later
* Signal filtering: [Filter](https://github.com/RealTimeBiomechanics/Filter)
* Thread synchronisation: [Concurrency](https://github.com/RealTimeBiomechanics/Concurrency)


Building from source
-------
### Windows:
1. Get the dependencies
    * Method 1: Obtain them individually
        * [CMake](https://cmake.org/download/): Follow the instruction on the website
        * [Visual Studio](http://www.visualstudio.com): Follow the instruction on the website
        * [CodeSynthesis XSD](https://www.codesynthesis.com/products/xsd/download.xhtml): Download and run the .msi package which includes the precompiled Xerces-C++ libraries that are needed
        * [OpenSim](https://github.com/opensim-org/opensim-core): The instructions for building OpenSim from source are provided in the README on GitHub
        * [Filter](https://github.com/RealTimeBiomechanics/Filter):
            Download the code and from the directory run:
            ```bash
            mkdir build
            cd build
            cmake -G "Visual Studio 16 2019" ..
            cmake --build . --config Release --target install
            ```
            After installation, create a Windows environment variable named `Filter_DIR` which points at the installation directory.
        * [Concurrency](https://github.com/RealTimeBiomechanics/Concurrency):
            Download the code from the develop (default) branch and from the directory run:
            ```bash
            mkdir build
            cd build
            cmake -G "Visual Studio 16 2019" ..
            cmake --build . --config Release --target install
            ```
            After installation, create a Windows environment variable named `Concurrency_DIR` which points at the installation directory. 
    * Method 2: Run the included "install_dependencies" script on Windows to automatically search for the dependencies and install them if not found 
2. Download the CEINMS code
3. Configure and generate the project files using CMake
    * Open the CMake GUI
    * In the field labelled "Where is the source code:" enter the directory where the CEINMS source code is located
    * In the field labelled "Where to build the binaries:" enter a directory where you want the code to be built
    * Click Configure, specify Visual Studio 16 2019 as the project generator and click finish
    * After CMake has finished configuring you can set the install directory in the "CMAKE_INSTALL_PREFIX" field and check that the dependencies (Concurrency, Filter, OpenSim, Simbody, Xerces and XSD) were successfully located. If a dependency is not found automatically then enter the correct directory where it was installed into the corresponding field
    * Click Configure again, all fields should change from red back to white, then click generate
    * Click on Open Project to open the generated project in Visual Studio
4. In Visual Studio set the Solution Configuration from the dropdown box at the top (e.g. RelWithDebInfo) then build ALL_BUILD
    * Once the build has finished you can build Install to install the CEINMS files to the directory that you specified


### Linux:
1.  Get the dependencies
    * [CMake](https://cmake.org/download/): Follow instructions from website or install from terminal:
        ```
        $ sudo apt-get install cmake-gui
        ```
    * [gcc](http://gcc.gnu.org/): Ensure version 8 or later
    * [CodeSynthesis XSD](https://www.codesynthesis.com/products/xsd/download.xhtml): Download 64 bot .rpm file and extract it to obtain the precompiled files
        * You will also need to obtain [Xerces](https://www.codesynthesis.com/products/xsd/download.xhtml) by downloading (e.g. xerces-c-3.1.1.tar.gz from https://archive.apache.org/dist/xerces/c/3/sources/), extracting and building following the provided instructions
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
        * From the directory where you downloaded CEINMS to run: 
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
        * Click Configure again, all fields should change from red back to white, then click generate
4. Open a terminal in the build directory and run the following commands to build and install to the specified location:
        ```bash
        make -j8
        make install -j8
        ```
    * If you have an error finding a library when running CEINMS from the install location, ensure the dependency library was correctly added to the path
    * You can add the chosen install directory to the path if you wish to run CEINMS from the terminal outside of the install directory
        e.g.
        ```bash
        $ echo 'export PATH=~CEINMS/install/bin:$PATH' >> ~/.bashrc
        ```


Acknowledgments
---------------
* Australian National Health and Medical Research Council (628850)
* Royal Society of NZ Marsden Fund (12-UOA-1221)
* US National Institutes of Health Grant (R01EB009351)
* Commission of the European Union Grant (IFP7-ICT-2013-10-611695)
* European Research Council (ERC) Advanced Grant DEMOVE (267888)
