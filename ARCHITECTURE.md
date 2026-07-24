# TezCrypt Architecture Documentation

## Design Patterns and Principles

This document explains the architectural decisions and design patterns used in TezCrypt, enabling you to extend the application effectively.

## 1. Core Architecture

### 1.1 Layered Architecture

```
┌─────────────────────────────┐
│    User Interface (GUI)      │  ← MainWindow (Qt Widgets)
│  - Algorithm selection       │
│  - Text input/output        │
│  - Button handlers          │
└────────────┬────────────────┘
             │
             ↓
┌─────────────────────────────┐
│   Business Logic (Core)      │  ← EncryptionManager
│  - Algorithm registry       │
│  - Algorithm lookup         │
│  - Polymorphic dispatch     │
└────────────┬────────────────┘
             │
             ↓
┌─────────────────────────────┐
│  Algorithms (Implementation) │  ← EncryptionAlgorithm implementations
│  - CaesarCipher            │
│  - ROT13                    │
│  - Future algorithms...     │
└─────────────────────────────┘
```

**Key Benefit**: Each layer is independent and can be tested, modified, or extended without affecting others.

### 1.2 Dependencies Flow

```
main.cpp
  ↓
  ├→ Creates EncryptionManager
  ├→ Registers algorithms (CaesarCipher, ROT13)
  └→ Creates MainWindow(manager)
       ↓
       ├→ Queries manager for algorithm names
       ├→ On encrypt/decrypt click:
       │  ├→ Gets algorithm from manager
       │  └→ Calls algorithm→encrypt() or decrypt()
       └→ Displays result
```

**Key Benefit**: GUI never imports concrete algorithm classes. Loose coupling enables easy testing.

## 2. Design Patterns Used

### 2.1 Strategy Pattern

**What**: Each algorithm is a "strategy" for encryption/decryption.

**Implementation**:
```cpp
// Base strategy interface
class EncryptionAlgorithm {
    virtual QString encrypt(const QString&) const = 0;
    virtual QString decrypt(const QString&) const = 0;
};

// Concrete strategies
class CaesarCipher : public EncryptionAlgorithm { ... };
class ROT13 : public EncryptionAlgorithm { ... };
```

**Benefits**:
- New algorithms can be added without modifying existing ones
- Runtime algorithm selection (user picks from dropdown)
- Easy testing of individual algorithms

**Example Extension**:
```cpp
// Add AES cipher - no GUI changes needed
class AESCipher : public EncryptionAlgorithm {
    QString name() const override { return "AES-256"; }
    QString encrypt(const QString& pt) const override { ... }
    QString decrypt(const QString& ct) const override { ... }
};

// Just register it
manager->registerAlgorithm(std::make_shared<AESCipher>());
```

### 2.2 Registry Pattern

**What**: Central registry stores all available algorithms.

**Implementation** (`EncryptionManager`):
```cpp
class EncryptionManager {
private:
    std::unordered_map<std::string, EncryptionAlgorithmPtr> m_algorithms;

public:
    void registerAlgorithm(EncryptionAlgorithmPtr algo);
    EncryptionAlgorithmPtr getAlgorithm(const QString& id) const;
    QStringList getAlgorithmNames() const;
};
```

**Benefits**:
- Centralized algorithm management
- No switch/if-else chains for algorithm selection
- Easy to introspect available algorithms

**Usage in GUI**:
```cpp
// Populate dropdown (no hardcoded list!)
QStringList names = m_manager->getAlgorithmNames();
m_algorithmCombo->addItems(names);

// When user clicks encrypt
QString selectedName = m_algorithmCombo->currentText();
auto algorithm = m_manager->getAlgorithm(id); // Returns concrete algorithm
QString result = algorithm->encrypt(input);
```

### 2.3 Dependency Injection

**What**: Dependencies are provided externally, not created internally.

