# Arbor OS Root Filesystem

## Directory Structure

Standard Linux Filesystem Hierarchy (FHS 3.0):

```
/
├── bin -> usr/bin              # Essential user binaries (symlink)
├── boot                        # Boot files (kernel, initramfs)
├── dev                         # Device files (managed by udev)
├── etc                         # System configuration
├── home                        # User home directories
├── lib -> usr/lib              # Essential libraries (symlink)
├── lib64 -> usr/lib64          # 64-bit libraries (symlink)
├── media                       # Removable media mount points
├── mnt                         # Temporary mount points
├── opt                         # Optional/add-on packages
├── proc                        # Process information (virtual)
├── root                        # Root user home
├── run                         # Runtime data (tmpfs)
├── sbin -> usr/sbin            # System binaries (symlink)
├── srv                         # Service data
├── sys                         # System information (virtual)
├── tmp                         # Temporary files (tmpfs)
├── usr                         # User programs and data
└── var                         # Variable data
```

## Directory Purposes

### /bin, /sbin, /lib, /lib64
**Symlinks to /usr/** (merged /usr)

Modern distros merge these into /usr for consistency.
Maintains backward compatibility.

### /boot
**Purpose:** Boot files
**Contents:**
- vmlinuz-* (kernel)
- initramfs-* (initial ramdisk)
- System.map-* (kernel symbol table)
- config-* (kernel config)

**Size:** 1GB partition
**Filesystem:** ext4 or vfat (UEFI ESP)

### /dev
**Purpose:** Device files
**Type:** devtmpfs (managed by kernel/udev)
**Contents:**
- Block devices (/dev/sda, /dev/nvme0n1)
- Character devices (/dev/tty, /dev/null)
- Pseudo devices (/dev/urandom, /dev/zero)

### /etc
**Purpose:** System configuration
**Key directories:**
```
/etc/
├── default/            # Service defaults
├── fstab               # Filesystem mounts
├── hostname            # System hostname
├── hosts               # Static host table
├── passwd              # User accounts
├── shadow              # Password hashes
├── group               # User groups
├── os-release          # OS identification
├── systemd/            # systemd config
│   └── system/         # Service units
├── sysctl.d/           # Kernel parameters
├── udev/               # Device rules
│   └── rules.d/
└── NetworkManager/     # Network config
```

### /home
**Purpose:** User home directories
**Structure:**
```
/home/
├── arbor/              # Default user
│   ├── .bashrc
│   ├── .config/
│   ├── Documents/
│   ├── Downloads/
│   └── ...
└── <other-users>/
```

**Filesystem:** Btrfs subvolume (@home)
**Encryption:** Per-user via systemd-homed

### /media
**Purpose:** Removable media auto-mount points
**Managed by:** udisks2
**Example:**
```
/media/arbor/USB_DRIVE/
/media/arbor/CDROM/
```

### /mnt
**Purpose:** Administrator mount points
**Usage:** Manual temporary mounts

### /opt
**Purpose:** Optional third-party software
**Example:**
```
/opt/google/chrome/
/opt/proprietary-app/
```

### /proc
**Type:** procfs (virtual filesystem)
**Purpose:** Process and kernel information
**Key files:**
- /proc/cpuinfo
- /proc/meminfo
- /proc/[pid]/*
- /proc/sys/* (sysctl interface)

### /root
**Purpose:** Root user home directory
**Not** in /home for security isolation

### /run
**Type:** tmpfs (RAM)
**Purpose:** Runtime state since last boot
**Contents:**
- PID files
- Sockets
- Transient files
**Cleared:** Every boot

### /srv
**Purpose:** Site-specific service data
**Example:** Web server content

### /sys
**Type:** sysfs (virtual filesystem)
**Purpose:** Device and driver information
**Managed by:** Kernel

### /tmp
**Type:** tmpfs (RAM) 
**Purpose:** Temporary files
**Cleared:** Every boot or periodically
**Size:** 4GB (configurable)

### /usr
**Purpose:** User programs and data
**Structure:**
```
/usr/
├── bin/                # User binaries
├── sbin/               # System binaries
├── lib/                # Libraries
├── lib64/              # 64-bit libraries
├── libexec/            # Binary helpers
├── share/              # Architecture-independent data
│   ├── applications/   # Desktop entries
│   ├── icons/          # Icon themes
│   ├── man/            # Manual pages
│   └── doc/            # Documentation
├── src/                # Source code
├── include/            # C headers
└── local/              # Locally installed software
    ├── bin/
    ├── lib/
    └── share/
```

**Immutable:** Read-only in atomic image

### /var
**Purpose:** Variable data
**Structure:**
```
/var/
├── cache/              # Application caches
├── lib/                # Persistent app data
│   ├── flatpak/        # Flatpak data
│   ├── systemd/        # systemd state
│   └── rpm/            # RPM database
├── log/                # Log files
│   ├── journal/        # systemd journal
│   └── ...
├── spool/              # Queues (mail, print)
├── tmp/                # Preserved across reboots
└── www/                # Web content (if applicable)
```

**Filesystem:** Btrfs subvolume (@var)
**Mutable:** System state changes here

## Filesystem Table (/etc/fstab)

```
# <device>      <mount>     <type>  <options>                                           <dump> <pass>
UUID=<ROOT>     /           btrfs   defaults,noatime,compress=zstd:3,subvol=@           0      0
UUID=<ROOT>     /home       btrfs   defaults,noatime,compress=zstd:3,subvol=@home       0      0
UUID=<ROOT>     /var        btrfs   defaults,noatime,compress=zstd:3,subvol=@var        0      0
UUID=<ROOT>     /.snapshots btrfs   defaults,noatime,compress=zstd:3,subvol=@snapshots  0      0
UUID=<ESP>      /boot/efi   vfat    defaults,noatime                                     0      2
tmpfs           /tmp        tmpfs   defaults,noatime,mode=1777,size=4G                   0      0
tmpfs           /run        tmpfs   defaults,noatime,mode=0755                           0      0
```

## Btrfs Subvolumes

```
/
├── @ (root)
├── @home (user data)
├── @var (system state)
└── @snapshots (snapshot storage)
```

**Benefits:**
- Independent snapshots per subvolume
- Separate mount options
- Snapshot isolation
- Rollback granularity

## Mount Options

### Btrfs
```
noatime              # No access time updates (performance)
compress=zstd:3      # Transparent compression
space_cache=v2       # Fast free space tracking
discard=async        # SSD TRIM (async)
```

### tmpfs
```
noatime              # No access time
mode=1777            # Sticky bit (tmp)
size=4G              # Max size limit
```

### FAT32 (ESP)
```
noatime              # Performance
umask=0077           # Secure permissions
```

## Immutability Model

**Read-only:** /usr (OSTree-managed)
**Read-write:** /etc, /var, /home
**Layering:** rpm-ostree overlay

Changes to /usr require new deployment (atomic update).

## Permissions

Standard Unix permissions:

```
Directory   Owner:Group    Mode    Sticky
/           root:root      0755    -
/home       root:root      0755    -
/root       root:root      0700    -
/tmp        root:root      1777    yes
/var/tmp    root:root      1777    yes
/etc        root:root      0755    -
/boot       root:root      0755    -
```

## SELinux Context

SELinux labels applied to all files.
Default policy: targeted

## Snapshot Strategy

**Tool:** Snapper (Btrfs)

**Timeline:**
- Before OS update
- Before rpm-ostree layering
- Hourly (home), keep 24
- Daily (home), keep 7
- Weekly (home), keep 4

**Disk space:** 15% free space minimum
Auto-prune oldest when threshold hit.

## References

- Spec: §3.5 Filesystem
- FHS: https://refspecs.linuxfoundation.org/fhs.shtml
