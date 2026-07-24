# Adding New Algorithms - Step-by-Step Guide

This guide walks through adding a new encryption algorithm to TezCrypt using a real-world example.

## Example: Adding the Substitution Cipher

We'll add a Substitution Cipher that uses a keyword to create a substitution table.

### Step 1: Create the Header File

Create `algorithms/SubstitutionCipher.h`:

```cpp
#ifndef SUBSTITUTION_CIPHER_H
#define SUBSTITUTION_CIPHER_H

#include "EncryptionAlgorithm.h"
#include <QString>

/**
 * @class SubstitutionCipher
 * @brief Substitution cipher using keyword-based substitution table
 * 
 * Uses a simple substitution based on the alphabet with a fixed pattern.
 * This is NOT cryptographically secure - for demonstration only.
 */
class SubstitutionCipher : public EncryptionAlgorithm {
public:
    SubstitutionCipher();
    ~SubstitutionCipher() override = default;

    QString name() const override;
    QString encrypt(const QString& plaintext) const override;
    QString decrypt(const QString& ciphertext) const override;

private:
    static constexpr const char* CIPHER_KEY = 
        "nopqrstuvwxyzabcdefghijklm";  // ROT13-like key (for demo)

    /**
     * @brief Apply substitution to a character
     * @param ch Character to substitute
     * @param forward If true, apply encryption; if false, reverse
     * @return Substituted character
     */
    QChar substituteChar(QChar ch, bool forward) const;
};

#endif // SUBSTITUTION_CIPHER_H
```

### Step 2: Create the Implementation

Create `algorithms/SubstitutionCipher.cpp`:

```cpp
#include "SubstitutionCipher.h"

SubstitutionCipher::SubstitutionCipher() = default;

QString SubstitutionCipher::name() const {
    return QStringLiteral("Substitution Cipher");
}

QString SubstitutionCipher::encrypt(const QString& plaintext) const {
    QString result;
    result.reserve(plaintext.size());
    
    for (const QChar& ch : plaintext) {
        result.append(substituteChar(ch, true));
    }
    
    return result;
}

QString SubstitutionCipher::decrypt(const QString& ciphertext) const {
    QString result;
    result.reserve(ciphertext.size());
    
    for (const QChar& ch : ciphertext) {
        result.append(substituteChar(ch, false));
    }
    
    return result;
}

QChar SubstitutionCipher::substituteChar(QChar ch, bool forward) const {
    if (ch.isUpper()) {
        int pos = ch.unicode() - 'A';
        if (pos >= 0 && pos < 26) {
            // Apply substitution table
            int newPos = (pos + 13) % 26;  // Simple ROT13 for demo
            return QChar('A' + newPos);
        }
    } else if (ch.isLower()) {
        int pos = ch.unicode() - 'a';
        if (pos >= 0 && pos < 26) {
            int newPos = (pos + 13) % 26;
            return QChar('a' + newPos);
        }
    }
    
    // Non-alphabetic characters pass through unchanged
    return ch;
}
```

### Step 3: Add to CMakeLists.txt

Edit `algorithms/CMakeLists.txt`:

**Before:**
```cmake
set(ALGORITHMS_SOURCES
    CaesarCipher.cpp
    ROT13.cpp
)

set(ALGORITHMS_HEADERS
    EncryptionAlgorithm.h
    CaesarCipher.h
    ROT13.h
)
```

**After:**
```cmake
set(ALGORITHMS_SOURCES
    CaesarCipher.cpp
    ROT13.cpp
    SubstitutionCipher.cpp   # Add this line
)

set(ALGORITHMS_HEADERS
    EncryptionAlgorithm.h
    CaesarCipher.h
    ROT13.h
    SubstitutionCipher.h     # Add this line
)
```

### Step 4: Register in main.cpp

Edit `src/main.cpp`:

**Before:**
```cpp
#include "CaesarCipher.h"
#include "ROT13.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    auto manager = std::make_shared<EncryptionManager>();

    manager->registerAlgorithm(std::make_shared<CaesarCipher>());
    manager->registerAlgorithm(std::make_shared<ROT13>());

    MainWindow window(manager);
    window.show();

    return app.exec();
}
```

**After:**
```cpp
#include "CaesarCipher.h"
#include "ROT13.h"
#include "SubstitutionCipher.h"  // Add this include

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    auto manager = std::make_shared<EncryptionManager>();

    manager->registerAlgorithm(std::make_shared<CaesarCipher>());
    manager->registerAlgorithm(std::make_shared<ROT13>());
    manager->registerAlgorithm(std::make_shared<SubstitutionCipher>());  // Add this line

    MainWindow window(manager);
    window.show();

    return app.exec();
}
```

### Step 5: Rebuild

```bash
cd build
cmake --build .
./bin/tezcrypt
```

That's it! The new "Substitution Cipher" should appear in the dropdown.

## Advanced Example: Algorithm with Parameters

This example shows how to prepare for parameter-based algorithms (future feature).

### Header with Parameter Support

```cpp
#ifndef ADVANCED_CIPHER_H
#define ADVANCED_CIPHER_H

#include "EncryptionAlgorithm.h"

class AdvancedCipher : public EncryptionAlgorithm {
public:
    AdvancedCipher();
    
    QString name() const override;
    QString encrypt(const QString& plaintext) const override;
    QString decrypt(const QString& ciphertext) const override;
    
    // Future: support for parameters
    void setShift(int shift) { m_shift = shift; }
    int getShift() const { return m_shift; }
    
private:
    int m_shift = 5;  // User-configurable parameter
};

#endif
```

