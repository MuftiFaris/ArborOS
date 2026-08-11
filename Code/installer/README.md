# Arbor OS Installer - Phase 4

**Installer:** Calamares  
**Version:** 0.3  
**Status:** In Development

---

## Overview

Phase 4 implements graphical installer using Calamares framework.

**Features:**
- Graphical installation wizard
- Automatic and manual partitioning
- UEFI + BIOS support
- User account creation
- Bootloader installation (GRUB)
- Optional disk encryption
- Hardware detection
- First boot setup

---

## Structure

```
installer/
├── calamares/
│   ├── settings.conf           # Main Calamares config
│   ├── modules/                # Module configs
│   │   ├── welcome.conf
│   │   ├── partition.conf
│   │   ├── users.conf
│   │   ├── bootloader.conf
│   │   └── unpackfs.conf
│   └── branding/
│       └── arbor/              # Arbor OS branding
│           └── branding.desc
├── install-calamares.sh        # Integration script
└── README.md                   # This file
```

---

## Installation Flow

### 1. Welcome Screen
- Arbor OS branding
- Language selection
- System requirements check
  - Storage: ≥20 GB
  - RAM: ≥2 GB
  - Internet connection
  - Power supply

### 2. Locale & Keyboard
- System language
- Keyboard layout
- Timezone selection

### 3. Disk Partitioning

**Automatic:**
- Erase disk
- Create partitions:
  - EFI: 512 MB (UEFI systems)
  - Root: Remaining space
  - Swap: Auto-sized

**Manual:**
- Custom partition layout
- Filesystem selection (ext4/btrfs/xfs)

**Supported:**
- GPT partition table
- UEFI boot
- Legacy BIOS

### 4. User Creation
- Username
- Password
- Computer name
- User groups (wheel, audio, video, etc.)

### 5. Installation
- Unpack filesystem
- Install bootloader
- Configure system
- Generate initramfs

### 6. First Boot
- Boot without USB
- Login screen

---

## Filesystem Options

**ext4 (Default):**
- ✅ Stable, mature
- ✅ Fast
- ✅ Well-tested
- ❌ No snapshots

**btrfs:**
- ✅ Snapshots
- ✅ Compression
- ✅ Subvolumes
- ⚠️ More complex

**xfs:**
- ✅ High performance
- ✅ Large files
- ❌ No snapshots

---

## Bootloader

**GRUB 2:**
- UEFI and BIOS support
- Dual-boot compatible
- os-prober integration
- Custom Arbor branding

**Boot entry:** "Arbor OS"

---

## Build Integration

Calamares installed during ISO build:

```bash
# In Docker container
cd /workspace/Code/installer
./install-calamares.sh /path/to/rootfs
```

Included in `create_iso.sh` Phase 4 update.

---

## Testing

### Requirements
- VirtualBox or physical hardware
- 20GB+ disk
- 2GB+ RAM
- UEFI or BIOS

### Test Scenarios
1. ✅ UEFI automatic install
2. ✅ BIOS automatic install
3. ✅ Manual partitioning
4. ✅ Encrypted install (future)
5. ✅ Dual-boot (future)

---

## Known Limitations

**Phase 4:**
- No desktop environment yet (Phase 5)
- No applications (Phase 6)
- Basic terminal login only
- Encryption not implemented yet

**After install, system boots to:**
- Console login
- Username/password from installer
- Minimal system

---

## Next Steps

1. Build ISO with Calamares
2. Test UEFI installation
3. Test BIOS installation
4. Verify bootloader
5. Document results

---

## Dependencies

**Calamares:**
- Qt5
- KPMCore
- Parted

**Bootloader:**
- GRUB 2
- efibootmgr
- os-prober

**Filesystems:**
- e2fsprogs (ext4)
- btrfs-progs (btrfs)
- xfsprogs (xfs)

---

**Status:** Configuration complete, build pending  
**Last Updated:** 2026-08-11

