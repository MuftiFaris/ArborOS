# Arbor OS Userspace - Phase 2

## Scope

Minimal working environment.
Terminal access only.
NO desktop environment (Phase 4).

## Shell

### Default: bash

Reason:
- Universal compatibility
- Script standard
- Well-documented

Configuration: `/etc/bash.bashrc`

### Root prompt
```bash
PS1='\[\e[31m\]\u@arbor-os\[\e[0m\]:\w# '
```

### User prompt
```bash
PS1='\[\e[32m\]\u@arbor-os\[\e[0m\]:\w$ '
```

## Essential Tools

### Core Utilities (coreutils)

File operations:
```
ls, cp, mv, rm, mkdir, rmdir, touch
cat, more, less, head, tail
chmod, chown, chgrp
ln, readlink
stat, wc, sort, uniq
```

Text processing:
```
grep, sed, awk
cut, paste, join
tr, expand, unexpand
```

System:
```
date, uptime, hostname
uname, arch
env, printenv
```

### File Management

```
find                # Find files
locate              # Fast file search
tree                # Directory tree
file                # File type detection
du                  # Disk usage
df                  # Filesystem usage
```

### Archiving

```
tar                 # Tape archive
gzip, gunzip        # Gzip compression
bzip2, bunzip2      # Bzip2 compression
xz, unxz            # XZ compression
zstd, unzstd        # Zstandard compression
zip, unzip          # ZIP archives
```

### Networking Tools

```
ip                  # Network config (iproute2)
ping                # ICMP echo
traceroute          # Route tracing
dig, nslookup       # DNS queries
curl                # HTTP client
wget                # File download
ssh                 # Secure shell
scp                 # Secure copy
```

### System Management

```
systemctl           # Service control
journalctl          # Log viewer
loginctl            # Session manager
hostnamectl         # Hostname config
timedatectl         # Time config
localectl           # Locale config
```

### Process Management

```
ps                  # Process list
top, htop           # Process monitor
kill, killall       # Signal processes
pgrep, pkill        # Process search/kill
nice, renice        # Priority
```

### Disk Tools

```
fdisk               # Partition editor
parted              # Partition tool
mkfs.btrfs          # Btrfs format
mkfs.ext4           # ext4 format
mkfs.vfat           # FAT format
mount, umount       # Mounting
btrfs               # Btrfs management
```

### Text Editors

```
nano                # Simple editor (default)
vim                 # Power user editor
```

nano default for Phase 2 (user-friendly).

### Package Management

```
rpm-ostree          # System packages
flatpak             # Applications
```

### Development Tools (minimal)

```
git                 # Version control
gcc, g++            # Compilers
make                # Build tool
python3             # Python
```

More dev tools in Phase 4 Developer Mode.

## Package List

Base system packages (Fedora):

```
# Core
filesystem
basesystem
setup
glibc
glibc-common
bash
coreutils
util-linux

# System
systemd
systemd-udev
dbus
dbus-broker

# Kernel
kernel
dracut

# Boot
systemd-boot
grub2-efi-x64 (fallback)

# Filesystem
btrfs-progs
e2fsprogs
dosfstools
snapper

# Network
NetworkManager
iproute
iputils
bind-utils
curl
wget
openssh-clients
openssh-server

# Text
nano
vim-minimal
less

# Archive
tar
gzip
bzip2
xz
zstd
zip
unzip

# Development
git
gcc
gcc-c++
make
python3

# Monitoring
htop
lsof
strace

# User management
shadow-utils
passwd
sudo

# SELinux
selinux-policy-targeted
policycoreutils

# Misc
man-db
info
which
hostname
```

## User Management

### Default Users

**root:**
- UID 0
- Home: /root
- Shell: /bin/bash
- Purpose: System administration

**arbor:**
- UID 1000
- Home: /home/arbor
- Shell: /bin/bash
- Groups: wheel (sudo access)
- Purpose: Default user account

### /etc/passwd
```
root:x:0:0:root:/root:/bin/bash
arbor:x:1000:1000:Arbor User:/home/arbor:/bin/bash
```

### /etc/group
```
root:x:0:
wheel:x:10:arbor
arbor:x:1000:
```

### sudo Configuration

```
# /etc/sudoers.d/wheel
%wheel ALL=(ALL) ALL
```

arbor user has sudo access via wheel group.

### Password Policy

**Default passwords:**
- root: `arbor` (CHANGE IMMEDIATELY)
- arbor: `arbor` (CHANGE IMMEDIATELY)

**Force change on first login:**
```bash
passwd -e root
passwd -e arbor
```

**Password requirements:**
- Minimum 8 characters
- No dictionary words
- Enforced by PAM

## Login System

### Console Login

Getty on tty1-6:
```
systemctl enable getty@tty1.service
```

Login prompt:
```
Arbor OS 0.1 Alpha
arbor-os login: _
```

### SSH Server

Enabled by default:
```
systemctl enable sshd.service
```

Configuration: `/etc/ssh/sshd_config`
```
PermitRootLogin no
PasswordAuthentication yes
PubkeyAuthentication yes
```

### systemd-homed

User homes encrypted per-user:
```bash
homectl create arbor \
    --storage=luks \
    --disk-size=50G
```

## Environment Variables

System-wide: `/etc/environment`
```
PATH=/usr/local/bin:/usr/bin:/bin
EDITOR=nano
VISUAL=nano
PAGER=less
```

## Localization

### Default Locale
```
LANG=en_US.UTF-8
LC_ALL=en_US.UTF-8
```

### Timezone
```
timedatectl set-timezone America/New_York
```

Adjust based on install.

### Keyboard
```
localectl set-keymap us
```

## Manual Pages

```
man-db              # Man page system
info                # GNU info
```

Documentation available offline:
```bash
man systemctl
man bash
info coreutils
```

## Logging

### systemd Journal

Persistent logs:
```bash
mkdir -p /var/log/journal
systemd-tmpfiles --create --prefix /var/log/journal
```

View logs:
```bash
journalctl
journalctl -b                   # Current boot
journalctl -f                   # Follow
journalctl -u systemd           # Service logs
journalctl -p err               # Errors only
```

## Performance

### Idle RAM Target

**≤ 400 MB** (no GUI)

Measured:
```bash
free -h
```

### Boot Time

**< 5 seconds** to login prompt

Measured:
```bash
systemd-analyze
```

## Security

### SELinux

Mode: Permissive (Phase 2)
Enforcing in Phase 6.

Check:
```bash
getenforce
sestatus
```

### Firewall

Default: block incoming, allow outgoing

```bash
systemctl enable firewalld
firewall-cmd --set-default-zone=public
```

SSH allowed:
```bash
firewall-cmd --permanent --add-service=ssh
firewall-cmd --reload
```

## Testing

Phase 2 complete when:

✅ System boots to login prompt
✅ Can login as arbor
✅ Can sudo as arbor
✅ Network works (ping, curl)
✅ Package management works
✅ Shutdown/reboot work
✅ Boot time < 8s
✅ Idle RAM < 500 MB

## References

- Spec: §3 System Architecture
- FHS: https://refspecs.linuxfoundation.org/fhs.shtml
