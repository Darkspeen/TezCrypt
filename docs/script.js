/**
 * TezCrypt Web - Encryption Algorithms
 * 
 * This module implements encryption algorithms in JavaScript,
 * mirroring the desktop application's functionality.
 */

// === Encryption Algorithms ===

const TAGGED_FORMAT_VERSION = 1;

/**
 * Caesar Cipher - Shifts each letter by a fixed amount (3)
 */
class CaesarCipher {
    constructor() {
        this.shift = 3;
        this.name = "Caesar Cipher";
    }

    metadataTag() {
        return "CSA";
    }

    parameterPlaceholder() {
        return "offset";
    }

    encrypt(plaintext) {
        return this._transform(plaintext, this.shift);
    }

    decrypt(ciphertext) {
        return this._transform(ciphertext, -this.shift);
    }

    encryptSegment(segment, params = []) {
        const parsed = Number.parseInt(params[0], 10);
        const shift = Number.isFinite(parsed) ? parsed : this.shift;
        return this._transform(segment, shift);
    }

    decryptSegment(segment, params = []) {
        const parsed = Number.parseInt(params[0], 10);
        const shift = Number.isFinite(parsed) ? parsed : this.shift;
        return this._transform(segment, -shift);
    }

    trailerParameter(_segment, params = []) {
        const parsed = Number.parseInt(params[0], 10);
        return String(Number.isFinite(parsed) ? parsed : this.shift);
    }

