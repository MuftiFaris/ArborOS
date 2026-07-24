# Arbor OS Base System Implementation

**Phase:** 2 - First Bootable Linux System  
**Target Release:** ArborOS 0.1 Alpha  
**Status:** Implementation Complete (Documentation)

---

## 1. Overview

### Purpose

ArborOS 0.1 is the first bootable version of Arbor OS. It is NOT a daily-use operating system. It is the foundation layer that later phases will build upon.

**What it is:**
- Minimal bootable Linux system
- Terminal-only access
- Working user management
- Basic networking
- System administration tools

**What it is NOT:**
- Desktop environment (Phase 4)
- Graphical installer (Phase 4)
- Application store (Phase 5)
- Polished user experience (Phase 5)

### Target Users

Internal team and early testers only.

### Success Criteria

✅ Boots from USB  
✅ Boots in QEMU/VirtualBox/VMware  
✅ Kernel loads successfully  
✅ systemd initializes  
✅ Terminal login works  
✅ Network connectivity works  
✅ Shutdown/reboot function  
✅ Boot time < 8 seconds  
✅ Idle RAM < 500 MB (no GUI)

---

## 2. Architecture

### Boot Process

```
Hardware Power-On
    ↓
UEFI/BIOS Firmware (~1.5s)
    ↓
Bootloader (systemd-boot or GRUB) (~0.3s)
    ↓
Linux Kernel Load (~1.0s)
    ↓
initramfs Decompress + Mount (~0.5s)
    ↓
systemd PID 1 Start (~0.5s)
    ↓
System Services (~2.0s)
    ↓
Login Prompt
    ↓
Total: ~6s target
```

### System Layers

```
┌─────────────────────────────────────┐
│ USER APPLICATIONS                    │
│ (Terminal, CLI tools)                │
├─────────────────────────────────────┤
│ SHELL & USERSPACE                    │
│ bash, coreutils, utilities           │
├─────────────────────────────────────┤
│ INIT SYSTEM                          │
│ systemd + services                   │
├─────────────────────────────────────┤
│ NETWORK LAYER                        │
│ NetworkManager, systemd-resolved     │
├─────────────────────────────────────┤
│ FILESYSTEM                           │
│ Btrfs (root, home, var)              │
├─────────────────────────────────────┤
│ KERNEL                               │
│ Linux 6.8+, x86_64                   │
├─────────────────────────────────────┤
│ BOOTLOADER                           │
│ systemd-boot (UEFI) / GRUB (BIOS)    │
├─────────────────────────────────────┤
│ FIRMWARE                             │
│ UEFI or Legacy BIOS                  │
└─────────────────────────────────────┘
```

### Filesystem Layout

```
/
├── bin -> usr/bin          # Essential binaries
├── boot/                   # Boot files (kernel, initramfs)
├── dev/                    # Device files (devtmpfs)
├── etc/                    # Configuration
├── home/                   # User homes (Btrfs @home)
│   └── arbor/              # Default user
├── lib -> usr/lib          # Libraries
├── mnt/                    # Mount points
├── opt/                    # Optional packages
├── proc/                   # Process info (procfs)
├── root/                   # Root home
├── run/                    # Runtime data (tmpfs)
├── sbin -> usr/sbin        # System binaries
├── sys/                    # System info (sysfs)
├── tmp/                    # Temp files (tmpfs)
├── usr/                    # User programs
│   ├── bin/                # Binaries
│   ├── lib/                # Libraries
│   ├── lib64/              # 64-bit libraries
│   └── share/              # Shared data
└── var/                    # Variable data (Btrfs @var)
    ├── log/                # Logs
    └── lib/                # State
```

---

## 3. Technology Choices

### Kernel

**Choice:** Linux 6.8+ (Fedora mainline)

**Reasoning:**
- Modern hardware support
- sched-ext (SCX) support (6.8+)
- EEVDF scheduler (6.6+)
- Active CVE patching
- Desktop-tuned config

**Configuration Highlights:**
- CONFIG_HZ=1000 (low latency)
- CONFIG_PREEMPT=y (desktop preemption)
- CONFIG_ZSWAP=y (memory compression)
- x86_64-v2 optimization

### Bootloader

**Primary:** systemd-boot (UEFI)  
**Fallback:** GRUB2 (Legacy BIOS)

**Reasoning:**
- systemd-boot: Simple, fast, UEFI-native
- GRUB: Wide hardware compatibility
- Boot menu hidden by default
- <300ms bootloader overhead

### Init System

**Choice:** systemd

**Reasoning:**
- Industry standard
- cgroups v2 integration
- Socket activation
- systemd-homed (user encryption)
- systemd-oomd (OOM management)
- systemd-resolved (DNS-over-TLS)
- Fast parallel startup

### Filesystem

**Choice:** Btrfs