### Implementation

```cpp
#include "AdvancedCipher.h"

AdvancedCipher::AdvancedCipher() = default;

QString AdvancedCipher::name() const {
    return QStringLiteral("Advanced Cipher");
}

QString AdvancedCipher::encrypt(const QString& plaintext) const {
    QString result;
    result.reserve(plaintext.size());
    
    // Use m_shift instead of hardcoded value
    for (const QChar& ch : plaintext) {
        if (ch.isUpper()) {
            int pos = ch.unicode() - 'A';
            pos = (pos + m_shift) % 26;
            result.append(QChar('A' + pos));
        } else if (ch.isLower()) {
            int pos = ch.unicode() - 'a';
            pos = (pos + m_shift) % 26;
            result.append(QChar('a' + pos));
        } else {
            result.append(ch);
        }
    }
    
    return result;
}

QString AdvancedCipher::decrypt(const QString& ciphertext) const {
    QString result;
    result.reserve(ciphertext.size());
    
    for (const QChar& ch : ciphertext) {
        if (ch.isUpper()) {
            int pos = ch.unicode() - 'A';
            pos = (pos - m_shift) % 26;
            if (pos < 0) pos += 26;
            result.append(QChar('A' + pos));
        } else if (ch.isLower()) {
            int pos = ch.unicode() - 'a';
            pos = (pos - m_shift) % 26;
            if (pos < 0) pos += 26;
            result.append(QChar('a' + pos));
        } else {
            result.append(ch);
        }
    }
    
    return result;
}
```

### Future: Dynamic GUI Generation

When parameter support is implemented in the GUI:

```cpp
// In MainWindow: Auto-generate UI elements
auto cipher = dynamic_cast<AdvancedCipher*>(algorithm.get());
if (cipher) {
    // Create spin box for m_shift parameter
    auto spinBox = new QSpinBox();
    spinBox->setMaximum(25);
    spinBox->setValue(cipher->getShift());
    
    connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [cipher](int value) { cipher->setShift(value); });
    
    m_parameterLayout->addWidget(spinBox);
}
```

## Testing Your New Algorithm

Create a simple test file `test_new_algorithm.cpp`:

```cpp
#include <iostream>
#include "SubstitutionCipher.h"

int main() {
    SubstitutionCipher cipher;
    
    // Test 1: Basic encryption
    QString plaintext = "Hello World";
    QString encrypted = cipher.encrypt(plaintext);
    
    std::cout << "Original: " << plaintext.toStdString() << std::endl;
    std::cout << "Encrypted: " << encrypted.toStdString() << std::endl;
    
    // Test 2: Decryption returns to original
    QString decrypted = cipher.decrypt(encrypted);
    std::cout << "Decrypted: " << decrypted.toStdString() << std::endl;
    
    if (decrypted == plaintext) {
        std::cout << "✓ Test PASSED: Encryption/Decryption is symmetric" << std::endl;
        return 0;
    } else {
        std::cout << "✗ Test FAILED" << std::endl;
        return 1;
    }
}
```

## Algorithm Checklist

Before adding a new algorithm, ensure:

- [ ] Class extends `EncryptionAlgorithm`
- [ ] `name()` returns a user-friendly name
- [ ] `encrypt()` accepts plaintext, returns ciphertext
- [ ] `decrypt()` accepts ciphertext, returns plaintext
- [ ] Non-alphabetic characters handled correctly (preserved or documented)
- [ ] Unicode support considered (if needed)
- [ ] `.h` file includes guard and documentation
- [ ] `.cpp` file implements all virtual methods
- [ ] Added to `algorithms/CMakeLists.txt`
- [ ] Included in `src/main.cpp`
- [ ] Algorithm registered: `manager->registerAlgorithm(...)`
- [ ] Tested manually through GUI
- [ ] Edge cases tested (empty string, special chars, numbers)

## Common Issues

### Issue: New algorithm doesn't appear in dropdown

**Check:**
1. Algorithm registered in `main.cpp`?
2. `CMakeLists.txt` updated with new files?
3. Clean build performed? (`rm -rf build && mkdir build && cd build && cmake .. && cmake --build .`)
4. `name()` returns non-empty string?

### Issue: Encryption produces garbage

**Check:**
1. Character encoding handled correctly?
2. Bounds checking (modulo arithmetic correct)?
3. Non-alphabetic characters handled?
4. Test with simple strings first ("abc" → expected output)

### Issue: Decryption doesn't reverse encryption

**Check:**
1. `m_shift` value in `decrypt()` correct (should be negative if positive in `encrypt()`)?
2. Negative modulo handled correctly: `(pos - shift) % 26` can be negative!
   ```cpp
   int pos = (pos - shift) % 26;
   if (pos < 0) pos += 26;  // Critical!
   ```

## Performance Tips

- Reserve string sizes: `result.reserve(input.size())`
- Use `QString::toLatin1()` for large text
- Batch character operations when possible
- Profile with: `std::chrono::high_resolution_clock`

## Next Steps

1. Add your algorithm following this guide
2. Test it through the GUI
3. Consider edge cases and Unicode
4. Document any assumptions (e.g., "only handles ASCII")
5. Look at [ARCHITECTURE.md](ARCHITECTURE.md) for advanced patterns
6. Plan parameter support if your algorithm needs it

For questions or examples, see [ARCHITECTURE.md](ARCHITECTURE.md) and existing algorithms: [CaesarCipher.cpp](algorithms/CaesarCipher.cpp), [ROT13.cpp](algorithms/ROT13.cpp).