    trailerParameterCount() {
        return 1;
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

    metadataTag() {
        return "RSA";
    }

    parameterPlaceholder() {
        return "";
    }

    encrypt(plaintext) {
        return this._rotate(plaintext);
    }

    decrypt(ciphertext) {
        return this._rotate(ciphertext);
    }

    encryptSegment(segment) {
        return this.encrypt(segment);
    }

    decryptSegment(segment) {
        return this.decrypt(segment);
    }

    trailerParameterCount() {
        return 0;
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
        this.name = "XOR Key-Weaver";
    }

    metadataTag() {
        return "XFA";
    }

    parameterPlaceholder() {
        return "key";
    }

    async encrypt(plaintext) {
        const payload = await this._buildPayload(plaintext, "tezcrypt");
        return `1:${payload.start}:${payload.keyLength}:${payload.body}`;
    }

    async decrypt(ciphertext) {
        const legacyStandalone = /^(?:1|SF1):(\d+):(\d+):/.exec(ciphertext);
        if (legacyStandalone) {
            const payload = ciphertext.slice(legacyStandalone[0].length);
            return this._decryptEmbeddedAtStart(
                payload,
                Number(legacyStandalone[1]),
                Number(legacyStandalone[2])
            );
        }

        const standalone = /^1:(\d+):/.exec(ciphertext);
        if (standalone) {
            const payload = ciphertext.slice(standalone[0].length);
            return await this._decryptEmbedded(payload, Number(standalone[1])) ?? ciphertext;
        }

        return ciphertext;
    }

    async encryptSegment(segment, params = []) {
        if (!segment) {
            return "";
        }
        const payload = await this._buildPayload(segment, params[0] ?? "tezcrypt");
        return payload.body;
    }

    trailerParameterCount() {
        return 2;
    }

    async encryptTaggedSegment(segment, params = []) {
        if (!segment) {
            const keyBytes = new TextEncoder().encode(params[0] ?? "tezcrypt");
            const start = await this._startIndexForKey(keyBytes);
            return {
                ciphertext: "",
                trailerParams: `${TAGGED_FORMAT_VERSION},${start},${keyBytes.length * 2}`
            };
        }
        const payload = await this._buildPayload(segment, params[0] ?? "tezcrypt");
        return {
            ciphertext: payload.body,
            trailerParams: `${TAGGED_FORMAT_VERSION},${payload.start},${payload.keyLength}`
        };
    }

    async decryptSegment(segment, params = []) {
        if (params.length === 1 && /^\d+$/.test(params[0])) {
            const plaintext = await this._decryptEmbedded(segment, Number(params[0]));
            if (plaintext !== null) {
                return plaintext;
            }
        }
        if (params.length === 2 && /^\d+$/.test(params[0]) && /^\d+$/.test(params[1])) {
            return this._decryptEmbeddedAtStart(segment, Number(params[0]), Number(params[1]));
        }
        if (params.length === 3 && /^(?:1|SF1)$/.test(params[0])) {
            return this._decryptEmbeddedAtStart(segment, Number(params[1]), Number(params[2]));
        }
        return segment;
    }

    async _buildPayload(plaintext, key) {
        const keyBytes = new TextEncoder().encode(key);
        const plaintextBytes = new TextEncoder().encode(plaintext);
        const encryptedBytes = await this._xorWithKey(plaintextBytes, keyBytes);
        const encodedKey = this._toHex(keyBytes);
        const encodedText = this._toBase64(encryptedBytes);
        const start = await this._startIndexForKey(keyBytes);
        return {
            body: this._weaveKey(encodedText, encodedKey, start),
            start,
            keyLength: encodedKey.length
        };
    }

    async _startIndexForKey(keyBytes) {
        if (typeof crypto === "undefined" || !crypto.subtle) {
            throw new Error("Web Crypto is unavailable; use HTTPS or localhost.");
        }
        const digest = new Uint8Array(await crypto.subtle.digest("SHA-256", keyBytes));
        return digest[0] % 12;
    }

    async _decryptEmbedded(payload, keyLength) {
        if (!Number.isInteger(keyLength) || keyLength < 0
            || keyLength > payload.length || keyLength % 2 !== 0) {
            return payload;
        }

        for (let start = 0; start < 12; start++) {
            const plaintext = await this._decryptEmbeddedAtStart(payload, start, keyLength, true);
            if (plaintext !== null) {
                return plaintext;
            }
        }
        return null;
    }

    async _decryptEmbeddedAtStart(payload, start, keyLength, verifyStart = false) {
        if (!Number.isInteger(start) || !Number.isInteger(keyLength)
            || start < 0 || start >= 12 || keyLength < 0 || keyLength > payload.length
            || keyLength % 2 !== 0) {
            return verifyStart ? null : payload;
        }

        const [encodedText, encodedKey] = this._unweaveKey(payload, start, keyLength);
        if (!/^(?:[0-9a-f]{2})*$/i.test(encodedKey)) {
            return verifyStart ? null : payload;
        }

        const keyBytes = this._fromHex(encodedKey);
        const encryptedBytes = this._fromBase64(encodedText);
        if (!keyBytes || !encryptedBytes
            || (verifyStart && await this._startIndexForKey(keyBytes) !== start)) {
            return verifyStart ? null : payload;
        }
        try {
            return new TextDecoder("utf-8", { fatal: true }).decode(
                await this._xorWithKey(encryptedBytes, keyBytes)
            );
        } catch (error) {
            if (error instanceof TypeError && verifyStart) {
                return null;
            }
            throw error;
        }
    }

    async _xorWithKey(input, key) {
        if (typeof crypto === "undefined" || !crypto.subtle) {
            throw new Error("Web Crypto is unavailable; use HTTPS or localhost.");
        }
        let state = new Uint8Array(await crypto.subtle.digest("SHA-256", key));
        const output = new Uint8Array(input.length);
        const encoder = new TextEncoder();
        for (let i = 0; i < input.length; i++) {
            const counter = encoder.encode(String(i));
            const combined = new Uint8Array(key.length + counter.length + state.length);
            combined.set(key);
            combined.set(counter, key.length);
            combined.set(state, key.length + counter.length);
            state = new Uint8Array(await crypto.subtle.digest("SHA-256", combined));
            output[i] = input[i] ^ state[0];
        }
        return output;
    }

    _buildKeyPositions(baseLength, keyLength, start) {
        const positions = [];
        let basePosition = Math.min(start, baseLength);
        let firstJump = 1;
        let secondJump = 1;
        const jumpLimit = Math.max(1, baseLength);
        for (let i = 0; i < keyLength; i++) {
            positions.push(basePosition + i);
            const jump = firstJump;
            firstJump = secondJump;
            secondJump = Math.min(jumpLimit, firstJump + secondJump);
            basePosition = Math.min(baseLength, basePosition + jump);
        }
        return positions;
    }

    _weaveKey(text, key, start) {
        const positions = this._buildKeyPositions(text.length, key.length, start);
        let result = "";
        let keyIndex = 0;
        let textIndex = 0;
        for (let i = 0; i < text.length + key.length; i++) {
            if (keyIndex < positions.length && positions[keyIndex] === i) {
                result += key[keyIndex++];
            } else {
                result += text[textIndex++];
            }
        }
        return result;
    }

    _unweaveKey(payload, start, keyLength) {
        const baseLength = payload.length - keyLength;
        const positions = this._buildKeyPositions(baseLength, keyLength, start);
        let text = "";
        let key = "";
        let keyIndex = 0;
        for (let i = 0; i < payload.length; i++) {
            if (keyIndex < positions.length && positions[keyIndex] === i) {
                key += payload[i];
                keyIndex++;
            } else {
                text += payload[i];
            }
        }
        return [text, key];
    }

    _toHex(bytes) {
        return Array.from(bytes, byte => byte.toString(16).padStart(2, "0")).join("");
    }

    _fromHex(hex) {
        const bytes = new Uint8Array(hex.length / 2);
        for (let i = 0; i < bytes.length; i++) {
            bytes[i] = Number.parseInt(hex.slice(i * 2, i * 2 + 2), 16);
        }
        return bytes;
    }

    _toBase64(bytes) {
        let binary = "";
        for (let i = 0; i < bytes.length; i += 0x8000) {
            binary += String.fromCharCode(...bytes.subarray(i, i + 0x8000));
        }
        return btoa(binary);
    }

    _fromBase64(text) {
        try {
            const binary = atob(text);
            return Uint8Array.from(binary, char => char.charCodeAt(0));
        } catch {
            return null;
        }
    }
}

class WeaverSimpleA {
    constructor() {
        this.name = "Weaver";
    }

