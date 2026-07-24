# Web Version Quick Test

Want to test the web version locally before pushing to GitHub?

## Option 1: Simple Python Server (Recommended)

```bash
cd /home/donagan/Repositories/TezCrypt/docs
python3 -m http.server 8000
```

Then open: http://localhost:8000

Press `Ctrl+C` to stop the server.

## Option 2: Using Node.js (if installed)

```bash
cd /home/donagan/Repositories/TezCrypt/docs
npx http-server
```

Or install globally:
```bash
npm install -g http-server
http-server
```

## Option 3: VS Code Live Server

If using VS Code:
1. Install "Live Server" extension
2. Right-click on `docs/index.html`
3. Select "Open with Live Server"
4. Browser opens automatically

## Testing Checklist

- [ ] Caesar Cipher encrypts "hello" → "khoor"
- [ ] Caesar decrypts "khoor" → "hello"
- [ ] ROT13 encrypts "hello" → "uryyb"
- [ ] ROT13 decrypts "uryyb" → "hello"
- [ ] Copy button works
- [ ] Switching algorithms updates selection
- [ ] Mobile view is responsive (shrink browser)
- [ ] Works on different devices

## Dark Mode Testing

Open DevTools (F12) and check `Preferences → Dark theme` to test dark mode compatibility.

---

Once tested locally, follow [GITHUB_PAGES.md](GITHUB_PAGES.md) to deploy! 🚀
