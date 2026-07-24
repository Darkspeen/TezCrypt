# TezCrypt - Modular Encryption/Decryption Utility

A cross-platform desktop application built with C++ and Qt 6, designed as a highly extensible encryption toolkit.

## Features

- **Cross-Platform**: Builds and runs on Windows and Linux without code changes
- **Modular Architecture**: Easily add new encryption algorithms without modifying UI code
- **Clean Separation of Concerns**: GUI, Core logic, and Algorithms in separate modules
- **Modern C++**: Uses C++20 with smart pointers and SOLID principles
- **Qt 6 Widgets**: Native desktop UI with responsive layouts

## Project Structure

```
TezCrypt/
├── CMakeLists.txt                 # Root CMake configuration
├── algorithms/                    # Encryption algorithm implementations
│   ├── EncryptionAlgorithm.h     # Abstract base class for all algorithms
│   ├── CaesarCipher.h/.cpp       # Caesar cipher implementation
│   ├── ROT13.h/.cpp              # ROT13 cipher implementation
│   └── CMakeLists.txt
├── core/                          # Core business logic
│   ├── EncryptionManager.h/.cpp  # Registry and manager for algorithms
│   └── CMakeLists.txt
├── gui/                           # User interface
│   ├── MainWindow.h/.cpp          # Main application window
│   └── CMakeLists.txt
└── src/                           # Application entry point
    ├── main.cpp                   # Application startup
    └── CMakeLists.txt
```

## Architecture Overview

### Design Principles

1. **Strategy Pattern**: Each encryption algorithm implements the `EncryptionAlgorithm` interface
2. **Registry Pattern**: `EncryptionManager` manages all available algorithms
3. **Dependency Injection**: GUI receives manager via constructor
4. **Polymorphism**: GUI never knows concrete algorithm types
5. **SOLID Principles**:
   - Single Responsibility: Each class has one reason to change
   - Open/Closed: Open for extension, closed for modification
   - Liskov Substitution: All algorithms are interchangeable
   - Interface Segregation: Minimal required interface
   - Dependency Inversion: Depends on abstractions, not concrete classes

### Adding New Algorithms

To add a new encryption algorithm:

1. **Create Header** (`algorithms/MyAlgorithm.h`):
   ```cpp
   #include "EncryptionAlgorithm.h"
   
   class MyAlgorithm : public EncryptionAlgorithm {
   public:
       QString name() const override;
       QString encrypt(const QString& plaintext) const override;
       QString decrypt(const QString& ciphertext) const override;
   };
   ```

2. **Implement** (`algorithms/MyAlgorithm.cpp`):
   ```cpp
   #include "MyAlgorithm.h"
   
   QString MyAlgorithm::name() const { return "My Algorithm"; }
   QString MyAlgorithm::encrypt(const QString& pt) const { ... }
   QString MyAlgorithm::decrypt(const QString& ct) const { ... }
   ```

3. **Register** (in `src/main.cpp`):
   ```cpp
   manager->registerAlgorithm(std::make_shared<MyAlgorithm>());
   ```

4. **Update CMake** (`algorithms/CMakeLists.txt`):
   ```cmake
   set(ALGORITHMS_SOURCES
       MyAlgorithm.cpp  # Add this
       ...
   )
   ```

That's it! No UI changes needed.

## Building

### Prerequisites

- CMake 3.24 or later
- Qt 6.0 or later
- C++20 compatible compiler (GCC 10+, Clang 11+, MSVC 2019+)

### Linux

```bash
# Create build directory
mkdir build
cd build

# Configure (assumes Qt6 is in standard location)
cmake ..

# Build
cmake --build .

# Run
./bin/tezcrypt
```

### Windows

```bash
# Create build directory
mkdir build
cd build

# Configure (adjust Qt path as needed)
cmake -DCMAKE_PREFIX_PATH="C:\Qt\6.x\msvc2019_64" ..

# Build
cmake --build . --config Release

# Run
.\bin\tezcrypt.exe
```

### macOS

```bash
mkdir build
cd build
cmake ..
cmake --build .
./bin/tezcrypt
```

## Usage

1. **Select Algorithm**: Choose an encryption method from the dropdown
2. **Enter Text**: Paste text to encrypt/decrypt in the input box
3. **Encrypt/Decrypt**: Click the corresponding button
4. **View Result**: The output appears in the result box

## Current Algorithms

- **Caesar Cipher**: Classic substitution cipher with fixed shift of 3
- **ROT13**: Rotation cipher by 13 positions (symmetric)

## Future Enhancements

### Dynamic Parameter Interface

The architecture supports future development of dynamic parameter interfaces:

```cpp
class EncryptionAlgorithm {
    virtual QList<ParameterDescriptor> getParameters() const;
    virtual void setParameter(const QString& name, const QVariant& value);
};
```

This would allow the GUI to auto-generate parameter input widgets based on algorithm requirements.

### Planned Algorithms

- AES (with password/IV parameters)
- Blowfish
- Twofish
- Base64 encoding/decoding
- Substitution cipher with custom key
- Vigenère cipher with user-supplied key

## Code Quality

- **Smart Pointers**: All ownership is explicit using `std::shared_ptr` and `std::unique_ptr`
- **No Raw Pointers**: Except for Qt parent-child relationships (Qt manages cleanup)
- **Modern C++**: Uses C++20 features like structured bindings, auto, and range-based for loops
- **const Correctness**: Methods marked const where appropriate
- **Exception Safety**: Basic exception guarantees with error handling
- **Documentation**: Comprehensive Doxygen-style comments

## Platform Compatibility

- ✅ Linux (tested on Ubuntu 20.04+, Fedora 33+)
- ✅ Windows (tested on Windows 10/11)
- ✅ macOS (theoretical support, cross-platform CMake)

The application uses only:
- Qt 6 Core, Gui, Widgets (cross-platform)
- Standard C++ library
- CMake (cross-platform build system)

No platform-specific code is used, ensuring clean portability.

## License

This project is provided as-is for educational and professional use.

## Author

Created as a demonstration of modern C++ desktop application architecture using Tezcatlipoca's legacy.
