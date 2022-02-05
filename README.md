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
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
```

Build project for LPC17xx Development Board:

```sh
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBOARD=LPC17xx_DevKit -DCMAKE_TOOLCHAIN_FILE=Libs/xcore/toolchains/cortex-m3.cmake -DPLATFORM=LPC17XX
```

Useful settings
----------

* CMAKE_BUILD_TYPE — specifies the build type. Possible values are empty, Debug, Release, RelWithDebInfo and MinSizeRel.
* USE_LTO — option enables Link Time Optimization.
