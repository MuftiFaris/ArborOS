# Arbor OS Configuration Files

## Phase 2 Configuration Strategy

Configuration files for the base system.

**Location in ISO:** `/etc/` in rootfs

## Configuration Categories

### System Identity

**Hostname:**
```
/etc/hostname
---
arbor-os
```

**Hosts:**
```
/etc/hosts
---
127.0.0.1   localhost
127.0.1.1   arbor-os
::1         localhost ip6-localhost ip6-loopback
```

**OS Release:**
```
/etc/os-release
---
NAME="Arbor OS"
VERSION="0.1 Alpha"
ID=arbor
ID_LIKE=fedora
VERSION_ID=0.1
PRETTY_NAME="Arbor OS 0.1 Alpha"
ANSI_COLOR="0;32"
HOME_URL="https://arbor-os.example.com"
BUG_REPORT_URL="https://github.com/arbor-os/issues"
```

### Locale & Time

**Locale:**
```
/etc/locale.conf
---
LANG=en_US.UTF-8
LC_ALL=en_US.UTF-8
```

**Timezone:**
```bash
# Symlink
/etc/localtime -> /usr/share/zoneinfo/UTC
```

**Keyboard:**
```
/etc/vconsole.conf
---
KEYMAP=us
FONT=latarcyrheb-sun16
```

### User Management

**Default Shell:**
```
/etc/shells
---
/bin/bash
/bin/sh
```

**sudo Configuration:**
```
/etc/sudoers.d/wheel
---
%wheel ALL=(ALL) ALL
```

**Login Defaults:**
```
/etc/login.defs
---
PASS_MAX_DAYS   99999
PASS_MIN_DAYS   0
PASS_MIN_LEN    8
PASS_WARN_AGE   7
UID_MIN         1000
UID_MAX         60000
GID_MIN         1000
GID_MAX         60000
CREATE_HOME     yes
UMASK           022
```

### Networking

**systemd-resolved:**
```
/etc/systemd/resolved.conf
---
[Resolve]
DNS=1.1.1.1#cloudflare-dns.com 1.0.0.1
FallbackDNS=8.8.8.8 8.8.4.4
DNSOverTLS=opportunistic
DNSSEC=allow-downgrade
Domains=~.
```

**SSH Server:**
```
/etc/ssh/sshd_config
---
Port 22
PermitRootLogin no
PasswordAuthentication yes
PubkeyAuthentication yes
X11Forwarding no
PrintMotd no
AcceptEnv LANG LC_*
Subsystem sftp /usr/libexec/openssh/sftp-server
```

**NetworkManager:**
```
/etc/NetworkManager/NetworkManager.conf
---
[main]
plugins=keyfile
dns=systemd-resolved

[keyfile]
unmanaged-devices=none
```

**Firewall:**
```
/etc/firewalld/firewalld.conf
---
DefaultZone=public
CleanupOnExit=yes
```

### Kernel & Boot

**Kernel Parameters:**
```
/etc/sysctl.d/99-arbor.conf
---
# Memory
vm.swappiness=10
vm.dirty_ratio=15
vm.dirty_background_ratio=5
vm.vfs_cache_pressure=50

# Network
net.core.netdev_max_backlog=16384
net.ipv4.tcp_congestion_control=bbr
net.ipv4.tcp_fastopen=3

# Scheduler
kernel.sched_autogroup_enabled=1
```

**Boot Parameters:**
```
/etc/default/grub
---
GRUB_TIMEOUT=5
GRUB_CMDLINE_LINUX="ro quiet"
GRUB_DISABLE_OS_PROBER=true
```

**Dracut:**
```
/etc/dracut.conf.d/arbor.conf
---
hostonly="yes"
hostonly_cmdline="yes"
compress="lz4"
omit_dracutmodules+=" plymouth "
add_drivers+=" btrfs "
```

### systemd Services

**Journal:**
```
/etc/systemd/journald.conf
---
[Journal]
Storage=persistent
Compress=yes
SystemMaxUse=500M
RuntimeMaxUse=100M
```

