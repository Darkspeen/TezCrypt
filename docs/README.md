# TezCrypt Web - Browser-Based Encryption

Fast, simple encryption tool that runs 100% in your browser.

## Features

✨ **Instant Encryption** - Caesar Cipher and ROT13 right in your browser
🔒 **Private** - All processing happens locally, nothing sent to servers  
📱 **Mobile Friendly** - Works on phones, tablets, and desktop
🌓 **Dark Mode** - Automatically matches your system theme
⚡ **No Installation** - Just open in a browser

## How To Use

1. Select an encryption algorithm from the dropdown
2. Enter text in the "Input Text" box
3. Click **Encrypt** to encrypt or **Decrypt** to decrypt
4. Copy the result to your clipboard with the **Copy Result** button

## Algorithms

### Caesar Cipher
A classic substitution cipher that shifts each letter by 3 positions.

**Example:**
- Input: `hello`
- Encrypted: `khoor`

### ROT13
A rotation cipher by 13 positions. Encoding and decoding are identical!

**Example:**
- Input: `hello`  
- Encrypted: `uryyb`

## Technical Details

- **Language:** Vanilla JavaScript (no dependencies!)
- **Data Privacy:** All encryption happens in your browser
- **Open Source:** [View code on GitHub](https://github.com/yourusername/tezcrypt)
- **Performance:** Handles text of any size instantly

## Desktop Version

Want the full-featured desktop application? Get it here:
- **Source Code:** https://github.com/yourusername/tezcrypt
- **Building:** See `BUILDING.md` for platform-specific instructions
- **Documentation:** Complete architecture and extension guides included

## Limitations

⚠️ **Caesar and ROT13 are demonstration ciphers only.** They are trivially breakable and should NOT be used to protect real data.

For production encryption, use:
- **AES-256** (via Web Crypto API)
- **OpenSSL** (desktop)
- **libsodium** (modern cryptography)

## Features Coming Soon

- [ ] AES encryption via Web Crypto API
- [ ] Vigenere cipher with custom key
- [ ] File encryption/decryption
- [ ] Password-protected encryption
- [ ] Shareable encrypted messages

## Feedback

Found a bug? Have a feature request? Submit an issue:
https://github.com/yourusername/tezcrypt/issues

## License

This project is open source and free to use.

---

**Made with ❤️ using vanilla JavaScript**

Built by the same team behind the desktop TezCrypt application.
