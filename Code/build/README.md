# ArborOS Build System

Build ArborOS ISO from Windows without VirtualBox.

## Quick Start (Windows)

**1. Install Docker Desktop**
- Download: https://www.docker.com/products/docker-desktop/
- Install and enable WSL2
- Start Docker Desktop

**2. Build ISO**

**Option A: Double-click (Easiest)**
```
Code/build/build-docker.bat
```
Double-click file, wait 15-30 min, get `ArborOS-phase3.iso`

**Option B: Git Bash / WSL**
```bash
cd Code/build
bash build-docker.sh
```

**3. Test ISO**
- VirtualBox > New VM
- Load `ArborOS-phase3.iso`
- RAM: 2GB+
- Boot and test

## Build Methods Comparison

| Method | Speed | Disk | Setup | Best For |
|--------|-------|------|-------|----------|
| **Docker** (NEW) | ⚡ Fast | Low (temp) | Easy | **Development** |
| VirtualBox | 🐌 Slow | High (VM) | Complex | One-time |
| Native Linux | ⚡ Fast | Medium | Manual | Production |

**Docker Wins:**
- ✅ Build directly from Windows
- ✅ No VM disk space issues
- ✅ Faster (no VM overhead)
- ✅ Auto-cleanup (temp files deleted)
- ✅ One command to build
- ✅ Great for design iteration (theme, apps, etc)

## Build Time

**Phases:**
1. Download Fedora packages: 3-5 min
2. Install packages: 3-10 min
3. Create initramfs: 1-2 min
4. Build squashfs: 5-10 min
5. Create ISO: 1-2 min

**Total: 15-30 minutes**
(varies by internet speed and CPU)

## Output

```
ArborOS/
└── ArborOS-phase3.iso    # 800-900 MB
```

ISO contains:
- Fedora 39 base
- Hardware support (GPU, audio, WiFi)
- arbor-hwinfo tool
- Live boot system

## Requirements

**Windows:**
- Docker Desktop (with WSL2)
- 4GB RAM free
- 10GB disk space (temp)

**Linux (alternative):**
- Fedora/RHEL/CentOS
- Root access
- `dnf`, `mkisofs`, `squashfs-tools`

## Customization

**For Phase 5+ (Desktop, Theme):**

1. Edit packages in build script
2. Add theme files to `Code/configs/`
3. Rebuild with Docker (fast iteration)
4. Test changes immediately

**Example: Add package**
```bash
# Edit build-docker.sh
# Add to package list:
dnf install ... \
    your-package-name \
    another-package
```

**Example: Add theme**
```bash
# In build script, after system config:
cp -r /workspace/Code/configs/theme/* \
    /tmp/build/work/rootfs/usr/share/themes/
```

## Troubleshooting

**Docker not starting:**
```
Error: Cannot connect to Docker daemon
Fix: Start Docker Desktop, wait for green icon
```

**Build fails - disk space:**
```
Error: No space left
Fix: Docker uses temp space, auto-cleans after build
Check: Docker Desktop > Settings > Resources > Disk
```

**ISO not booting:**
```
Check: VirtualBox settings
- RAM: 2GB minimum
- Boot order: Optical first
- UEFI: Disabled (use BIOS)
```

**Permission denied (Windows):**
```
Error: Permission denied accessing /workspace
Fix: Docker Desktop > Settings > Resources > File Sharing
Add: C:\Users\YourName\... to shared paths
```

## Advanced

**Build in background:**
```bash
# Git Bash / WSL
nohup bash build-docker.sh > build.log 2>&1 &
tail -f build.log
```

**Custom ISO name:**
```bash
# Edit build-docker.sh
# Change output filename:
cp /tmp/ArborOS-phase3.iso /workspace/ArborOS-custom.iso
```

**Debug build:**
```bash
# Enter container interactively
docker run --rm -it --privileged \
    -v "$(pwd)/../..:/workspace" \
    fedora:39 bash

# Run commands manually
cd /tmp
dnf install genisoimage ...
# etc
```

## Files

```
Code/build/
├── build-docker.bat       # Windows: Double-click to build
├── build-docker.sh        # Linux/WSL: Bash script
├── create_iso.sh          # Legacy: VirtualBox method
└── README.md              # This file
```

## Why Docker?

**Before (VirtualBox):**
```
1. Start Ubuntu VM
2. Login
3. Run build script
4. Wait 30+ min
5. Copy ISO to Windows
6. Shutdown VM
Total: 40+ min + VM disk space
```

**After (Docker):**
```
1. Double-click .bat
2. Wait 15-30 min
3. ISO ready in same folder
Total: 15-30 min, no VM needed
```

**For design work (Phase 5+):**
- Change theme files
- Rebuild in Docker (15 min)
- Test immediately
- Iterate fast

No more VirtualBox slowness.

## Next Steps

**Phase 5 - Desktop Environment:**
- Edit `build-docker.sh`
- Add GNOME/Plasma/Xfce packages
- Add theme/wallpaper files
- Rebuild and test
- Fast iteration cycle

**Phase 6 - Applications:**
- Add app packages to script
- Configure app defaults
- Rebuild
- Test UX

Docker makes development workflow smooth.

---

**Current Phase:** 3 (Hardware Support)  
**Build Method:** Docker (recommended)  
**ISO Version:** 0.2 / phase3