    metadataTag() {
        return "WSA";
    }

    parameterPlaceholder() {
        return "";
    }

    trailerParameterCount() {
        return 0;
    }

    encrypt(plaintext) {
        return this.encryptSegment(plaintext);
    }

    decrypt(ciphertext) {
        return this.decryptSegment(ciphertext);
    }

    encryptSegment(segment) {
        if (segment.length <= 1) {
            return segment;
        }
        const split = Math.floor(segment.length / 2);
        const firstHalf = segment.slice(0, split);
        const secondHalf = segment.slice(split).split("").reverse().join("");
        let output = "";
        for (let i = 0; i < Math.max(firstHalf.length, secondHalf.length); i++) {
            if (i < firstHalf.length) output += firstHalf[i];
            if (i < secondHalf.length) output += secondHalf[i];
        }
        return output;
    }

    decryptSegment(segment) {
        if (segment.length <= 1) {
            return segment;
        }
        const firstSize = Math.floor(segment.length / 2);
        const secondSize = segment.length - firstSize;
        let first = "";
        let reversedSecond = "";
        let index = 0;
        while (index < segment.length) {
            if (first.length < firstSize) first += segment[index++];
            if (index < segment.length && reversedSecond.length < secondSize) {
                reversedSecond += segment[index++];
            }
        }
        return first + reversedSecond.split("").reverse().join("");
    }
}

function decryptVersionedSegment(algorithm, segment, trailerParams) {
    const expectedCount = typeof algorithm.trailerParameterCount === "function"
        ? algorithm.trailerParameterCount()
        : 0;
    let params = trailerParams;
    if (params.length === expectedCount + 1 && /^\d+$/.test(params[0])) {
        if (Number(params[0]) !== TAGGED_FORMAT_VERSION) {
            return segment;
        }
        params = params.slice(1);
    }
    return typeof algorithm.decryptSegment === "function"
        ? algorithm.decryptSegment(segment, params)
        : algorithm.decrypt(segment);
}

/**
 * Algorithm Registry - Store and manage available algorithms
 */
class AlgorithmRegistry {
    constructor() {
        this.algorithms = {
            'caesar': new CaesarCipher(),
            'rot13': new ROT13(),
            'weaver-simple-a': new WeaverSimpleA(),
            'xor-key-weaver': new SaltedFibonacciCipher()
        };
        this.tagMap = this.buildTagMap();
    }

