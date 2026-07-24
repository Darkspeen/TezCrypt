# Building TezCrypt - Detailed Guide

## Prerequisites Installation

### Linux - Ubuntu/Debian

```bash
# Update package list
sudo apt update

# Install Qt6 development files
sudo apt install -y \
    qt6-base-dev \
    qt6-declarative-dev \
    qt6-tools-dev \
    cmake \
    build-essential

# Verify installation
qmake6 --version
```

### Linux - Fedora/RHEL

```bash
# Install Qt6 development files
sudo dnf install -y \
    qt6-qtbase-devel \
    cmake \
    gcc-c++ \
    make

# Verify installation
qmake6 --version
```

### Linux - Arch

```bash
sudo pacman -S qt6-base cmake gcc
qmake6 --version
```

### Windows

1. **Download Qt Online Installer**
   - Visit: https://www.qt.io/download
   - Download "Qt Online Installer"

2. **Run Installer**
   - Choose: "Custom installation"
   - Select: Qt 6.x (Latest LTS recommended)
   - Select Component: "**Desktop MSVC 2022 64-bit**" (for Visual Studio 2022)
     - Or "**Desktop MinGW 11.2.0 64-bit**" (for MinGW)
   - Choose installation folder (e.g., `C:\Qt`)

3. **Configure CMake**
   ```bash
   mkdir build
   cd build
   cmake -DCMAKE_PREFIX_PATH="C:\Qt\6.6.3\msvc2022_64" ..
   ```
   (Adjust the version and year as needed)

### macOS

```bash
# Install using Homebrew
brew install qt6 cmake

# Verify installation
qmake6 --version

# Find Qt installation
brew --prefix qt6
```

## Building TezCrypt

### Quick Start (Linux/macOS)

```bash
# Clone/navigate to TezCrypt directory
cd /path/to/TezCrypt

# Create build directory
mkdir -p build
cd build

# Configure CMake
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build project (uses all available cores)
cmake --build . --parallel

# Run application
./bin/tezcrypt
```

### Linux with Custom Qt6 Location

If Qt6 was installed to a non-standard location:

```bash
cd /path/to/TezCrypt
mkdir -p build
cd build

cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH=/path/to/qt6/installation \
      ..

cmake --build . --parallel
./bin/tezcrypt
```

**Finding your Qt6 installation:**

```bash
# If installed with Online Installer:
ls ~/Qt/6.*/gcc*/

# If installed with system package:
ls /usr/lib*/cmake/Qt6/

# Using qmake to find it:
qmake6 -query QT_INSTALL_PREFIX
```

### Windows with MSVC

```bash
# Open Command Prompt or PowerShell with administrator privileges
cd C:\path\to\TezCrypt
mkdir build
cd build

# Configure (adjust Qt path and version as needed)
cmake -DCMAKE_PREFIX_PATH="C:\Qt\6.6.3\msvc2022_64" ^
      -DCMAKE_BUILD_TYPE=Release ^
      ..

# Build using Visual Studio
cmake --build . --config Release --parallel

# Run
.\bin\tezcrypt.exe
```

### Windows with MinGW

```bash
cmake -DCMAKE_PREFIX_PATH="C:\Qt\6.6.3\mingw_64" ^
      -DCMAKE_BUILD_TYPE=Release ..

cmake --build . --parallel
.\bin\tezcrypt.exe
```

### macOS with Homebrew

```bash
cd /path/to/TezCrypt
mkdir -p build
cd build

# Qt6 from Homebrew is usually auto-detected
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel
./bin/tezcrypt
```

## Troubleshooting

### CMake can't find Qt6

**Error:**
```
Could not find a package configuration file provided by "Qt6"
```

**Solution:**
```bash
# Find your Qt6 installation
qmake6 -query QT_INSTALL_PREFIX

# Use that path with CMake
cmake -DCMAKE_PREFIX_PATH=/path/from/above ..
```

### Permission denied on Linux

If you get permission errors running the application:

```bash
# Make sure the binary is executable
chmod +x ./bin/tezcrypt

# Run with ./bin/tezcrypt
```

### Linker errors on Windows

Make sure you're using the correct Qt version for your compiler:
- Qt "MSVC 2022 64-bit" requires Visual Studio 2022
- Qt "MinGW 11.2.0 64-bit" requires MinGW compiler

### Application won't start on Linux

Missing dependencies can prevent the GUI from initializing:

```bash
# Check for missing libraries
ldd ./bin/tezcrypt

# On Ubuntu/Debian, install any missing Qt6 runtime libraries:
sudo apt install libqt6gui6 libqt6widgets6 libqt6core6
```

### Rebuilding after CMakeLists.txt changes

When CMakeLists.txt changes, do a clean rebuild:

```bash
cd build
rm -rf * CMakeCache.txt
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel
```

## Alternative: Using Qt Creator

1. Open **Qt Creator**
2. File → Open File or Project
3. Navigate to and select `CMakeLists.txt` from TezCrypt root
4. Select your preferred Kit (compiler and Qt version)
5. Click "Configure Project"
6. Press Ctrl+B (or Cmd+B on macOS) to build
7. Press Ctrl+R to run

## Development Workflow

After successful build, for development:

```bash
# In the build directory
cd build

# Rebuild after code changes
cmake --build .

# Run from build directory
./bin/tezcrypt
```

For incremental builds during development, just run `cmake --build .` from the build directory without reconfiguring.

## Running Unit Tests (Future)

When unit tests are added:

```bash
cd build
cmake --build .
ctest --output-on-failure
```

## Creating a Distribution Package

### Linux

```bash
cd build
cmake --install . --prefix /tmp/tezcrypt_install
# Creates installable package at /tmp/tezcrypt_install/bin/tezcrypt
```

### Windows

```bash
cd build
cmake --install . --prefix "C:\Program Files\TezCrypt"
```

## Getting Help

If you encounter issues:

1. Check this file for solutions
2. Ensure Qt6 is properly installed: `qmake6 --version`
3. Verify CMake version: `cmake --version` (should be 3.24+)
4. Check compiler version: `g++ --version` or `clang --version` (C++20 support required)
5. Try a clean build: Remove `build/` directory and start over
