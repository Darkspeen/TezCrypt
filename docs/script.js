/**
 * TezCrypt Web - Encryption Algorithms
 * 
 * This module implements encryption algorithms in JavaScript,
 * mirroring the desktop application's functionality.
 */

// === Encryption Algorithms ===

/**
 * Caesar Cipher - Shifts each letter by a fixed amount (3)
 */
class CaesarCipher {
    constructor() {
        this.shift = 3;
        this.name = "Caesar Cipher";
    }

    encrypt(plaintext) {
        return this._transform(plaintext, this.shift);
    }

    decrypt(ciphertext) {
        return this._transform(ciphertext, -this.shift);
    }

    _transform(text, shift) {
        let result = "";
        
        for (let char of text) {
            if (char >= 'A' && char <= 'Z') {
                let charCode = ((char.charCodeAt(0) - 65 + shift) % 26 + 26) % 26;
                result += String.fromCharCode(charCode + 65);
            } else if (char >= 'a' && char <= 'z') {
                let charCode = ((char.charCodeAt(0) - 97 + shift) % 26 + 26) % 26;
                result += String.fromCharCode(charCode + 97);
            } else {
                result += char;
            }
        }
        
        return result;
    }
}

/**
 * ROT13 - Rotation cipher by 13 positions (symmetric)
 */
class ROT13 {
    constructor() {
        this.shift = 13;
        this.name = "ROT13";
    }

    encrypt(plaintext) {
        return this._rotate(plaintext);
    }

    decrypt(ciphertext) {
        return this._rotate(ciphertext);
    }

    _rotate(text) {
        let result = "";
        
        for (let char of text) {
            if (char >= 'A' && char <= 'Z') {
                let charCode = (char.charCodeAt(0) - 65 + this.shift) % 26;
                result += String.fromCharCode(charCode + 65);
            } else if (char >= 'a' && char <= 'z') {
                let charCode = (char.charCodeAt(0) - 97 + this.shift) % 26;
                result += String.fromCharCode(charCode + 97);
            } else {
                result += char;
            }
        }
        
        return result;
    }
}

class SaltedFibonacciCipher {
    constructor() {
        this.name = "Salted Fibonacci Cipher";
    }

    encrypt(plaintext) {
        const salt = "tezcrypt";
        const encodedSalt = this._b64(salt);
        const encodedText = this._b64(plaintext);
        const hexIndex = this._hashHexIndex(salt);
        const fibonacciCount = this._computeFibonacciCount(encodedText.length, encodedSalt.length, parseInt(hexIndex, 16));
        const embedded = this._injectSalt(encodedText, encodedSalt, fibonacciCount, parseInt(hexIndex, 16));
        return embedded;
    }

    decrypt(ciphertext) {
        const hexIndex = this._hashHexIndex("tezcrypt");
        const fibonacciCount = this._computeFibonacciCount(ciphertext.length, 8, parseInt(hexIndex, 16));
        const base64Payload = this._removeSalt(ciphertext, fibonacciCount, parseInt(hexIndex, 16));
        try {
            return this._decodeBase64(base64Payload);
        } catch (error) {
            return ciphertext;
        }
    }

    _b64(text) {
        return btoa(unescape(encodeURIComponent(text)));
    }

    _decodeBase64(text) {
        return decodeURIComponent(escape(atob(text)));
    }

    _hashHexIndex(salt) {
        let hash = 0;
        for (let i = 0; i < salt.length; i++) {
            hash = (hash * 31 + salt.charCodeAt(i)) >>> 0;
        }
        return (hash % 16).toString(16).toUpperCase();
    }

    _computeFibonacciCount(baseLength, saltLength, insertIndex) {
        if (baseLength <= 0 || saltLength <= 0) {
            return 0;
        }

        const fib = [1, 1];
        let count = 0;
        let total = 0;
        while (count < saltLength) {
            const step = fib[count % fib.length];
            if (total + step > baseLength + saltLength) {
                break;
            }
            total += step;
            count += 1;
            if (fib.length <= count) {
                fib.push(fib[fib.length - 1] + fib[fib.length - 2]);
            }
        }

        return Math.min(count, saltLength);
    }

    _injectSalt(text, salt, fibonacciCount, insertIndex) {
        const totalLength = text.length + salt.length;
        const positions = this._buildInsertionPositions(text.length, salt.length, fibonacciCount, insertIndex);
        let result = "";
        let saltIndex = 0;
        let textIndex = 0;
        for (let i = 0; i < totalLength; i++) {
            if (saltIndex < positions.length && positions[saltIndex] === i) {
                result += salt[saltIndex];
                saltIndex += 1;
            } else {
                result += text[textIndex];
                textIndex += 1;
            }
        }
        return result;
    }