    buildTagMap() {
        return Object.values(this.algorithms).reduce((map, algo) => {
            if (algo && typeof algo.metadataTag === 'function') {
                const tag = algo.metadataTag();
                if (tag) {
                    map[tag] = algo;
                }
                if (typeof algo.legacyMetadataTags === "function") {
                    for (const legacyTag of algo.legacyMetadataTags()) {
                        map[legacyTag] = algo;
                    }
                }
            }
            return map;
        }, {});
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

    getAlgorithmByTag(tag) {
        return this.tagMap[tag] || null;
    }
}

// === UI Controller ===

class TezCryptWeb {
    constructor() {
        this.registry = new AlgorithmRegistry();
        this.currentAlgorithmId = 'caesar';
        
        this.elements = {
            algorithm: document.getElementById('algorithm'),
            subAlgorithm: document.getElementById('subAlgorithm'),
            variant: document.getElementById('variant'),
            wrapBtn: document.getElementById('wrapBtn'),
            uploadBtn: document.getElementById('uploadBtn'),
            fileInput: document.getElementById('fileInput'),
            inputText: document.getElementById('inputText'),
            outputText: document.getElementById('outputText'),
            encryptBtn: document.getElementById('encryptBtn'),
            decryptBtn: document.getElementById('decryptBtn'),
            copyBtn: document.getElementById('copyBtn')
        };

        this.init();
    }

