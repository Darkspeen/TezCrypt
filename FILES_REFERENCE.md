# TezCrypt Project Files Reference

## Directory Structure

```
TezCrypt/
│
├── CMakeLists.txt                 # Root CMake configuration (C++20, Qt6 packages)
├── README.md                      # Project overview and usage guide
├── BUILDING.md                    # Detailed build instructions for all platforms
├── ARCHITECTURE.md                # Design patterns and extensibility documentation
├── EXTENDING.md                   # Step-by-step guides for adding new algorithms
│
├── src/
│   ├── main.cpp                   # Application entry point (registers algorithms, creates GUI)
│   └── CMakeLists.txt             # Builds tezcrypt executable
│
├── algorithms/
│   ├── EncryptionAlgorithm.h      # Abstract base class (Strategy pattern)
│   ├── CaesarCipher.h             # Caesar cipher header (shift by 3)
│   ├── CaesarCipher.cpp           # Caesar cipher implementation
│   ├── ROT13.h                    # ROT13 cipher header (shift by 13, symmetric)
│   ├── ROT13.cpp                  # ROT13 cipher implementation
│   └── CMakeLists.txt             # Builds tezcrypt_algorithms static library
│
├── core/
│   ├── EncryptionManager.h        # Registry pattern for algorithm management
│   ├── EncryptionManager.cpp      # Manager implementation (stores/retrieves algorithms)
│   └── CMakeLists.txt             # Builds tezcrypt_core static library
│
└── gui/
    ├── MainWindow.h               # Qt MainWindow (Strategy pattern user)
    ├── MainWindow.cpp             # GUI implementation (non-specific to algorithms)
    └── CMakeLists.txt             # Builds tezcrypt_gui static library
```

## File Purposes and Key Details

### Core Architecture Files

| File | Purpose | Key Classes/Functions |
|------|---------|----------------------|
| `algorithms/EncryptionAlgorithm.h` | Abstract interface for all algorithms | `EncryptionAlgorithm`, `EncryptionAlgorithmPtr` |
| `core/EncryptionManager.h/.cpp` | Registry and lifecycle management | `EncryptionManager::registerAlgorithm()`, `getAlgorithm()` |
| `gui/MainWindow.h/.cpp` | Qt-based user interface | `MainWindow`, slots for encrypt/decrypt |
| `src/main.cpp` | Application bootstrap | Creates manager, registers algorithms, launches window |

### Algorithm Implementation Files

| File | Algorithm | Shift | Symmetric | Notes |
|------|-----------|-------|-----------|-------|
| `algorithms/CaesarCipher.h/.cpp` | Caesar | 3 | No | Classic substitution cipher |
| `algorithms/ROT13.h/.cpp` | ROT13 | 13 | Yes | Encoding/decoding are identical |

### CMake Build Files

| File | Purpose | Produces |
|------|---------|----------|
| `CMakeLists.txt` (root) | Finds Qt6, adds subdirectories | Orchestrates build |
| `algorithms/CMakeLists.txt` | Compiles algorithm implementations | `libtezcrypt_algorithms.a` |
| `core/CMakeLists.txt` | Compiles manager | `libtezcrypt_core.a` |
| `gui/CMakeLists.txt` | Compiles GUI (with Qt6 MOC) | `libtezcrypt_gui.a` |
| `src/CMakeLists.txt` | Links everything into executable | `tezcrypt` (or `tezcrypt.exe` on Windows) |

### Documentation Files

| File | Audience | Content |
|------|----------|---------|
| `README.md` | End users | Overview, features, quick start |
| `BUILDING.md` | Developers | Platform-specific build instructions |
| `ARCHITECTURE.md` | Developers | Design patterns, SOLID principles, extensibility |
| `EXTENDING.md` | Algorithm developers | Step-by-step guide to add new algorithms |

## Dependencies and Relationships

### Build Order (CMake handles automatically)

```
algorithms/CaesarCipher.cpp
algorithms/ROT13.cpp
├→ libtezcrypt_algorithms.a
│
core/EncryptionManager.cpp
├→ depends on: libtezcrypt_algorithms
│
└→ libtezcrypt_core.a
   │
   gui/MainWindow.cpp
   ├→ depends on: libtezcrypt_core
   │
   └→ libtezcrypt_gui.a
      │
      src/main.cpp
      ├→ depends on: libtezcrypt_gui, libtezcrypt_core, libtezcrypt_algorithms
      │
      └→ tezcrypt (executable)
```

### Header Dependencies

```
src/main.cpp
├── MainWindow.h
│   ├── EncryptionManager.h
│   │   └── EncryptionAlgorithm.h
│   │       └── Qt6::Core
│   └── Qt6::Widgets
├── EncryptionManager.h (direct)
├── CaesarCipher.h
│   └── EncryptionAlgorithm.h
└── ROT13.h
    └── EncryptionAlgorithm.h
```

## Code Patterns Used

### Pattern 1: Strategy (Algorithm Interface)
- **File**: `algorithms/EncryptionAlgorithm.h`
- **Usage**: All algorithms inherit and implement `encrypt()`, `decrypt()`
- **Benefit**: GUI doesn't import concrete algorithm headers