**Implementation**:
```cpp
// GUI receives manager from outside - doesn't create it
MainWindow::MainWindow(std::shared_ptr<EncryptionManager> manager, 
                       QWidget* parent)
    : QMainWindow(parent), m_manager(manager)
{
    // GUI uses manager but doesn't know how to create it
}
```

**Benefits**:
- Testability: Can inject mock manager for unit testing
- Flexibility: Manager can be configured differently for different uses
- Separation of concerns: GUI doesn't manage object lifecycle

### 2.4 Factory Pattern (Implicit)

**What**: `main.cpp` acts as a factory that creates and configures objects.

**Implementation**:
```cpp
int main(int argc, char* argv[]) {
    // Factory creates and wires everything
    auto manager = std::make_shared<EncryptionManager>();
    manager->registerAlgorithm(std::make_shared<CaesarCipher>());
    manager->registerAlgorithm(std::make_shared<ROT13>());
    
    MainWindow window(manager);
    window.show();
    
    return app.exec();
}
```

**Benefits**:
- Single place to add/remove algorithms
- Easy to enable/disable algorithms based on configuration
- Testable (can create different managers for different tests)

## 3. SOLID Principles

### 3.1 Single Responsibility Principle

Each class has one reason to change:

| Class | Responsibility | Reason to Change |
|-------|-----------------|------------------|
| `EncryptionAlgorithm` | Define algorithm interface | New method needed by all algorithms |
| `CaesarCipher` | Implement Caesar cipher | Algorithm logic changes |
| `EncryptionManager` | Store and retrieve algorithms | Storage/lookup mechanism changes |
| `MainWindow` | User interface | UI layout or interaction changes |

### 3.2 Open/Closed Principle

Classes are **open for extension, closed for modification**.

**Good** (Open for extension):
```cpp
// Adding a new algorithm: create new file, extend EncryptionAlgorithm
class BlowfishCipher : public EncryptionAlgorithm { ... };
manager->registerAlgorithm(std::make_shared<BlowfishCipher>());
// No existing code changes!
```

**Bad** (Requires modification):
```cpp
// Without manager, adding algorithm requires GUI changes:
if (algo == "blowfish") {
    result = blowfishEncrypt(input);  // GUI modified!
} else if (algo == "caesar") {
    result = caesarEncrypt(input);
}
```

### 3.3 Liskov Substitution Principle

Any `EncryptionAlgorithm` can be used anywhere another is expected.

```cpp
// Works with any algorithm, no special cases
void performCrypto(EncryptionAlgorithmPtr algo, const QString& input) {
    return algo->encrypt(input);  // Works for Caesar, ROT13, Blowfish...
}
```

### 3.4 Interface Segregation Principle

Algorithms only expose what they need to expose.

```cpp
// Minimal interface - algorithms don't expose internal state
class EncryptionAlgorithm {
public:
    virtual QString name() const = 0;
    virtual QString encrypt(const QString&) const = 0;
    virtual QString decrypt(const QString&) const = 0;
    // No internal details exposed!
};
```

### 3.5 Dependency Inversion Principle

Depend on abstractions, not concrete implementations.

**Good** (Depends on abstraction):
```cpp
class MainWindow {
    std::shared_ptr<EncryptionManager> m_manager;  // Abstraction layer
    
    void onEncryptClicked() {
        auto algo = m_manager->getAlgorithm(id);  // Returns EncryptionAlgorithmPtr
        algo->encrypt(input);  // Works with any algorithm
    }
};
```

**Bad** (Depends on concrete types):
```cpp
#include "CaesarCipher.h"
#include "ROT13.h"
// Now GUI depends on ALL algorithm implementations!
m_caesar.encrypt(input);
m_rot13.encrypt(input);
// Adding AES requires modifying GUI
```

## 4. Extensibility Points

### 4.1 Adding a New Algorithm

**Minimal changes required**:

1. **Create** `algorithms/MyAlgorithm.h`:
   ```cpp
   #include "EncryptionAlgorithm.h"
   
   class MyAlgorithm : public EncryptionAlgorithm {
   public:
       QString name() const override;
       QString encrypt(const QString& plaintext) const override;
       QString decrypt(const QString& ciphertext) const override;
   };
   ```