    _removeSalt(text, fibonacciCount, insertIndex) {
        const saltLength = Math.max(1, Math.floor(text.length / 2));
        const positions = this._buildInsertionPositions(text.length - saltLength, saltLength, fibonacciCount, insertIndex);
        let result = "";
        let saltIndex = 0;
        for (let i = 0; i < text.length; i++) {
            if (saltIndex < positions.length && positions[saltIndex] === i) {
                saltIndex += 1;
            } else {
                result += text[i];
            }
        }
        return result;
    }

    _buildInsertionPositions(baseLength, saltLength, fibonacciCount, insertIndex) {
        if (saltLength <= 0 || baseLength <= 0) {
            return [];
        }

        const fib = [1, 1];
        while (fib.length < Math.max(1, fibonacciCount)) {
            fib.push(fib[fib.length - 1] + fib[fib.length - 2]);
        }

        const totalLength = baseLength + saltLength;
        const start = Math.max(0, insertIndex % totalLength);
        const occupied = new Array(totalLength).fill(false);
        const positions = [];
        for (let i = 0; i < saltLength; i++) {
            const step = fib[Math.min(i, fib.length - 1)];
            let position = (start + i * step) % totalLength;
            while (occupied[position]) {
                position = (position + 1) % totalLength;
            }
            occupied[position] = true;
            positions.push(position);
        }

        return positions.sort((a, b) => a - b);
    }
}

/**
 * Algorithm Registry - Store and manage available algorithms
 */
class AlgorithmRegistry {
    constructor() {
        this.algorithms = {
            'caesar': new CaesarCipher(),
            'rot13': new ROT13(),
            'salted-fibonacci': new SaltedFibonacciCipher()
        };
    }

    getAlgorithm(id) {
        return this.algorithms[id] || null;
    }

    getAlgorithmNames() {
        return Object.values(this.algorithms).map(algo => algo.name);
    }

    getAllAlgorithms() {
        return this.algorithms;
    }
}

// === UI Controller ===

class TezCryptWeb {
    constructor() {
        this.registry = new AlgorithmRegistry();
        this.currentAlgorithmId = 'caesar';
        
        this.elements = {
            algorithm: document.getElementById('algorithm'),
            inputText: document.getElementById('inputText'),
            outputText: document.getElementById('outputText'),
            encryptBtn: document.getElementById('encryptBtn'),
            decryptBtn: document.getElementById('decryptBtn'),
            copyBtn: document.getElementById('copyBtn')
        };

        this.init();
    }

    init() {
        // Set up event listeners
        this.elements.algorithm.addEventListener('change', (e) => {
            this.currentAlgorithmId = e.target.value;
        });

        this.elements.encryptBtn.addEventListener('click', () => {
            this.performCrypto(true);
        });

        this.elements.decryptBtn.addEventListener('click', () => {
            this.performCrypto(false);
        });

        this.elements.copyBtn.addEventListener('click', () => {
            this.copyToClipboard();
        });

        // Allow Enter key in text areas for crypto operations
        this.elements.inputText.addEventListener('keydown', (e) => {
            if ((e.ctrlKey || e.metaKey) && e.key === 'Enter') {
                this.performCrypto(true);
            }
        });
    }

    performCrypto(isEncrypt) {
        const inputText = this.elements.inputText.value;

        if (!inputText.trim()) {
            this.elements.outputText.value = '';
            return;
        }

        const algorithm = this.registry.getAlgorithm(this.currentAlgorithmId);
        if (!algorithm) {
            this.elements.outputText.value = 'Error: Algorithm not found.';
            return;
        }

        try {
            const result = isEncrypt 
                ? algorithm.encrypt(inputText) 
                : algorithm.decrypt(inputText);
            
            this.elements.outputText.value = result;
        } catch (error) {
            this.elements.outputText.value = `Error: ${error.message}`;
            console.error('Encryption error:', error);
        }
    }

    copyToClipboard() {
        const outputText = this.elements.outputText.value;

        if (!outputText) {
            return;
        }

        navigator.clipboard.writeText(outputText)
            .then(() => {
                // Show temporary success message
                const btn = this.elements.copyBtn;
                const originalText = btn.textContent;
                btn.textContent = 'Copied!';
                btn.style.background = '#27ae60';

                setTimeout(() => {
                    btn.textContent = originalText;
                    btn.style.background = '';
                }, 2000);
            })
            .catch((err) => {
                console.error('Failed to copy:', err);
                alert('Failed to copy to clipboard');
            });
    }
}

// === Initialize Application ===

document.addEventListener('DOMContentLoaded', () => {
    new TezCryptWeb();
});

// Expose for testing (optional)
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { CaesarCipher, ROT13, AlgorithmRegistry };
}
