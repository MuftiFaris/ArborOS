# Arbor OS Packages

## Phase 2 Package Strategy

**Approach:** Direct package installation via dnf during build

**Future (Phase 3+):** rpm-ostree for atomic updates

## Package Categories

### Essential System

```
filesystem
basesystem
setup
glibc
glibc-common
bash
coreutils
util-linux
```

### Kernel & Boot

```
kernel
kernel-modules
dracut
systemd-boot
grub2-efi-x64       # BIOS fallback
grub2-efi-x64-modules
shim-x64            # Secure Boot (future)
```

### Init & System Services

```
systemd
systemd-udev
systemd-resolved
dbus
dbus-broker
```

### Filesystem Tools

```
btrfs-progs
e2fsprogs
dosfstools
xfsprogs
snapper              # Snapshot management (future)
```

### Networking

```
NetworkManager
NetworkManager-wifi  # Phase 3+
iproute
iputils
bind-utils
curl
wget
openssh-clients
openssh-server
firewalld
nftables
```

### User Management

```
shadow-utils
passwd
sudo
cronie               # Cron daemon
```

### Text Editors

```
nano                 # Default
vim-minimal
```

### Compression/Archives

```
tar
gzip
bzip2
xz
zstd
zip
unzip
p7zip
```

### Development Tools (Minimal)

```
git
gcc
gcc-c++
make
python3
python3-pip
```

### Monitoring Tools

```
htop
iotop
lsof
strace
tcpdump
```

### System Info

```
pciutils             # lspci
usbutils             # lsusb
dmidecode
hdparm
smartmontools
```

### Documentation

```
man-db
man-pages
info
which
```

### SELinux

```
selinux-policy-targeted
policycoreutils
selinux-policy-devel  # Future hardening
```

### Misc Utilities

```
tree
rsync
screen
tmux
less
findutils
grep
sed
gawk
```

## Package Sources

### Primary Repository

Fedora 39 base repositories:
- fedora
- updates

### Optional (Phase 3+)

- RPM Fusion (multimedia codecs)
- Flathub (GUI applications)
- Arbor repo (custom packages)

## Installation Method

### Phase 2: Direct Install

During ISO build:
```bash
dnf --installroot=$ROOTFS \
    --releasever=39 \
    --setopt=install_weak_deps=False \
    --nodocs \
    -y install <packages>
```

### Phase 3+: rpm-ostree

Atomic image updates:
```bash
rpm-ostree install <package>
rpm-ostree ex override remove <package>
```

## Size Optimization

**Flags:**
- `--setopt=install_weak_deps=False` - No weak dependencies
- `--nodocs` - No documentation (saves ~500MB)

**Result:**
- Base system: ~2-3 GB
- With compression: ~1.5 GB

## Package Exclusions

**NOT included in Phase 2:**

- Desktop environment packages
- GUI applications
- X11/Wayland (only needed Phase 4+)
- Office suites
- Web browsers
- Media players
- Games
- Development IDEs

Reason: Terminal-only Phase 2 focus

## Future Package Management

### Phase 3+: Flatpak

GUI applications delivered as Flatpaks:
- Sandboxed
- Cross-distro
- User-installable
- Arbor Store integration

### Phase 4+: rpm-ostree Layering

System-level packages:
- Atomic updates
- Rollback support
- Image-based
- CI-verified

### Developer Tooling

- Homebrew for CLI tools
- Distrobox for dev containers
- Language-specific tools (cargo, npm, pip)

## References

- Fedora Packages: https://packages.fedoraproject.org
- DNF: https://dnf.readthedocs.io
- rpm-ostree: https://coreos.github.io/rpm-ostree/
