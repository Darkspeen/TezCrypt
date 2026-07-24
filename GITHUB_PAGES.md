# GitHub Pages - Host TezCrypt Web Version

This guide walks you through hosting your TezCrypt web application on GitHub Pages.

## What You'll Get

A shareable link like: `https://yourusername.github.io/tezcrypt`

Anyone can access the encryption tool from this URL without installing anything!

---

## Step 1: Create a GitHub Repository

### Option A: If you already have a GitHub account

1. Go to [github.com/new](https://github.com/new)
2. Name it: `tezcrypt`
3. Add description: "Modular Encryption/Decryption Utility"
4. Choose Public (so GitHub Pages works)
5. Click "Create repository"
6. Do NOT initialize with README yet

### Option B: If you don't have a GitHub account yet

1. Go to [github.com/signup](https://github.com/signup)
2. Create account (takes 2 minutes)
3. Verify email
4. Come back to Step 1 Option A

---

## Step 2: Push Your Code to GitHub

### 2.1 Initialize Git (if not already done)

```bash
cd /home/donagan/Repositories/TezCrypt

# Check if git is initialized
git status

# If NOT initialized, run:
git init
git branch -M main
```

### 2.2 Add GitHub as Remote

```bash
# Replace YOUR_USERNAME with your actual GitHub username
git remote add origin https://github.com/YOUR_USERNAME/tezcrypt.git
```

### 2.3 Configure Git (if first time)

```bash
git config --global user.name "Your Name"
git config --global user.email "your.email@example.com"
```

### 2.4 Commit and Push Everything

```bash
cd /home/donagan/Repositories/TezCrypt

# Stage all files
git add .

# Commit with a message
git commit -m "Initial commit: Complete TezCrypt desktop and web versions"

# Push to GitHub (creates main branch if needed)
git push -u origin main
```

**First time pushing?** You might need to authenticate:
- GitHub will prompt for Personal Access Token (PAT)
- Create one at: https://github.com/settings/tokens/new
  - Give it `repo` scope
  - Copy and paste when prompted

---

## Step 3: Enable GitHub Pages

### 3.1 Go to Repository Settings

1. Go to your repo: `github.com/YOUR_USERNAME/tezcrypt`
2. Click **Settings** (top right)
3. Click **Pages** (left sidebar)

### 3.2 Configure GitHub Pages

Under "Build and deployment":

1. **Source**: Select `Deploy from a branch`
2. **Branch**: Select `main`
3. **Folder**: Select `/docs`
4. Click **Save**

GitHub will automatically build and deploy your site!

### 3.3 Wait for Deployment

- Green checkmark appears on GitHub Actions (usually 1-2 minutes)
- Your site is now live at: `https://YOUR_USERNAME.github.io/tezcrypt`

You can see status in the **Environments** section on the right side of your repo.

---

## Step 4: Update Links (Optional)

The web version has placeholder GitHub links. Update them:

### In `/docs/index.html`

Find this line:
```html
<p><a href="https://github.com/yourusername/tezcrypt" target="_blank">View on GitHub</a> | <a href="https://github.com/yourusername/tezcrypt#readme" target="_blank">Documentation</a></p>
```

Replace `yourusername` with your actual GitHub username.

Then commit and push:
```bash
git add docs/index.html
git commit -m "Update GitHub links"
git push
```

---

## Step 5: Share Your Link

Your web app is now live! 🎉

Share this URL with friends:
```
https://YOUR_USERNAME.github.io/tezcrypt
```

They can:
- ✅ Use Caesar Cipher and ROT13 encryption
- ✅ Encrypt/decrypt text
- ✅ Copy results
- ✅ No installation required
- ✅ Works on any device with a browser

---

## Updating Your Web Version

When you update the web files (`docs/index.html`, `docs/script.js`, `docs/style.css`):

```bash
cd /home/donagan/Repositories/TezCrypt

# Make your changes to files in /docs/

# Commit and push
git add docs/
git commit -m "Update web UI: [describe changes]"
git push
```

GitHub automatically redeploys within 1-2 minutes!

---

## Troubleshooting

### Site not appearing (404 error)

**Check:**
1. Is the repo public? (Settings → Visibility)
2. Have you waited 2+ minutes after enabling Pages?
3. Is `/docs` folder selected in Pages settings?
4. Does `/docs/index.html` exist?

Try:
```bash
# View build logs on GitHub
# Settings → Pages → Click your deployment
```

### Site appears but assets (CSS/JS) not loading

**Likely cause:** GitHub Pages settings showing `/root` instead of `/docs`

**Fix:**
1. Go to Settings → Pages
2. Source: `Deploy from a branch`
3. Branch: `main`
4. Folder: `/docs` (not `/root`)
5. Save
6. Wait 2 minutes

### Caching issues

GitHub Pages caches aggressively. If changes don't show:

```bash
# Hard refresh in browser
Ctrl+Shift+R (Windows/Linux)
Cmd+Shift+R (macOS)
```

Or add a cache buster to URLs (for developers - not needed normally).

---

## Custom Domain (Advanced - Optional)

Want to use `tezcrypt.com` instead of `github.io`?

1. Buy a domain (Google Domains, Namecheap, etc.)
2. GitHub Pages → Settings → Custom domain
3. Add your domain name
4. Update DNS records (GitHub shows instructions)

Takes 10-30 minutes for DNS to propagate.

---

## GitHub Pages Features You Can Use

### Add a README

Create `/docs/README.md`:
```bash
cat > /home/donagan/Repositories/TezCrypt/docs/README.md << 'EOF'
# TezCrypt Web

Free, open-source encryption tool running in your browser!

## Features
- Caesar Cipher
- ROT13 
- Fast, client-side encryption
- No data sent to servers
- Works offline

## Usage

Access at: https://yourusername.github.io/tezcrypt

## Desktop Version

Want the full-featured desktop app? See the main repository.
EOF
```

### Track Visitors

Add Google Analytics (optional):
1. Create account at [analytics.google.com](https://analytics.google.com)
2. Add to HTML `<head>`:
```html
<script async src="https://www.googletagmanager.com/gtag/js?id=G-XXXXX"></script>
<script>
  window.dataLayer = window.dataLayer || [];
  function gtag(){dataLayer.push(arguments);}
  gtag('js', new Date());
  gtag('config', 'G-XXXXX');
</script>
```

---

## Directory Structure on GitHub

```
tezcrypt/
├── src/                    ← Desktop C++ code
├── algorithms/
├── core/
├── gui/
├── CMakeLists.txt
├── README.md              ← Main project README
├── BUILDING.md
├── ARCHITECTURE.md
├── EXTENDING.md
├── docs/                  ← GitHub Pages web app
│   ├── index.html        ← Entry point
│   ├── style.css
│   ├── script.js
│   ├── .nojekyll
│   └── README.md         ← Web version info
└── ... (other files)
```

---

## What Happens Behind the Scenes

1. You push to `main` branch
2. GitHub Actions builds your site from `/docs`
3. Generated static files served from GitHub's CDN
4. Takes 1-2 minutes usually
5. Always available at your `github.io` URL

No server to maintain. No costs. Automatic HTTPS. Automatic backups.

---

## Next Steps

1. ✅ Create GitHub account (if needed)
2. ✅ Create `tezcrypt` repository  
3. ✅ Push your code
4. ✅ Enable GitHub Pages from `/docs`
5. ✅ Wait 2 minutes
6. ✅ Visit `https://yourusername.github.io/tezcrypt`
7. ✅ Share with friends!

---

## Tips for Sharing

### Share as QR Code

Use a QR code generator to create a scannable code:
1. Go to [qr-server.com](https://qr-server.com)
2. Enter your URL: `https://yourusername.github.io/tezcrypt`
3. Generate and download QR code
4. Share as image

### Shorten the URL (Optional)

Services like [bit.ly](https://bitly.com) or GitHub's built-in shortener let you create shorter links.

### Add to Portfolio

Add this to your resume or portfolio:
```
Live Demo: https://YOUR_USERNAME.github.io/tezcrypt
GitHub: https://github.com/YOUR_USERNAME/tezcrypt
```

### Share on Social Media

> "I built TezCrypt - an open-source encryption tool you can use in your browser! No installation needed. Check it out: [link]"

---

## Security Note

The web version runs **100% in your browser**:
- ✅ Text never sent to servers
- ✅ No cookies or tracking
- ✅ Works offline (after first load)
- ✅ Open source code (view on GitHub)

**Still development-grade** - Caesar and ROT13 are not cryptographically secure. For real data protection, use production algorithms.

---

## Next Enhancement Ideas

Once GitHub Pages is working, consider:

1. **Add more algorithms**: Vigenere, Atbash, Simple substitution
2. **Add Web Crypto API**: AES-GCM for real security
3. **Add file encryption**: Encrypt/decrypt files in browser
4. **Add dark mode toggle**: (CSS already supports it!)
5. **Add algorithm info**: Show description of each algorithm
6. **Add sharing**: Generate encrypted message with embedded algorithm choice
7. **Add settings**: Let users configure Caesar shift amount

All without touching the desktop app code!

---

## References

- [GitHub Pages Documentation](https://docs.github.com/en/pages)
- [GitHub Pages Site Started](https://docs.github.com/en/pages/getting-started-with-github-pages)
- [GitHub Actions](https://docs.github.com/en/actions)

---

**Your TezCrypt web version is ready to share with the world!** 🌍🔐