**Reasoning:**
- Atomic snapshots
- Transparent compression (zstd)
- Subvolumes (/, /home, /var separation)
- Copy-on-write
- SSD-optimized
- Native Linux support

**Mount Options:**
```
noatime              # Performance
compress=zstd:3      # ~60% compression
space_cache=v2       # Fast free space
discard=async        # SSD TRIM
```

### Package Strategy

**Phase 2:** Manual package selection  
**Phase 3+:** rpm-ostree (atomic updates)

**Base Packages:**
- Fedora Minimal Install
- systemd ecosystem
- NetworkManager
- OpenSSH
- Basic CLI tools

**Rationale:**
- Minimal attack surface
- Fast boot
- Low memory usage
- Standard tools

---

## 4. Build Instructions

### Prerequisites

**Host System:** Linux (Fedora, Ubuntu, or similar)

**Required Tools:**
- dnf (Fedora package manager)
- mkisofs / genisoimage
- isohybrid
- mksquashfs
- QEMU (testing)

Install on Fedora:
```bash
sudo dnf install -y \
    dnf-plugins-core \
    genisoimage \
    syslinux \
    squashfs-tools \
    qemu-system-x86
```

Install on Ubuntu/Debian:
```bash
sudo apt install -y \
    debootstrap \
    genisoimage \
    syslinux-utils \
    squashfs-tools \
    qemu-system-x86
```

### Build Process

1. **Clone repository:**
```bash
cd /path/to/MyLinuxOS/Code
```

2. **Run build script:**
```bash
sudo ./build/create_iso.sh
```

This will:
- Create root filesystem
- Bootstrap Fedora base
- Install packages
- Configure system
- Create initramfs
- Build bootloader
- Generate squashfs
- Build ISO

3. **Output:**
```
build/output/ArborOS-0.1.iso
```

### Build Time

Approximately 15-30 minutes depending on:
- Internet speed (downloads packages)
- CPU speed (compression)
- Disk speed (I/O)

### Build Artifacts

```
build/
├── work/                    # Temporary build files
│   ├── rootfs/              # Root filesystem
│   └── ...
├── iso/                     # ISO staging
│   ├── boot/
│   ├── isolinux/
│   └── LiveOS/
└── output/
    └── ArborOS-0.1.iso      # Final ISO
```

### Customization

Edit before building:

**Packages:** Modify package list in `create_iso.sh`

**Users:** Change default user in configure_system()

**Services:** Enable/disable in configure_system()

**Kernel:** Specify version in create_rootfs()

---

## 5. Testing Guide

### Virtual Machine Testing

#### QEMU

**Automated test:**
```bash
./tests/test_boot.sh
```

**Manual:**
```bash
qemu-system-x86_64 \
    -enable-kvm \
    -m 2G \
    -cdrom build/output/ArborOS-0.1.iso \
    -boot d
```

**Controls:**
- Ctrl+Alt+G: Release mouse
- Ctrl+Alt+2: QEMU monitor
- Type `quit` to exit

#### VirtualBox

1. Create new VM:
   - Type: Linux
   - Version: Fedora (64-bit)
   - RAM: 2048 MB
   - No hard disk needed (live boot)

2. Settings:
   - Storage → Add optical drive → Select ISO
   - System → Enable EFI (optional)

3. Start VM

#### VMware

1. Create new VM:
   - Typical configuration
   - Linux → Fedora 64-bit
   - RAM: 2048 MB

2. Settings:
   - CD/DVD → Use ISO image → Select ISO

3. Power on

### USB Boot Testing

**⚠️ WARNING:** This will write to USB drive. All data will be lost.

1. **Identify USB device:**
```bash
lsblk
# Look for your USB drive (e.g., /dev/sdb)
```

2. **Write ISO:**
```bash
sudo dd if=build/output/ArborOS-0.1.iso of=/dev/sdX bs=4M status=progress
sync
```

Replace `/dev/sdX` with your USB device.

3. **Boot from USB:**
- Restart computer
- Enter boot menu (usually F12, F2, or Del)
- Select USB drive
- Boot Arbor OS

### Hardware Testing

**Minimum Hardware:**
- CPU: x86_64 (64-bit)
- RAM: 1 GB (2 GB recommended)
- Storage: USB or 8 GB+ disk
- Network: Ethernet (WiFi Phase 3+)

**Test Checklist:**

✅ System boots  
✅ Login prompt appears  
✅ Can login as `arbor`  
✅ Password: `arbor`  
✅ Can run `sudo` commands  
✅ Network interface detected: `ip link`  
✅ IP address assigned: `ip addr`  
✅ DNS works: `ping google.com`  
✅ Internet works: `curl https://example.com`  
✅ Shutdown works: `sudo systemctl poweroff`  
✅ Reboot works: `sudo systemctl reboot`

### Performance Testing

**Boot Time:**
```bash
systemd-analyze
systemd-analyze blame
```

Target: < 8 seconds total

