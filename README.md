Installation
------------

Examples require GNU toolchain for ARM Cortex-M processors and CMake version 3.6 or newer.

Quickstart
----------

Clone git repository:

```sh
git clone https://github.com/stxent/vfs-shell.git
cd vfs-shell
git submodule update --init --recursive
```

Build project for Linux:

```sh
cmake .. -DPLATFORM=Linux
```

Build project for LPC17xx Development Board:

```sh
cmake .. -DPLATFORM=LPC17xx_DevKit -DCMAKE_TOOLCHAIN_FILE=Toolchains/arm-none-eabi.cmake
```

Useful settings
----------

* CMAKE_BUILD_TYPE — specifies the build type. Possible values are empty, Debug, Release, RelWithDebInfo and MinSizeRel.
* USE_LTO — option enables Link Time Optimization.