**systemd-oomd:**
```
/etc/systemd/oomd.conf
---
[OOM]
SwapUsedLimit=80%
DefaultMemoryPressureLimit=60%
DefaultMemoryPressureDurationSec=10s
```

**systemd-homed:**
```
/etc/systemd/homed.conf
---
[Home]
DefaultStorage=luks
DefaultFileSystemType=btrfs
```

### Filesystem

**fstab:**
```
/etc/fstab
---
# <device>      <mount>     <type>  <options>                                           <dump> <pass>
UUID=<ROOT>     /           btrfs   defaults,noatime,compress=zstd:3,subvol=@           0      0
UUID=<ROOT>     /home       btrfs   defaults,noatime,compress=zstd:3,subvol=@home       0      0
UUID=<ROOT>     /var        btrfs   defaults,noatime,compress=zstd:3,subvol=@var        0      0
UUID=<ESP>      /boot/efi   vfat    defaults,noatime                                     0      2
tmpfs           /tmp        tmpfs   defaults,noatime,mode=1777,size=4G                   0      0
```

**tmpfiles:**
```
/etc/tmpfiles.d/arbor.conf
---
# Type Path        Mode UID  GID  Age Argument
d /run/user        0755 -    -    -   -
d /var/cache/arbor 0755 root root -   -
```

### SELinux

**Config:**
```
/etc/selinux/config
---
SELINUX=permissive
SELINUXTYPE=targeted
```

Phase 2: permissive  
Phase 6+: enforcing

### Shell

**bash global:**
```
/etc/bash.bashrc
---
# Arbor OS bash configuration

# Prompt
if [ "$UID" -eq 0 ]; then
    PS1='\[\e[31m\]\u@\h\[\e[0m\]:\w# '
else
    PS1='\[\e[32m\]\u@\h\[\e[0m\]:\w$ '
fi

# Aliases
alias ll='ls -lh'
alias la='ls -lah'
alias ..='cd ..'
alias ...='cd ../..'

# History
HISTSIZE=10000
HISTFILESIZE=20000
HISTCONTROL=ignoredups:erasedups

# Safety
alias rm='rm -i'
alias cp='cp -i'
alias mv='mv -i'
```

**Profile:**
```
/etc/profile.d/arbor.sh
---
# Arbor OS environment

export EDITOR=nano
export VISUAL=nano
export PAGER=less
```

### Message of the Day

**motd:**
```
/etc/motd
---
   _            _                  ___  ____  
  / \   _ __  | |__    ___  _ __  / _ \/ ___| 
 / _ \ | '__| | '_ \  / _ \| '__|| | | \___ \ 
/ ___ \| |    | |_) || (_) | |   | |_| |___) |
/_/   \_\_|    |_.__/  \___/|_|    \___/|____/ 

Arbor OS 0.1 Alpha - First Bootable System

WARNING: Alpha software - Not for production use
Default password: arbor - CHANGE IMMEDIATELY

Documentation: /usr/share/doc/arbor
Report issues: https://github.com/arbor-os/issues

```

**issue:**
```
/etc/issue
---
Arbor OS 0.1 Alpha
Kernel \r on \m (\l)

```

## Configuration Management

### Phase 2: Static Files

Files baked into ISO during build.

### Phase 3+: Dynamic

- systemd-sysusers for users
- systemd-tmpfiles for directories
- Ansible for configuration management
- Git for version control

## Security

**File Permissions:**
```
/etc/shadow         0000 (root:root)
/etc/gshadow        0000 (root:root)
/etc/ssh/sshd_config 0600 (root:root)
/etc/sudoers.d/*    0440 (root:root)
```

**SELinux Contexts:**
Applied automatically by SELinux policy.

## Testing

Verify configurations:
```bash
# Syntax
sudo bash -n /etc/bash.bashrc
sudo sshd -t

# Apply
sudo systemctl daemon-reload
sudo systemctl restart sshd
sudo sysctl --system
```

## References

- Configuration locations: FHS 3.0
- systemd configs: https://systemd.io
- Fedora defaults: /usr/share/doc/*/
