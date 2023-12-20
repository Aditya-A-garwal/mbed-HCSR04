# mbed-HCSR04

![GitHub License](https://img.shields.io/github/license/Aditya-A-garwal/mbed-HCSR04)
![GitHub forks](https://img.shields.io/github/forks/Aditya-A-garwal/mbed-HCSR04?style=flat-square&color=blue)
![GitHub Repo stars](https://img.shields.io/github/stars/Aditya-A-garwal/mbed-HCSR04?style=flat-square&color=blue)
![GitHub issues](https://img.shields.io/github/issues-raw/Aditya-A-garwal/mbed-HCSR04?style=flat-square&color=indianred)
![GitHub closed issues](https://img.shields.io/github/issues-closed-raw/Aditya-A-garwal/mbed-HCSR04?style=flat-square)
![GitHub pull requests](https://img.shields.io/github/issues-pr/Aditya-A-garwal/mbed-HCSR04?style=flat-square&color=indianred)
![GitHub closed pull requests](https://img.shields.io/github/issues-pr-closed/Aditya-A-garwal/mbed-HCSR04?style=flat-square)

## Overview

This repository contains a simple library to use the *HCSR04 distance sensor* with MBed OS. An HCSR04 sensor measures distance, by measuring the amount of time it took between the transmission and receival of a sound wave. This duration can be used along with the speed of sound to calculate the distance between the sensor and a solid object.

The library provides *blocking* & *non-blocking* APIs, and allows different sensors connected to the same MCU to use different techniques at the same time. A more detailed explanation is found in the next sections.

## Usage

The process to use the library in your own *MBed CLI/MBed CE* projects consists of a few simple steps. This section assumes that CMake is being used as the build system. For other build systems/IDEs, use the source and header files directly and refer to their manuals.

The steps to use the library in your own projects are shown below -

1. Create a file named ```external/CMakeLists.txt``` in your project root with the following content. This will fetch the repository and add its targets to the project.

    ```diff
    FetchContent_Declare(mbed-HCSR04
        GIT_REPOSITORY
            https://github.com/Aditya-A-garwal/mbed-HCSR04.git
        GIT_TAG
            latest
    )

    FetchContent_MakeAvailable(mbed-HCSR04)
    ```

    ```latest``` after ```GIT_TAG``` uses the latest commit to the main branch of this repository. Replace it with the hash of the commit or the tag of the release that needs to be used in your project.

    More information about the FetchContent set of commands can be found [in the official CMake Docs](https://cmake.org/cmake/help/latest/module/FetchContent.html).

2. Add the following line to to the ```CMakeLists.txt``` in your project root. This will add the ```external``` directory to your project. Make sure to insert it before creating the ```APP_TARGET``` executable and after including the MBed OS directory.

    ```diff
    add_subdirectory(${MBED_PATH})

    + add_subdirectory(external)

    add_executable(${APP_TARGET}
        main.cpp
    )
    ```

3. Link the library with APP_TARGET (or other targets as required) by adding updating the following line -

    ```diff
    - target_link_libraries(${APP_TARGET} mbed-os)
    + target_link_libraries(${APP_TARGET} mbed-os mbed-HCSR04)
    ```

    This also updates the include path for the linked targets.

4. Configure and build the project by running the following commands in the root-directory of the project. This fetches the repository and makes the code available to the intellisense and autocomplete features in most IDEs.

    ```bash
    # Configure (fetches the repository and prepares the build-system)
    mbed-tools configure --toolchain <TOOLCHAIN> --mbed-target <TARGET_NAME> --profile <PROFILE>
    cmake -S . -B cmake_build/<TARGET_NAME>/<PROFILE>/<TOOLCHAIN>

    # Builds the code
    cmake --build cmake_build/<TARGET_NAME>/<PROFILE>/<TOOLCHAIN>
    ```

    Make sure to replace ```<TOOLCHAIN>```, ```<TARGET_NAME>``` and ```<PROFILE>``` with the appropriate
    values.

    **To change the version of the library being used, update the ```GIT_TAG``` parameter in ```external/CMakeLists.txt``` and re-run the configure and build commands. Re-building the project is not enough as the ```FetchContent_Declare``` command fetches the library while configuring the project only, and not while building.**

5. Include the header files in ```main.cpp``` (and other files as required) -

    ```cpp
    #include "HCSR04.h" // non-blocking APIs
    #include "HCSR04Blocking.h" // blocking APIs
    ```

## Organization of the Library

The library consists of two header files - ```HCSR04.h``` for non-blocking APIs and ```HCSR04Blocking.h``` for blocking APIs, each containing a single class. The classes are implemented in two source files - ```HCSR04.cpp``` and ```HCSR04Blocking.cpp```.

The steps to use the Blocking APIs are as follows -

1. Instantiate the ```HCSR04Blocking``` class.
2. Initialize the object by calling the ```initialize()``` method.
3. Measure distance using the ```get_distance()``` method. **This call blocks the execution of the current thread while the sensor measures distance.**
4. Finalize the object by calling the ```finalize()``` method. **Missing this step before the destructor is called will cause memory-leaks and zombie-threads.**
5. The object is destructed.

The steps to use the Non-Blocking APIs are as follows -

1. Instantiate the ```HCSR04``` class.
2. Initialize the object by calling the ```initialize()``` method.
3. Measure distance by calling the ```do_measurement(cb)``` method. **This call does not block the execution of the current thread. When the distance becomes available (or the sensor fails), the provided callback is executed in a separate thread.**
4. Automatically measure the distance at fixed intervals by using the ```start_measurement_periodic(period, cb)``` method. **This call does not block the execution of the current thread. Each time the distance becomes available (or the sensor fails), the provided callback is executed in a separate thread.**
5. Stop periodic measurement by calling the ```stop_measurement_periodic()``` method.
6. Finalize the object by calling the ```finalize()``` method. **Missing this step before the destructor is called will cause memory-leaks and zombie-threads.**
7. The object is destructed.

Detailed information is available as inline documentation within the header files.

## Documentation

The ```.h``` header files contain inline documentation for all classes, structs, functions and enums within it. This repository uses the Doxygen standard for inline-documentation. Regular comments explaining implementation details can be found in the ```.cpp``` source files.