### Pattern 2: Registry (Algorithm Management)
- **File**: `core/EncryptionManager.h/.cpp`
- **Usage**: Stores algorithms by ID, returns by lookup
- **Benefit**: No switch/if-else chains for algorithm selection

### Pattern 3: Dependency Injection (GUI Setup)
- **File**: `gui/MainWindow.h`, `src/main.cpp`
- **Usage**: Manager passed to MainWindow constructor
- **Benefit**: Testable, decoupled, flexible

### Pattern 4: Factory (Application Bootstrap)
- **File**: `src/main.cpp`
- **Usage**: Creates manager, registers all algorithms, creates window
- **Benefit**: Single place to add/remove algorithms

## SOLID Principles Implementation

| Principle | How Applied | File References |
|-----------|-------------|-----------------|
| **S**ingle Responsibility | Each class handles one concern | `CaesarCipher` only does Caesar encryption |
| **O**pen/Closed | Open for algorithm extension, closed for modification | Add algorithms without changing GUI |
| **L**iskov Substitution | Any algorithm implementer is interchangeable | `EncryptionAlgorithm` interface |
| **I**nterface Segregation | Minimal algorithm interface | Only 3 methods: `name()`, `encrypt()`, `decrypt()` |
| **D**ependency Inversion | Depends on abstractions, not concrete types | GUI uses `EncryptionManager`, not concrete algorithms |

## Key C++ Features Used

- **Smart Pointers**: `std::shared_ptr<EncryptionAlgorithm>` for ownership
- **Abstract Classes**: `EncryptionAlgorithm` with pure virtual methods
- **Structured Bindings**: `for (const auto& [id, algo] : m_algorithms)`
- **Move Semantics**: Qt containers and STL
- **Constexpr**: `static constexpr int DEFAULT_SHIFT = 3`
- **String Types**: `QString` for Qt integration, `std::string` for internal storage
- **Modern STL**: `std::unordered_map`, `std::make_shared()`

## Qt Features Used

- **Qt Core**: `QString`, `QStringList`, memory management
- **Qt Widgets**: `QMainWindow`, `QComboBox`, `QTextEdit`, `QPushButton`, `QVBoxLayout`
- **Qt MOC**: `Q_OBJECT` macro for signal/slot system
- **Qt Layouts**: `QVBoxLayout`, `QHBoxLayout` for responsive UI

## Adding Elements Checklist

### To Add New Algorithm
- [ ] Create `.h` file in `algorithms/`
- [ ] Create `.cpp` file in `algorithms/`
- [ ] Add to `algorithms/CMakeLists.txt` sources
- [ ] Include in `src/main.cpp`
- [ ] Register in `main()`

### To Add New GUI Element
- [ ] Add member variable to `MainWindow.h`
- [ ] Create widget in `setupUI()`
- [ ] Connect signals/slots
- [ ] Add handling in relevant slot

### To Add New Core Manager Feature
- [ ] Add method to `EncryptionManager.h`
- [ ] Implement in `EncryptionManager.cpp`
- [ ] Use from `MainWindow` as needed

## File Sizes (Typical)

| File | Lines | Purpose |
|------|-------|---------|
| `EncryptionAlgorithm.h` | 40 | Interface definition |
| `CaesarCipher.h` | 35 | Algorithm header |
| `CaesarCipher.cpp` | 35 | Algorithm implementation |
| `ROT13.h` | 30 | Algorithm header |
| `ROT13.cpp` | 30 | Algorithm implementation |
| `EncryptionManager.h` | 50 | Manager interface |
| `EncryptionManager.cpp` | 45 | Manager implementation |
| `MainWindow.h` | 60 | GUI class definition |
| `MainWindow.cpp` | 140 | GUI implementation |
| `main.cpp` | 25 | Application entry |
| CMakeLists.txt files | ~30 each | Build configuration |

## Quick References

### Running the Application
```bash
cd build
./bin/tezcrypt              # Linux/macOS
.\bin\tezcrypt.exe          # Windows
```

### Rebuilding
```bash
cd build
cmake --build .
```

### Full Rebuild
```bash
cd /path/to/TezCrypt
rm -rf build
mkdir build
cd build
cmake ..
cmake --build .
```

### Finding Algorithm Implementations
- Location: `algorithms/` directory
- Pattern: Each algorithm has `.h` and `.cpp` files
- Base: All inherit from `EncryptionAlgorithm`

### Finding GUI Code
- Main window: `gui/MainWindow.h` and `.cpp`
- Slots: `onEncryptClicked()`, `onDecryptClicked()`
- Layout setup: `setupUI()`

### Finding Manager/Registry
- Central registry: `core/EncryptionManager.h` and `.cpp`
- Key methods: `registerAlgorithm()`, `getAlgorithm()`, `getAlgorithmNames()`

## Future File Additions

Planned additions when implementing new features:

```
algorithms/VigenereCipher.h/.cpp        # For Vigenere cipher
algorithms/AESCipher.h/.cpp             # For AES encryption
core/ParameterDescriptor.h               # For algorithm parameters
gui/ParameterWidget.h/.cpp               # For dynamic parameter UI
tests/TestAlgorithms.cpp                 # Unit tests
tests/CMakeLists.txt                     # Test configuration
```
