# ArborOS Build System

Build ArborOS Phase 5 ISO with LXQt desktop environment.

## Quick Start

### Windows

**Requirements:**
- Docker Desktop installed and running
- 4GB RAM free
- 10GB disk space

**Build:**
```cmd
# Double-click this file:
Code\build\build-phase5.bat

# Or run in PowerShell:
cd Code\build
.\build-phase5.ps1
```

**Time:** 30-45 minutes

### Linux (Ubuntu/Debian)

**Requirements:**
- Docker installed
- Root access

**Install Docker (if needed):**
```bash
sudo apt update
sudo apt install docker.io -y
sudo systemctl start docker
sudo systemctl enable docker
```

**Build:**
```bash
cd Code/build
chmod +x build-ubuntu.sh
sudo bash build-ubuntu.sh
```

**Time:** 30-45 minutes

### Linux (Fedora/RHEL/CentOS)

**Requirements:**
- DNF package manager
- Root access
- Build tools installed

**Install dependencies:**
```bash
sudo dnf install -y genisoimage syslinux squashfs-tools \
    dracut-live e2fsprogs pciutils usbutils lshw dmidecode
```

**Build:**
```bash
cd Code/build
chmod +x create_iso.sh
sudo bash create_iso.sh
```

**Time:** 30-45 minutes

### macOS

**Requirements:**
- Docker Desktop installed and running

**Build:**
```bash
cd Code/build
chmod +x build-ubuntu.sh
sudo bash build-ubuntu.sh
```

**Time:** 30-45 minutes

## Output

All build methods create:
```
Code/build/output/ArborOS-0.5.iso
```

**Size:** ~1.2-1.5 GB  
**Format:** Hybrid ISO (boots USB and CD)

## What's Included

**ArborOS Phase 5 (v0.5):**
- Fedora 39 base system
- LXQt lightweight desktop environment
- Full hardware support (GPU, WiFi, Audio, Bluetooth)
- NetworkManager for networking
- LightDM display manager with autologin
- Basic applications (terminal, file manager, text editor)
- Live boot support

**Default credentials:**
- Username: `arbor`
- Password: `arbor`

## Testing the ISO

### VirtualBox

```bash
# Create new VM
Name: ArborOS Test
Type: Linux
Version: Fedora (64-bit)
RAM: 2048 MB (minimum)
Disk: Skip (live ISO)

# Settings
Storage > Controller IDE > Add ArborOS-0.5.iso
System > Boot Order > Optical first
Display > Video Memory > 128 MB

# Start VM
```

### Physical Hardware (USB)

**Linux:**
```bash
# Find USB device
lsblk

# Write ISO (replace sdX with your device)
sudo dd if=Code/build/output/ArborOS-0.5.iso of=/dev/sdX bs=4M status=progress
sync
```

**Windows:**
- Use Rufus, Etcher, or similar tool
- Write ArborOS-0.5.iso to USB drive
- Boot from USB

### QEMU

```bash
qemu-system-x86_64 -m 2G -cdrom Code/build/output/ArborOS-0.5.iso
```

## Build Methods Comparison

| Method | Platform | Speed | Complexity |
|--------|----------|-------|------------|
| **build-phase5.bat** | Windows | Fast | Easy |
| **build-phase5.ps1** | Windows/Linux | Fast | Easy |
| **build-ubuntu.sh** | Ubuntu/Debian/macOS | Fast | Easy |
| **create_iso.sh** | Fedora/RHEL | Fastest | Medium |

**Recommended:**
- Windows → `build-phase5.bat` (double-click)
- Ubuntu/Debian → `build-ubuntu.sh`
- Fedora/RHEL → `create_iso.sh` (native, fastest)
- macOS → `build-ubuntu.sh`

## Build Process Details

**Steps:**
1. **[1/9]** Install build tools
2. **[2/9]** Bootstrap Fedora base system (~300 packages)
3. **[3/9]** Install desktop and hardware packages (~400 packages)
4. **[4/9]** Configure system (users, hostname, services)
5. **[5/9]** Build initramfs with live boot support
6. **[6/9]** Copy kernel and initramfs to ISO structure
7. **[7/9]** Create squashfs compressed filesystem
8. **[8/9]** Configure bootloader (isolinux)
9. **[9/9]** Build final ISO image

