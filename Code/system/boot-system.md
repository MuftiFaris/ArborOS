# Arbor OS Boot System

## Boot Chain

```
Hardware
  ↓
UEFI/BIOS Firmware
  ↓
Bootloader (systemd-boot or GRUB)
  ↓
Linux Kernel
  ↓
initramfs
  ↓
Init System (systemd)
  ↓
Userspace
```

## Bootloader Strategy

### Primary: systemd-boot (UEFI)

**Why:**
- Simple, fast
- Native UEFI support
- Secure Boot ready
- No configuration complexity
- <300ms overhead

**Configuration:**
```
/boot/efi/loader/loader.conf
---
timeout 5
default arbor-@DEFAULT@
console-mode max
editor no
```

**Boot Entries:**
```
/boot/efi/loader/entries/arbor.conf
---
title Arbor OS
linux /vmlinuz-arbor
initrd /initramfs-arbor.img
options root=UUID=<ROOT-UUID> ro quiet loglevel=3
```

### Fallback: GRUB (Legacy BIOS)

For hardware without UEFI.

**Configuration:**
```
/boot/grub/grub.cfg
---
set timeout=5
set default=0

menuentry 'Arbor OS' {
    linux /vmlinuz-arbor root=UUID=<ROOT-UUID> ro quiet
    initrd /initramfs-arbor.img
}
```

## ESP (EFI System Partition)

**Filesystem:** FAT32 (vfat)
**Size:** 512MB minimum
**Mount:** /boot/efi

**Structure:**
```
/boot/efi/
├── EFI/
│   ├── BOOT/
│   │   └── BOOTX64.EFI          # Fallback bootloader
│   └── arbor/
│       └── systemd-bootx64.efi  # systemd-boot
└── loader/
    ├── loader.conf
    └── entries/
        └── arbor.conf
```

## Boot Menu

**Hidden by default** - goes straight to OS

**Shows when:**
- Key held during POST
- Previous boot failed
- Manual boot menu request

**Entries:**
1. Arbor OS (current)
2. Arbor OS (previous) - rollback
3. Recovery Mode

**Timeout:** 5 seconds (when visible)

## Kernel Boot Parameters

### Default (quiet boot)
```
root=UUID=<UUID>
ro
quiet
loglevel=3
splash=silent
```

### Debug Mode
```
root=UUID=<UUID>
ro
loglevel=7
systemd.log_level=debug
```

### Recovery Mode
```
root=UUID=<UUID>
rw
single
systemd.unit=rescue.target
```

## initramfs

**Tool:** dracut
**Compression:** LZ4 (fast decompression)

**Modules included:**
- btrfs
- dm-crypt (LUKS2)
- systemd
- network (minimal)

**Size target:** <20MB

**Generation:**
```bash
dracut --force \
       --hostonly \
       --compress=lz4 \
       --add-drivers "btrfs dm-crypt" \
       /boot/initramfs-arbor.img \
       <kernel-version>
```

## Secure Boot

**Chain of Trust:**
```
UEFI Firmware (Microsoft 3rd-party CA)
  ↓ verifies
Shim (Arbor-signed, MS-certified)
  ↓ verifies
systemd-boot (Arbor-signed)
  ↓ verifies  
Kernel + initramfs (Arbor-signed)
```

**Phase 2:** Not implemented yet
**Phase 4:** Full Secure Boot chain

## Boot Time Target

**< 8 seconds** cold boot to login screen

**Breakdown:**
- UEFI firmware: ~1.5s
- Bootloader: ~0.3s
- Kernel + initramfs: ~2.0s
- systemd to login: ~3.0s
- **Total: ~6.8s**

**CI Gate:** Any commit breaking <8s blocks release

## Boot Regression Testing

Automated via systemd-analyze in QEMU:

```bash
systemd-analyze
systemd-analyze blame
systemd-analyze critical-chain
```

Fail if total >8.0s.

## Plymouth

**Status:** DISABLED

**Reason:** 
- Boot time overhead
- Performance > cosmetics in Phase 2

**Alternative:**
- Clean black screen until login
- Session-start sound (not boot sound)

## Recovery Mode

Separate minimal system:
- Terminal access
- Network tools
- Filesystem tools
- Snapshot restore

Accessible from boot menu.

## References

- Spec: §3.6 Boot System
- Performance: §8 Boot Time Optimization