2. **Implement** `algorithms/MyAlgorithm.cpp`:
   ```cpp
   #include "MyAlgorithm.h"
   
   QString MyAlgorithm::name() const { 
       return "My Algorithm"; 
   }
   
   QString MyAlgorithm::encrypt(const QString& pt) const {
       // Implementation
   }
   
   QString MyAlgorithm::decrypt(const QString& ct) const {
       // Implementation
   }
   ```

3. **Add to CMake** (`algorithms/CMakeLists.txt`):
   ```cmake
   set(ALGORITHMS_SOURCES
       MyAlgorithm.cpp  # Add this line
       CaesarCipher.cpp
       ROT13.cpp
   )
   ```

4. **Register** in `src/main.cpp`:
   ```cpp
   manager->registerAlgorithm(std::make_shared<MyAlgorithm>());
   ```

5. **No GUI changes needed!**

### 4.2 Future: Algorithm Parameters

**Current state**: Fixed parameters (Caesar always shifts by 3)

**Future goal**: Dynamic parameters

**Planned interface**:
```cpp
struct ParameterSpec {
    QString name;           // "shift"
    QString displayName;    // "Shift Amount"
    QString type;           // "int", "string", "password"
    QVariant defaultValue;  // 3
    QString description;    // "Number of positions to shift"
};

class EncryptionAlgorithm {
    // ... existing methods ...
    virtual QList<ParameterSpec> getParameters() const;
    virtual void setParameter(const QString& name, const QVariant& value);
};
```

**Usage**:
```cpp
class CaesarCipher : public EncryptionAlgorithm {
public:
    QList<ParameterSpec> getParameters() const override {
        return {
            {"shift", "Shift Amount", "int", 3, "Characters to shift"}
        };
    }
    
    void setParameter(const QString& name, const QVariant& value) override {
        if (name == "shift") {
            m_shift = value.toInt();
        }
    }
    
private:
    int m_shift = 3;
};
```

**GUI Impact**:
```cpp
// Auto-generate parameter widgets
for (const auto& param : algorithm->getParameters()) {
    if (param.type == "int") {
        addSpinBox(param.name, param.defaultValue);
    } else if (param.type == "string") {
        addLineEdit(param.name, param.defaultValue);
    }
}
```

### 4.3 Future: File Operations

**Planned**:
- Encrypt files (not just text)
- Save encrypted output
- Load encrypted files for decryption

**Architecture**:
```cpp
class EncryptionAlgorithm {
    virtual QString encrypt(const QString&) const;     // Current
    virtual QString decrypt(const QString&) const;
    
    // Future
    virtual QByteArray encryptBytes(const QByteArray&) const;
    virtual QByteArray decryptBytes(const QByteArray&) const;
};
```

**No existing code breaks!**

## 5. Project Organization

### 5.1 Directory Rationale

```
algorithms/       Core encryption logic
  - Must not know about GUI
  - Must not know about EncryptionManager
  - Dependency: Only Qt Core
  
core/             Business logic and coordination
  - Manages algorithms
  - Must not know about GUI details
  - Dependency: Qt Core + algorithms
  
gui/              User interaction
  - Depends on core for algorithm access
  - Must not directly instantiate algorithms
  - Dependency: Qt Gui/Widgets + core
  
src/              Application entry point
  - Wires everything together
  - Dependency: everything else
```

**Dependency rule**: No circular dependencies. Higher layers can depend on lower layers, but not vice versa.

### 5.2 Build System Organization

```
CMakeLists.txt (root)
  ├── algorithms/CMakeLists.txt (tezcrypt_algorithms library)
  ├── core/CMakeLists.txt       (tezcrypt_core library, depends on algorithms)
  ├── gui/CMakeLists.txt        (tezcrypt_gui library, depends on core)
  └── src/CMakeLists.txt        (tezcrypt executable, depends on all)
```