**Disk usage during build:**
- Temporary: ~8 GB (auto-cleaned)
- Final ISO: ~1.2-1.5 GB

## Troubleshooting

### Docker not starting

**Windows:**
```
Error: Cannot connect to Docker daemon
Fix: Start Docker Desktop, wait for green icon in system tray
```

**Linux:**
```bash
# Check Docker service
sudo systemctl status docker

# Start Docker
sudo systemctl start docker
```

### Build fails - disk space

```
Error: No space left on device
Fix: Free up 10GB+ disk space
Check: df -h
```

### ISO won't boot

**Check:**
- VirtualBox: Optical drive first in boot order
- USB: Use proper flashing tool (Rufus, dd)
- UEFI: Some systems need Legacy/BIOS mode

**Verify ISO integrity:**
```bash
# Check file size (should be 1.2-1.5 GB)
ls -lh Code/build/output/ArborOS-0.5.iso

# Test with QEMU
qemu-system-x86_64 -m 2G -cdrom Code/build/output/ArborOS-0.5.iso
```

### Permission denied (Linux)

```bash
# Run build with sudo
sudo bash build-ubuntu.sh

# Or add user to docker group
sudo usermod -aG docker $USER
# Logout and login again
```

### Windows line ending issues

**If you see `$'\r': command not found`:**
```bash
# Convert script to Unix line endings
dos2unix build-ubuntu.sh

# Or reinstall dos2unix
sudo apt install dos2unix
```

## Advanced Usage

### Custom packages

**Edit build script before running:**

For `create_iso.sh`, add packages in `create_rootfs()` function:
```bash
dnf --installroot="$ROOTFS" ... install \
    your-package-name \
    another-package
```

For `build-ubuntu.sh` or `.bat`/`.ps1`, add to package list in Docker command.

### Custom configuration

**Add files to ISO:**
```bash
# After configure_system() in create_iso.sh
cp /path/to/your/config "$ROOTFS/etc/your-config"
```

### Rebuild without cache

**Docker methods:**
```bash
# Clear Docker cache
docker system prune -af

# Rebuild
bash build-ubuntu.sh
```

**Native Fedora:**
```bash
# Clean previous build
sudo rm -rf Code/build/work Code/build/iso
sudo bash create_iso.sh
```

## Contributing

Found an issue with the build system? Open an issue or PR at:
https://github.com/MuftiFaris/ArborOS/issues

## Build Files

```
Code/build/
├── build-phase5.bat       # Windows: Double-click build
├── build-phase5.ps1       # PowerShell: Windows/Linux
├── build-ubuntu.sh        # Ubuntu/Debian/macOS via Docker
├── create_iso.sh          # Fedora/RHEL native build
├── README.md              # This file
└── output/
    └── ArborOS-0.5.iso    # Build output (not in git)
```

## System Requirements

**Build machine:**
- 4GB RAM minimum
- 10GB free disk space
- Internet connection (downloads ~2GB packages)

**Target system (for running ISO):**
- 2GB RAM minimum (4GB recommended)
- 64-bit x86 processor
- 10GB+ disk space (for installation)
- VirtualBox 6.0+ or physical hardware

## Development Workflow

**Fast iteration for development:**

1. Make changes to config/theme files
2. Rebuild with Docker method (30-45 min)
3. Test in VirtualBox
4. Iterate

**Docker caches package downloads, speeding up rebuilds.**

## Next Steps

**After building ISO:**

1. Test in VirtualBox
2. Verify desktop loads (LXQt)
3. Check RAM usage: `free -h` (target <500MB idle)
4. Test hardware detection: `sudo arbor-hwinfo`
5. Test network: `ping google.com`

**Phase 6:** Core applications (browser, office, etc)

---

**Current Phase:** Phase 5 - Desktop Environment  
**ISO Version:** 0.5  
**Base:** Fedora 39  
**Desktop:** LXQt  
**Build Status:** Working