    init() {
        this.populateAlgorithmOptions();

        this.elements.algorithm.addEventListener('change', (e) => {
            this.currentAlgorithmId = e.target.value;
            this.populateTagOptions();
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

        this.elements.wrapBtn.addEventListener('click', () => {
            this.wrapTag();
        });

        this.elements.uploadBtn.addEventListener('click', () => {
            this.elements.fileInput.click();
        });

        this.elements.fileInput.addEventListener('change', (event) => {
            const files = event.target.files;
            if (files && files.length > 0) {
                this.loadFile(files[0]);
            }
            event.target.value = '';
        });

        this.elements.inputText.addEventListener('keydown', (e) => {
            if ((e.ctrlKey || e.metaKey) && e.key === 'Enter') {
                this.performCrypto(true);
            }
        });
    }

    populateAlgorithmOptions() {
        const algorithmSelect = this.elements.algorithm;
        algorithmSelect.innerHTML = '';

        const algorithms = this.registry.getAllAlgorithms();
        Object.keys(algorithms).forEach((algorithmId) => {
            const algorithm = algorithms[algorithmId];
            if (!algorithm) {
                return;
            }

            const option = document.createElement('option');
            option.value = algorithmId;
            option.textContent = algorithm.name;
            algorithmSelect.appendChild(option);
        });

        if (algorithmSelect.options.length > 0) {
            this.currentAlgorithmId = algorithmSelect.options[0].value;
        }
        this.populateTagOptions();
    }

    populateTagOptions() {
        const algorithm = this.registry.getAlgorithm(this.currentAlgorithmId);
        const tag = algorithm && typeof algorithm.metadataTag === "function"
            ? algorithm.metadataTag()
            : "";
        const subAlgorithmLabels = { S: "Simple", F: "Fibonacci", X: "XOR" };

        this.elements.subAlgorithm.innerHTML = "";
        this.elements.variant.innerHTML = "";
        if (tag.length < 3) {
            return;
        }

        const subAlgorithmCode = tag[1];
        const subAlgorithmOption = document.createElement("option");
        subAlgorithmOption.value = subAlgorithmCode;
        subAlgorithmOption.textContent = subAlgorithmLabels[subAlgorithmCode] || subAlgorithmCode;
        this.elements.subAlgorithm.appendChild(subAlgorithmOption);

        const variantOption = document.createElement("option");
        variantOption.value = tag[2];
        variantOption.textContent = tag[2];
        this.elements.variant.appendChild(variantOption);
    }

    async performCrypto(isEncrypt) {
        const inputText = this.elements.inputText.value;

        if (!inputText.trim()) {
            this.elements.outputText.value = '';
            return;
        }

        try {
            if (isEncrypt && this.isTaggedInput(inputText)) {
                this.elements.outputText.value = await this.encryptTaggedText(inputText);
                return;
            }

            if (!isEncrypt && this.hasTrailerTags(inputText)) {
                this.elements.outputText.value = await this.decryptTaggedText(inputText);
                return;
            }

            const algorithm = this.registry.getAlgorithm(this.currentAlgorithmId);
            if (!algorithm) {
                this.elements.outputText.value = 'Error: Algorithm not found.';
                return;
            }

            this.elements.outputText.value = await (isEncrypt
                ? algorithm.encrypt(inputText)
                : algorithm.decrypt(inputText));
        } catch (error) {
            this.elements.outputText.value = `Error: ${error.message}`;
            console.error('Encryption error:', error);
        }
    }

    wrapTag() {
        const algorithm = this.registry.getAlgorithm(this.currentAlgorithmId);
        if (!algorithm) {
            this.elements.outputText.value = 'Error: Algorithm not found.';
            return;
        }

        const algorithmTag = typeof algorithm.metadataTag === 'function' ? algorithm.metadataTag() : '';
        const params = typeof algorithm.parameterPlaceholder === 'function' ? algorithm.parameterPlaceholder() : '';

        if (!algorithmTag) {
            alert('This algorithm does not support wrapper tags.');
            return;
        }

        const prefix = params ? `|#${algorithmTag}:${params}|` : `|#${algorithmTag}|`;
        const suffix = '|#|';
        const textarea = this.elements.inputText;
        const { selectionStart, selectionEnd, value } = textarea;
        const selectedText = selectionStart !== selectionEnd
            ? value.slice(selectionStart, selectionEnd)
            : 'TEXT_TO_ENCRYPT';

        const before = value.slice(0, selectionStart);
        const after = value.slice(selectionEnd);
        textarea.value = `${before}${prefix}${selectedText}${suffix}${after}`;

        const cursorPosition = before.length + prefix.length + selectedText.length;
        textarea.setSelectionRange(cursorPosition, cursorPosition);
        textarea.focus();
    }

    loadFile(file) {
        const reader = new FileReader();
        reader.onload = () => {
            this.elements.inputText.value = reader.result || '';
        };
        reader.onerror = () => {
            alert('Could not load file.');
        };
        reader.readAsText(file, 'utf-8');
    }

    copyToClipboard() {
        const outputText = this.elements.outputText.value;
        if (!outputText) {
            return;
        }

        navigator.clipboard.writeText(outputText)
            .then(() => {
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

    isTaggedInput(text) {
        return /\|#[A-Za-z0-9]{2,8}(?::[^|]*)?\|/.test(text);
    }

    hasTrailerTags(text) {
        return /(?:\|\|x[0-9A-Fa-f]{6}x[0-9A-Fa-f]{6}#[A-Za-z0-9]{2,8}(?::[^|]*)?\|\|)+$/.test(text);
    }

    async encryptTaggedText(input) {
        const openTagRegex = /(?:\|\|#|\|#)([A-Za-z0-9]{2,8})(?::([^|]*))?(?:\|\||\|)/g;
        const closingTag = '|#|';
        let cursor = 0;
        let output = '';
        const ranges = [];

        while (cursor < input.length) {
            openTagRegex.lastIndex = cursor;
            const match = openTagRegex.exec(input);
            if (!match) {
                output += input.slice(cursor);
                break;
            }

            if (match.index > cursor) {
                output += input.slice(cursor, match.index);
            }

            const algorithmTag = match[1];
            const params = match[2] ? match[2].split(',') : [];
            const segmentStart = match.index + match[0].length;
            let closeIndex = input.indexOf(closingTag, segmentStart);
            let closeLength = closingTag.length;
            if (closeIndex < 0) {
                closeIndex = input.indexOf('||#||', segmentStart);
                closeLength = 5;
            }
            if (closeIndex < 0) {
                output += input.slice(match.index);
                break;
            }

            const segment = input.slice(segmentStart, closeIndex);
            const algorithm = this.registry.getAlgorithmByTag(algorithmTag);
            if (!algorithm) {
                output += input.slice(match.index, closeIndex + closeLength);
                cursor = closeIndex + closeLength;
                continue;
            }

            let encryptedSegment;
            let trailerParams;
            if (typeof algorithm.encryptTaggedSegment === "function") {
                const encrypted = await algorithm.encryptTaggedSegment(segment, params);
                encryptedSegment = encrypted.ciphertext;
                trailerParams = encrypted.trailerParams
                    ? encrypted.trailerParams.split(',')
                    : [];
            } else {
                encryptedSegment = typeof algorithm.encryptSegment === "function"
                    ? await algorithm.encryptSegment(segment, params)
                    : await algorithm.encrypt(segment);
                const algorithmParams = typeof algorithm.trailerParameter === "function"
                    ? algorithm.trailerParameter(segment, params)
                    : params.join(',');
                trailerParams = [String(TAGGED_FORMAT_VERSION)];
                if (algorithmParams) trailerParams.push(...algorithmParams.split(','));
            }
            const start = output.length;
            output += encryptedSegment;
            const end = output.length;
            ranges.push({ start, end, algorithmTag, params: trailerParams });
            cursor = closeIndex + closeLength;
        }

        if (ranges.length === 0) {
            return input;
        }

        for (const range of ranges) {
            const startHex = range.start.toString(16).padStart(6, '0').toUpperCase();
            const endHex = range.end.toString(16).padStart(6, '0').toUpperCase();
            const trailerParams = range.params.length ? `:${range.params.join(',')}` : '';
            output += `||x${startHex}x${endHex}#${range.algorithmTag}${trailerParams}||`;
        }

        return output;
    }

    async decryptTaggedText(input) {
        const trailerRegex = /^(?:\|\|)?x([0-9A-Fa-f]{6})x([0-9A-Fa-f]{6})#([A-Za-z0-9]{2,8})(?::([^|]*))?(?:\|\|)?$/;
        let body = input;
        const ranges = [];

        while (true) {
            let trailerStart = body.lastIndexOf('||x');
            if (trailerStart < 0) {
                trailerStart = body.lastIndexOf('|x');
            }
            if (trailerStart < 0) {
                break;
            }

            const trailerText = body.slice(trailerStart);
            const match = trailerRegex.exec(trailerText);
            if (!match) {
                break;
            }

            const start = parseInt(match[1], 16);
            const end = parseInt(match[2], 16);
            const algorithmTag = match[3];
            const params = match[4] ? match[4].split(',') : [];

            if (Number.isNaN(start) || Number.isNaN(end)) {
                break;
            }

            ranges.unshift({ start, end, algorithmTag, params });
            body = body.slice(0, trailerStart);
        }

        if (ranges.length === 0) {
            return input;
        }

        let result = body;
        for (let i = ranges.length - 1; i >= 0; i--) {
            const range = ranges[i];
            if (range.start < 0 || range.end > result.length || range.start >= range.end) {
                continue;
            }

            const algorithm = this.registry.getAlgorithmByTag(range.algorithmTag);
            if (!algorithm) {
                continue;
            }

            const segment = result.slice(range.start, range.end);
            const decryptedSegment = await decryptVersionedSegment(algorithm, segment, range.params);
            result = result.slice(0, range.start) + decryptedSegment + result.slice(range.end);
        }

        return result;
    }
}

// === Initialize Application ===

document.addEventListener('DOMContentLoaded', () => {
    new TezCryptWeb();
});

// Expose for testing (optional)
if (typeof module !== 'undefined' && module.exports) {
    module.exports = {
        CaesarCipher,
        ROT13,
        WeaverSimpleA,
        SaltedFibonacciCipher,
        AlgorithmRegistry,
        TezCryptWeb,
        TAGGED_FORMAT_VERSION
    };
}