**Benefits**:
- Each module builds independently
- Dependencies are explicit
- Easier to refactor (move classes between modules)
- Can create tests in parallel with libraries

## 6. Testing Strategy

### 6.1 Unit Testing Algorithms

**Goal**: Test each algorithm in isolation

```cpp
// test_caesar.cpp
#include <gtest/gtest.h>
#include "CaesarCipher.h"

class CaesarCipherTest : public ::testing::Test {
    CaesarCipher cipher;
};

TEST_F(CaesarCipherTest, EncryptSimple) {
    EXPECT_EQ(cipher.encrypt("abc"), "def");
}

TEST_F(CaesarCipherTest, DecryptSimple) {
    EXPECT_EQ(cipher.decrypt("def"), "abc");
}

TEST_F(CaesarCipherTest, Symmetric) {
    QString plaintext = "hello";
    EXPECT_EQ(cipher.decrypt(cipher.encrypt(plaintext)), plaintext);
}
```

### 6.2 Integration Testing Manager

```cpp
TEST(EncryptionManagerTest, RegisterAndRetrieve) {
    auto manager = std::make_shared<EncryptionManager>();
    manager->registerAlgorithm(std::make_shared<CaesarCipher>());
    
    auto algo = manager->getAlgorithm("caesar_cipher");
    EXPECT_NE(algo, nullptr);
    EXPECT_EQ(algo->name(), "Caesar Cipher");
}
```

### 6.3 GUI Testing

```cpp
TEST(MainWindowTest, AlgorithmSelection) {
    auto manager = std::make_shared<EncryptionManager>();
    manager->registerAlgorithm(std::make_shared<CaesarCipher>());
    
    MainWindow window(manager);
    
    // Verify dropdown was populated
    EXPECT_EQ(window.algorithmCount(), 1);
    EXPECT_TRUE(window.hasAlgorithm("Caesar Cipher"));
}
```

## 7. Performance Considerations

- **Text-based algorithms** (Caesar, ROT13): O(n) where n = text length
- **No memory leaks**: Smart pointers handle cleanup
- **Thread-safety**: Current implementation is single-threaded
  - Future: Can wrap algorithms in thread-safe proxy if needed

## 8. Security Considerations

⚠️ **IMPORTANT**: These are educational ciphers, NOT cryptographically secure:

- Caesar and ROT13: Trivial to break (brute-force all 26 shifts)
- Production use: Always use established libraries (OpenSSL, libsodium)
- This architecture **supports** secure algorithms without change

Example safe implementation:
```cpp
class AESCipher : public EncryptionAlgorithm {
    // Uses OpenSSL or similar for actual encryption
    // GUI doesn't change, just call encrypt() as usual
};
```

## 9. Future Enhancements

### 9.1 Configuration File Support

```cpp
// config.json
{
    "enabled_algorithms": ["caesar", "rot13", "aes"],
    "default_algorithm": "caesar",
    "ui_theme": "dark"
}
```

### 9.2 Plugin System

```cpp
class PluginManager {
    void loadPlugin(const QString& path);
    void unloadPlugin(const QString& id);
};

// Plugins can register algorithms at runtime
class MyPluginAlgorithm : public EncryptionAlgorithm { ... };
```

### 9.3 Batch Operations

```cpp
class BatchProcessor {
    void encryptAll(const QStringList& files, const QString& algorithmId);
    void decryptAll(const QStringList& files, const QString& algorithmId);
};
```

## Conclusion

TezCrypt demonstrates modern C++ architecture principles that scale from a simple 2-algorithm utility to a complex toolkit with dozens of algorithms. The key is:

1. **Abstract away implementation** (EncryptionAlgorithm)
2. **Centralize management** (EncryptionManager)
3. **Inject dependencies** (pass manager to GUI)
4. **Minimize coupling** (GUI never imports algorithms)

This allows the project to grow without architectural debt.
