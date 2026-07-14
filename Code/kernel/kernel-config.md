# Arbor OS Kernel Configuration Strategy

## Version Selection

**Target:** Linux 6.8+ (current Fedora mainline)

Reason:
- sched-ext (SCX) support (6.8+)
- EEVDF scheduler (6.6+)
- Modern hardware support
- Fast CVE turnaround via Fedora

## Kernel Channels

### arbor-kernel (default)
- Near-mainline Fedora kernel
- Desktop-tuned
- Gaming-friendly
- Full hardware support

### arbor-kernel-hardened  
- Security-focused
- Restricted debugging
- Module signing enforced
- Opt-in only

### LTS fallback
- Always present
- Boot menu entry
- Regression safety

## Required Features (CONFIG_*)

### Architecture
```
CONFIG_X86_64=y
CONFIG_SMP=y                    # Multi-core support
CONFIG_NR_CPUS=512              # Up to 512 CPUs
```

### Scheduler
```
CONFIG_HZ=1000                  # 1kHz timer - low latency
CONFIG_PREEMPT=y                # Full desktop preemption
CONFIG_SCHED_AUTOGROUP=y        # Isolate app groups
```

### Memory
```
CONFIG_TRANSPARENT_HUGEPAGE=y
CONFIG_TRANSPARENT_HUGEPAGE_MADVISE=y
CONFIG_ZSWAP=y
CONFIG_ZSWAP_DEFAULT_ON=y
CONFIG_ZSWAP_COMPRESSOR_DEFAULT_LZ4=y
```

### Filesystems
```
CONFIG_BTRFS_FS=y
CONFIG_BTRFS_FS_POSIX_ACL=y
CONFIG_EXT4_FS=y                # Compatibility
CONFIG_VFAT_FS=y                # ESP required
CONFIG_FUSE_FS=y                # Flatpak needs
```

### Storage
```
CONFIG_BLK_DEV_NVME=y
CONFIG_ATA=y
CONFIG_SATA_AHCI=y
CONFIG_USB_STORAGE=y
CONFIG_USB_UAS=y
```

### USB
```
CONFIG_USB_XHCI_HCD=y
CONFIG_USB_EHCI_HCD=y
CONFIG_USB_OHCI_HCD=y
```

### Networking
```
CONFIG_INET=y
CONFIG_IPV6=y
CONFIG_NETFILTER=y
CONFIG_NF_TABLES=y              # nftables firewall
CONFIG_WIRELESS=y
CONFIG_CFG80211=y
CONFIG_MAC80211=y
```

### Graphics
```
CONFIG_DRM=y
CONFIG_DRM_I915=m               # Intel
CONFIG_DRM_AMDGPU=m             # AMD
CONFIG_DRM_NOUVEAU=m            # NVIDIA open
CONFIG_FB=y
```

### ACPI/Power
```
CONFIG_ACPI=y
CONFIG_ACPI_BUTTON=y
CONFIG_ACPI_FAN=y
CONFIG_ACPI_PROCESSOR=y
CONFIG_CPU_FREQ=y
CONFIG_CPU_FREQ_DEFAULT_GOV_SCHEDUTIL=y
CONFIG_PCIE_ASPM=y
```

### Security
```
CONFIG_SECURITY=y
CONFIG_SECURITY_SELINUX=y
CONFIG_SECURITY_SELINUX_BOOTPARAM=y
CONFIG_DEFAULT_SECURITY_SELINUX=y
CONFIG_SECCOMP=y
CONFIG_STRICT_DEVMEM=y
```

### Containers/Namespaces
```
CONFIG_NAMESPACES=y
CONFIG_UTS_NS=y
CONFIG_IPC_NS=y
CONFIG_PID_NS=y
CONFIG_NET_NS=y
CONFIG_USER_NS=y
CONFIG_CGROUPS=y
CONFIG_CGROUP_CPUACCT=y
CONFIG_CGROUP_DEVICE=y
CONFIG_CGROUP_FREEZER=y
CONFIG_CGROUP_SCHED=y
CONFIG_CPUSETS=y
CONFIG_MEMCG=y
CONFIG_BLK_CGROUP=y
```

## Disabled Features

```
CONFIG_DEBUG_KERNEL=n           # Production build
CONFIG_IKCONFIG=n               # No config exposure
CONFIG_MODULE_SIG_FORCE=n       # Default kernel only
```

## Build Flags

```
CFLAGS="-O2 -march=x86-64-v2 -mtune=generic"
```

x86-64-v2: SSE4.2, POPCNT support (2009+ CPUs)

## Sysctl Runtime Tuning

Applied via `/etc/sysctl.d/99-arbor.conf`:

```ini
# Memory
vm.swappiness=10
vm.dirty_ratio=15
vm.dirty_background_ratio=5
vm.vfs_cache_pressure=50

# Network
net.core.netdev_max_backlog=16384
net.ipv4.tcp_congestion_control=bbr

# Scheduler
kernel.sched_autogroup_enabled=1
```

## Testing

Boot test must pass:
- Kernel loads
- Modules load
- All hardware detected
- <8s boot time

## References

- Spec: §3.3 Kernel Strategy
- Performance doc: §1 Kernel Optimization
