# Arbor OS Init System

## Choice: systemd

**Rationale:**
- Industry standard
- cgroups v2 native
- Socket activation
- Service dependencies
- Boot time optimization
- Integrated tools (homed, oomd, boot, resolved)

## Core Components

### systemd (PID 1)
Main init daemon
Manages all services and system state

### systemd-journald
Logging system
Binary logs in /var/log/journal

### systemd-udevd
Device manager
Handles hardware events

### systemd-logind
Session manager
Tracks users and sessions

### systemd-networkd
Network configuration
Low-level network management

### systemd-resolved
DNS resolver
DNS-over-TLS support

### systemd-homed
User home directories
Per-user encryption and portability

### systemd-oomd
OOM manager
Kills apps before system freeze

### systemd-boot
Boot loader (UEFI)
Simple, fast bootloader

### dbus-broker
D-Bus message bus
Lower overhead than reference dbus-daemon

## Boot Targets

### default.target
Normal multi-user graphical boot
Symlink to graphical.target

### graphical.target
Graphical login (GDM)
Requires multi-user.target

### multi-user.target
Multi-user text mode
No GUI

### rescue.target
Single-user rescue mode
Minimal services

### emergency.target
Emergency shell
Almost nothing loaded

## Service Management

### Starting services
```bash
systemctl start <service>
systemctl enable <service>      # Start at boot
systemctl enable --now <service> # Start now + at boot
```

### Stopping services
```bash
systemctl stop <service>
systemctl disable <service>     # Don't start at boot
```

### Status
```bash
systemctl status <service>
systemctl is-active <service>
systemctl is-enabled <service>
```

### Logs
```bash
journalctl -u <service>
journalctl -f                   # Follow
journalctl -b                   # Current boot
journalctl -p err               # Errors only
```

## Essential Services

### Early boot (sysinit.target)
```
systemd-journald.service
systemd-udevd.service
systemd-tmpfiles-setup.service
systemd-sysctl.service
systemd-modules-load.service
```

### Basic system (basic.target)
```
systemd-resolved.service
systemd-timesyncd.service
dbus-broker.service
```

### Multi-user (multi-user.target)
```
systemd-logind.service
systemd-homed.service
systemd-oomd.service
NetworkManager.service
firewalld.service
```

### Graphical (graphical.target)
```
gdm.service                     # Login manager
accounts-daemon.service
```

## Disabled by Default

Services that start only when needed:

```
avahi-daemon.service            # mDNS (needs printer)
bluetooth.service               # BT (needs device)
cups.service                    # Printing (needs printer)
ModemManager.service            # WWAN (needs modem)
```

Activated by udev or user action.

## Service Configuration

Location: `/etc/systemd/system/`

Example unit file:
```ini
[Unit]
Description=Example Service
After=network.target

[Service]
Type=simple
ExecStart=/usr/bin/example-daemon
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
```

## Socket Activation

Services start on-demand via socket:

```ini
# example.socket
[Unit]
Description=Example Socket

[Socket]
ListenStream=9999
Accept=no

[Install]
WantedBy=sockets.target
```

```ini
# example.service
[Unit]
Description=Example Service
Requires=example.socket

[Service]
ExecStart=/usr/bin/example-daemon
```

Start socket, not service:
```bash
systemctl enable --now example.socket
```

Service starts when connection arrives.

## Timer Units

Cron replacement:

```ini
# example.timer
[Unit]
Description=Example Timer

[Timer]
OnBootSec=5min
OnUnitActiveSec=1h

[Install]
WantedBy=timers.target
```

```ini
# example.service
[Unit]
Description=Example Task

[Service]
Type=oneshot
ExecStart=/usr/bin/example-task
```

## cgroups v2

All processes organized in control groups:

```
/
├── init.scope                  # PID 1
├── system.slice               # System services
│   ├── systemd-journald.service
│   ├── NetworkManager.service
│   └── ...
├── user.slice                 # User sessions
│   └── user-1000.slice        # UID 1000
│       ├── session-2.scope    # Login session
│       └── app.slice          # User apps
│           └── flatpak-*.scope
└── machine.slice              # VMs/containers
```

### Resource Limits

Per-service limits:
```ini
[Service]
MemoryHigh=512M                # Soft limit
MemoryMax=768M                 # Hard limit
CPUQuota=50%                   # CPU limit
IOWeight=100                   # I/O priority
```

### systemd-oomd Configuration

```ini
# /etc/systemd/oomd.conf
[OOM]
SwapUsedLimit=80%
DefaultMemoryPressureLimit=60%
DefaultMemoryPressureDurationSec=10s
```

Kills lowest priority process under memory pressure.

## systemd-homed

Per-user encrypted home directories:

```bash
# Create user
homectl create arbor \
    --real-name="Arbor User" \
    --member-of=wheel \
    --storage=luks \
    --disk-size=50G

# User info
homectl inspect arbor

# Lock user
homectl lock arbor
```

Home directory is LUKS2 container.
Unmounts on lock/logout.

## Network Management

### systemd-networkd
Low-level network configuration

### systemd-resolved
DNS resolution

### NetworkManager
High-level network GUI
Preferred for desktop

Configuration:
```ini
# /etc/systemd/resolved.conf
[Resolve]
DNS=1.1.1.1#cloudflare-dns.com
DNSOverTLS=yes
DNSSEC=allow-downgrade
```

## Time Synchronization

```ini
# /etc/systemd/timesyncd.conf
[Time]
NTP=time.cloudflare.com time.google.com
FallbackNTP=pool.ntp.org
```

## Boot Analysis

### Boot time
```bash
systemd-analyze
systemd-analyze blame
systemd-analyze critical-chain
systemd-analyze plot > boot.svg
```

### Service dependencies
```bash
systemd-analyze verify <unit>
systemd-analyze dot | dot -Tsvg > dependencies.svg
```

## Shutdown/Reboot

```bash
systemctl poweroff
systemctl reboot
systemctl suspend
systemctl hibernate
```

## Troubleshooting

### Service won't start
```bash
systemctl status <service>
journalctl -u <service> -n 50
systemctl cat <service>         # Show unit file
```

### Boot fails
Boot to rescue.target:
```
# Kernel command line
systemd.unit=rescue.target
```

### Emergency mode
```
systemd.unit=emergency.target
```

Gives root shell, minimal services.

## Performance Tuning

### Parallel startup
Services start in parallel by default.
Dependencies control order.

### Socket activation
Services start on-demand.
Reduces boot time.

### Service isolation
Private /tmp per service:
```ini
[Service]
PrivateTmp=yes
```

## Security Features

### Service sandboxing
```ini
[Service]
PrivateTmp=yes
ProtectSystem=strict
ProtectHome=yes
NoNewPrivileges=yes
RestrictAddressFamilies=AF_INET AF_INET6
SystemCallFilter=@system-service
```

### Secure boot
systemd-boot supports Secure Boot
Verifies kernel signatures

## Monitoring

### System status
```bash
systemctl status
systemctl list-units
systemctl list-units --failed
```

### Resource usage
```bash
systemd-cgtop                   # Like top for cgroups
systemctl show <service>
```

## References

- Spec: §3.4 Init System
- systemd docs: https://systemd.io
- Arch Wiki: https://wiki.archlinux.org/title/Systemd
