# OPC UA Server Geco Package

## Overview
This package provides an OPC UA server using the open62541 library. Depending on the build configuration, the open62541 library can either be included directly into the package (static linking) or linked externally as a shared library.

## Dependencies
The open62541 library ([GitHub Repository](https://github.com/open62541/open62541)) needs to be installed either as a shared library or built as a static library if you want to include it directly into the Geco OPC UA package.

## Building open62541
Refer to the official documentation for detailed instructions: [open62541 Build Guide](https://www.open62541.org/doc/1.0/building.html#building-the-library).

### Step-by-Step Instructions
1. **Clone the repository**:
   ```bash
   git clone https://github.com/open62541/open62541.git
   cd open62541
   ```

2. **Get dependencies**:
   ```bash
   sudo apt-get install git build-essential gcc pkg-config cmake python
   ```

3. **Initialize and update submodules**:
   ```bash
   git submodule update --init --recursive
   ```

4. **Create a `build` folder inside the `open62541` directory**:
   ```bash
   mkdir build
   cd build
   ```

5. **Configure the build process**:
   - For a **shared library**:
     ```bash
     cmake -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
     ```
   - For a **static library**:
     ```bash
     cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
     ```
   > **Note**: The `BUILD_SHARED_LIBS` flag controls whether the library is built as shared (`ON`) or static (`OFF`).

6. **Build the library**:
   ```bash
   make
   ```
   This will create either a shared or static library depending on the `BUILD_SHARED_LIBS` flag set in step 5.

7. **Install the shared library**:
   If a shared library was built, install it system-wide:
   ```bash
   sudo make install
   sudo ldconfig
   ```

8. **Getting the static library**:
   The static library `libopen62541.a` will typically be found in the `bin` folder of your `build` directory. Copy it to the folder of this Geco package:
   ```bash
   cp build/bin/libopen62541.a /path/to/geco/package
   ```

### Notes
As of now, there is an open issue ([#6765](https://github.com/open62541/open62541/issues/6765)) that prevents the latest version of open62541 from being fully compatible with C++ projects. To work around this issue, use the version from commit `2babcfc`. To retrieve that specific commit:
```bash
git checkout 2babcfc
```

## Building the Geco OPC UA Package
After building the open62541 library, proceed with building the Geco OPC UA package.

### Folder Structure Example
Make sure your project is organized like this (`libopen62541.a` only needed if you plan to include into the geco package):
```
project-root/
├── gecoOPCUAPkg.cc
├── gecoOPCUAServer.cc
├── gecoOPCUAPkg.h
├── gecoOPCUAServer.h
├── Makefile
└── libopen62541.a
```

### Build Options
There are two options for building the package, depending on whether you want to include the open62541 library or link to it externally.

1. **Linking against the shared library of open62541**:
   To build the package without including the open62541 library:
   ```bash
   make OPEN62541_LINK_TYPE=SHARED
   ```
   This means that Geco applications using this package must also link against the shared library of open62541.

2. **Including the open62541 library into the OPC UA package**:
   To build the package by statically including the open62541 library:
   ```bash
   make
   ```
   This way, Geco applications can use this package even if the open62541 library is not installed system-wide.

   > **Note**: If `OPEN62541_LINK_TYPE` is not specified, the build defaults to including the static library.

## Cleaning Up
To clean up the build directory, use:
```bash
make clean
```

## Installation
To install the package:
```bash
make install
```
This will copy the compiled package to `/usr/local/share/geco/pkg/` and update the shared library cache with:
```bash
sudo ldconfig
```
