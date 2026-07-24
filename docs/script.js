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
        // ROT13 is symmetric - encryption and decryption are the same
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

/**
 * Algorithm Registry - Store and manage available algorithms
 */
class AlgorithmRegistry {
    constructor() {
        this.algorithms = {
            'caesar': new CaesarCipher(),
            'rot13': new ROT13()
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
