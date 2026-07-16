# Arbor OS Networking - Phase 2

## Requirements

- Ethernet support (primary)
- Network interface detection
- DHCP automatic configuration
- DNS resolution
- Basic connectivity testing

WiFi improvements in Phase 3.

## Network Stack

```
Application
  ↓
NetworkManager (high-level control)
  ↓
systemd-networkd (low-level config)
  ↓
systemd-resolved (DNS)
  ↓
Kernel networking
  ↓
Hardware
```

## NetworkManager

**Why:**
- Desktop-standard
- Automatic configuration
- Easy DHCP
- Connection profiles
- CLI + future GUI support

### Configuration

Enable:
```bash
systemctl enable NetworkManager
systemctl start NetworkManager
```

### Ethernet Auto-Config

NetworkManager detects wired interfaces automatically.
DHCP enabled by default.

```bash
# Check status
nmcli device status

# Show connections
nmcli connection show

# Connect
nmcli connection up <connection-name>
```

## systemd-resolved

DNS resolver with DNS-over-TLS support.

### Configuration

`/etc/systemd/resolved.conf`:
```ini
[Resolve]
DNS=1.1.1.1#cloudflare-dns.com 1.0.0.1
FallbackDNS=8.8.8.8 8.8.4.4
DNSOverTLS=opportunistic
DNSSEC=allow-downgrade
Domains=~.
```

Privacy-respecting DNS:
- Cloudflare 1.1.1.1 (primary)
- Google 8.8.8.8 (fallback)

DNS-over-TLS when supported.

Enable:
```bash
systemctl enable systemd-resolved
systemctl start systemd-resolved
```

### /etc/resolv.conf

Symlink to systemd-resolved:
```bash
ln -sf /run/systemd/resolve/stub-resolv.conf /etc/resolv.conf
```

## Network Interfaces

### Interface Naming

Predictable names (systemd):
```
enp0s31f6           # Ethernet (PCI bus 0, slot 31, func 6)
wlp3s0              # WiFi (PCI bus 3, slot 0)
lo                  # Loopback
```

### List Interfaces

```bash
ip link show
nmcli device status
```

### Ethernet Configuration

Automatic DHCP via NetworkManager.

Manual static IP:
```bash
nmcli connection modify <connection> \
    ipv4.method manual \
    ipv4.addresses 192.168.1.100/24 \
    ipv4.gateway 192.168.1.1 \
    ipv4.dns "8.8.8.8 8.8.4.4"
```

## DHCP

DHCPv4 and DHCPv6 support.

NetworkManager's internal DHCP client.

Automatic:
- IP address
- Gateway
- DNS servers
- Search domains

## DNS Resolution

### Test

```bash
# systemd-resolved status
resolvectl status

# DNS query
dig example.com
nslookup example.com
host example.com

# Test resolution
ping google.com
```

### DNS Cache

systemd-resolved caches DNS.

Clear cache:
```bash
resolvectl flush-caches
```

## Firewall

### nftables Backend

Modern firewall (replaces iptables).

### firewalld

High-level firewall management.

Configuration:
```bash
systemctl enable firewalld
systemctl start firewalld
```

### Default Policy

**Zone:** public
**Incoming:** block (except SSH)
**Outgoing:** allow all

```bash
firewall-cmd --set-default-zone=public
firewall-cmd --permanent --add-service=ssh
firewall-cmd --reload
```

### Common Services

Allow HTTP/HTTPS (web server):
```bash
firewall-cmd --permanent --add-service=http
firewall-cmd --permanent --add-service=https
firewall-cmd --reload
```

Check rules:
```bash
firewall-cmd --list-all
```

## Network Testing

### Connectivity Tests

**Loopback:**
```bash
ping -c 4 127.0.0.1
```

**Gateway:**
```bash
ping -c 4 $(ip route | grep default | awk '{print $3}')
```

**Internet:**
```bash
ping -c 4 1.1.1.1           # IP (no DNS)
ping -c 4 google.com        # DNS + routing
```

**HTTP:**
```bash
curl -I https://example.com
curl https://icanhazip.com  # Show public IP
```

### Network Info