**Memory Usage:**
```bash
free -h
```

Target: < 500 MB idle (no GUI)

**Disk Usage:**
```bash
df -h
```

Base system: ~2-3 GB

### Network Testing

**Interface:**
```bash
ip link show
nmcli device status
```

**IP Address:**
```bash
ip addr show
```

**DNS:**
```bash
resolvectl status
dig google.com
```

**Connectivity:**
```bash
ping -c 4 1.1.1.1
ping -c 4 google.com
curl -I https://example.com
```

**Firewall:**
```bash
sudo firewall-cmd --list-all
```

SSH should be allowed.

---

## 6. Known Limitations

### Phase 2 Scope

**NOT IMPLEMENTED:**

❌ Desktop environment (terminal only)  
❌ Graphical login  
❌ File manager GUI  
❌ Application store  
❌ WiFi configuration UI  
❌ System installer  
❌ Secure Boot  
❌ Full disk encryption  
❌ Automated updates  
❌ Snapper snapshots  
❌ SELinux (permissive mode only)  
❌ Privacy controls  
❌ Custom branding  

### Hardware Limitations

**Phase 2 Support:**

✅ x86_64 CPUs  
✅ UEFI firmware  
✅ Legacy BIOS  
✅ Ethernet adapters  
✅ Standard storage (SATA, NVMe)  
✅ USB devices  

**Not Yet Supported:**

❌ WiFi (Phase 3+)  
❌ Bluetooth (Phase 3+)  
❌ Touchscreen (Phase 4+)  
❌ Fingerprint readers (Phase 4+)  
❌ Hybrid graphics (Phase 3+)  

### Security Limitations

**Phase 2 Status:**

⚠️ Default passwords (`arbor`)  
⚠️ Root SSH login disabled  
⚠️ Firewall enabled but basic  
⚠️ SELinux permissive mode  
⚠️ No Secure Boot  
⚠️ No disk encryption  

**Change default passwords immediately:**
```bash
passwd              # Change arbor password
sudo passwd root    # Change root password
```

### Stability

This is **ALPHA** software:

- Not production-ready
- Bugs expected
- Data loss possible
- Test on non-critical hardware
- No warranty

---

## 7. Next Steps

### Phase 3 - Hardware Support

- WiFi drivers and configuration
- Bluetooth support
- Fingerprint readers
- Graphics drivers (NVIDIA, AMD, Intel)
- Power management
- Suspend/resume
- Hardware compatibility list (HCL)

### Phase 4 - Desktop Environment

- Arbor Shell (GNOME-based)
- Graphical login (GDM)
- File manager
- System settings
- Graphical installer
- Application launcher

### Phase 5 - Applications

- Arbor Store
- First-party apps
- Flatpak support
- Developer tooling
- Container support (Distrobox)

### Phase 6 - Security & Performance

- SELinux enforcing mode
- Secure Boot
- Full disk encryption
- Performance optimization
- Gaming support
- Battery optimization

---

## 8. Getting Help

### Documentation

See `Code/` directory:
- `kernel/kernel-config.md` - Kernel configuration
- `system/boot-system.md` - Boot system
- `system/filesystem-layout.md` - Filesystem
- `system/init-system.md` - systemd
- `system/userspace.md` - Userspace tools
- `system/networking.md` - Networking

### Logs

**Boot logs:**
```bash
journalctl -b
```

**Service logs:**
```bash
journalctl -u NetworkManager
journalctl -u sshd
```

**Kernel messages:**
```bash
dmesg
```

### Troubleshooting

**Boot fails:**
- Check QEMU/VM settings
- Verify ISO integrity
- Try BIOS mode (not UEFI)

**Network not working:**
- Check interface: `ip link`
- Restart NetworkManager: `sudo systemctl restart NetworkManager`
- Check firewall: `sudo firewall-cmd --list-all`

**Can't login:**
- Default user: `arbor`
- Default password: `arbor`
- Try root login (console only)

---

## 9. Contributing

### Feedback

Report issues to development team.

### Testing

Help test on different hardware:
- Document hardware specs
- Report boot success/failure
- Test network connectivity
- Measure boot time
- Report bugs

### Development

Phase 2 is feature-complete.
Focus shifts to Phase 3 (Hardware Support).

---

## 10. References

### Specification Documents

- `FINAL_OS_SPECIFICATION_V2.md` - Product spec
- `IMPLEMENTATION_MASTER_PLAN.md` - Implementation plan
- `REPOSITORY_STRUCTURE.md` - Repo structure

### External Resources

- Linux Kernel: https://kernel.org
- systemd: https://systemd.io
- Btrfs: https://btrfs.wiki.kernel.org
- Fedora: https://fedoraproject.org

---

**Document Version:** 1.0  
**Last Updated:** 2026-07-26  
**Phase:** 2 - First Bootable System  
**Status:** ✅ Complete (Documentation)