**IP addresses:**
```bash
ip addr show
ip -4 addr show             # IPv4 only
ip -6 addr show             # IPv6 only
```

**Routes:**
```bash
ip route show
ip -6 route show
```

**DNS:**
```bash
resolvectl status
cat /etc/resolv.conf
```

**Connections:**
```bash
ss -tuln                    # Listening ports
ss -tn                      # TCP connections
```

## SSH Server

Remote access enabled by default.

### Configuration

`/etc/ssh/sshd_config`:
```
Port 22
PermitRootLogin no
PasswordAuthentication yes
PubkeyAuthentication yes
X11Forwarding no
AllowUsers arbor
```

Root login disabled (security).

### Service

```bash
systemctl enable sshd
systemctl start sshd
```

### Firewall

```bash
firewall-cmd --permanent --add-service=ssh
firewall-cmd --reload
```

### Usage

From another machine:
```bash
ssh arbor@<ip-address>
```

Copy files:
```bash
scp file.txt arbor@<ip>:/home/arbor/
```

## MAC Address

### Show MAC

```bash
ip link show
```

### Privacy

MAC randomization (Phase 3+):
```bash
nmcli connection modify <connection> \
    802-11-wireless.cloned-mac-address random
```

## IPv6

Enabled by default.

Disable if needed:
```bash
# Kernel parameter
ipv6.disable=1
```

## Network Drivers

### Ethernet

Most Intel, Realtek, Broadcom drivers in mainline kernel.

Common modules:
```
e1000e              # Intel
r8169               # Realtek
tg3                 # Broadcom
```

### WiFi (Phase 3)

```
iwlwifi             # Intel
ath10k              # Qualcomm Atheros
rtw88               # Realtek
```

## Troubleshooting

### No Network

1. Check interface exists:
```bash
ip link show
```

2. Check interface is up:
```bash
ip link set <interface> up
```

3. Check NetworkManager:
```bash
systemctl status NetworkManager
nmcli device status
```

4. Check DHCP:
```bash
journalctl -u NetworkManager
```

5. Manual IP:
```bash
ip addr add 192.168.1.100/24 dev <interface>
ip route add default via 192.168.1.1
```

### DNS Not Working

1. Check resolved:
```bash
systemctl status systemd-resolved
resolvectl status
```

2. Check /etc/resolv.conf:
```bash
ls -l /etc/resolv.conf
cat /etc/resolv.conf
```

3. Test DNS directly:
```bash
dig @1.1.1.1 example.com
```

4. Restart resolved:
```bash
systemctl restart systemd-resolved
```

### Firewall Blocking

1. Check rules:
```bash
firewall-cmd --list-all
```

2. Temporarily disable:
```bash
systemctl stop firewalld
```

3. If works, add exception:
```bash
firewall-cmd --permanent --add-port=<port>/tcp
firewall-cmd --reload
```

## Performance

### Network Speed Test

```bash
# Download test
curl -o /dev/null http://speedtest.tele2.net/100MB.zip

# iperf3 (if available)
iperf3 -c speedtest.example.com
```

### Network Stats

```bash
ip -s link show <interface>
```

## Security

### Encrypted DNS

DNS-over-TLS configured in systemd-resolved.

Verify:
```bash
resolvectl status | grep "DNS over TLS"
```

### SSH Keys

Generate keypair:
```bash
ssh-keygen -t ed25519 -C "arbor@arbor-os"
```

Copy public key:
```bash
ssh-copy-id arbor@<remote-host>
```

Disable password auth after key setup:
```
# /etc/ssh/sshd_config
PasswordAuthentication no
```

## Testing Checklist

Phase 2 networking complete when:

✅ Ethernet detected
✅ DHCP assigns IP
✅ DNS resolution works
✅ Can ping gateway
✅ Can ping 1.1.1.1
✅ Can ping google.com
✅ Can curl https://example.com
✅ SSH server accessible
✅ Firewall active

## References

- Spec: §3.7 Networking
- NetworkManager: https://networkmanager.dev
- systemd-resolved: https://www.freedesktop.org/software/systemd/man/systemd-resolved.html
