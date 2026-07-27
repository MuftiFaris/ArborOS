# Cara Cek & Test Arbor OS Phase 2

## Quick Start

### 1. Build ISO (Harus di Linux)

**Opsi A: Linux Native (Fedora/Ubuntu)**
```bash
cd /path/to/MyLinuxOS/Code
sudo ./build/create_iso.sh
```

**Opsi B: WSL2 (Windows Subsystem for Linux)**
```bash
# Di PowerShell/CMD Windows
wsl

# Di dalam WSL
cd /mnt/c/Users/mufti/Downloads/MyLinuxOS/Code
sudo ./build/create_iso.sh
```

**Waktu build:** 15-30 menit (download packages + compress)

**Output:** `Code/build/output/ArborOS-0.1.iso`

---

## 2. Test ISO

### Cara 1: QEMU (Otomatis)

**Di Linux/WSL:**
```bash
./tests/test_boot.sh
```

**Manual QEMU:**
```bash
qemu-system-x86_64 \
    -enable-kvm \
    -m 2G \
    -cdrom build/output/ArborOS-0.1.iso \
    -boot d
```

**Kontrol:**
- `Ctrl+Alt+G` - Release mouse
- `Ctrl+Alt+2` - QEMU monitor
- Ketik `quit` - Exit

---

### Cara 2: VirtualBox

**Setup:**
1. Buka VirtualBox
2. New VM:
   - Name: ArborOS-Test
   - Type: Linux
   - Version: Fedora (64-bit)
   - RAM: 2048 MB
   - No hard disk (live boot)
3. Settings:
   - Storage → Controller IDE → Add optical drive
   - Choose ISO: `ArborOS-0.1.iso`
4. Start VM

---

### Cara 3: VMware

**Setup:**
1. Buka VMware Workstation/Player
2. Create New VM:
   - Typical configuration
   - Linux → Fedora 64-bit
   - RAM: 2048 MB
3. Settings:
   - CD/DVD → Use ISO image
   - Browse ke `ArborOS-0.1.iso`
4. Power On

---

### Cara 4: USB Boot (Real Hardware)

**⚠️ HATI-HATI: Data di USB akan HILANG**

**Di Linux:**
```bash
# Cek USB device
lsblk

# Write ISO (ganti sdX dengan device USB lu)
sudo dd if=build/output/ArborOS-0.1.iso of=/dev/sdX bs=4M status=progress
sync
```

**Di Windows (Rufus):**
1. Download Rufus: https://rufus.ie
2. Select ArborOS-0.1.iso
3. Select USB device
4. DD Image mode
5. Start

**Boot:**
1. Colok USB
2. Restart komputer
3. Tekan F12/F2/Del (boot menu)
4. Pilih USB
5. Boot Arbor OS

---

## 3. Login & Cek Sistem

### Login

**User:** `arbor`  
**Password:** `arbor`

```
Arbor OS 0.1 Alpha
arbor-os login: arbor
Password: arbor

arbor@arbor-os:~$
```

---

### Cek Sistem Berjalan

**1. Cek versi kernel:**
```bash
uname -r
# Output: 6.x.x-xxx.fc39.x86_64
```

**2. Cek hostname:**
```bash
hostname
# Output: arbor-os
```

**3. Cek systemd:**
```bash
systemctl status
# Harus running, warna hijau
```

**4. Cek RAM usage:**
```bash
free -h
# Target: < 500 MB used
```

**5. Cek boot time:**
```bash
systemd-analyze
# Target: < 8 seconds total
```

**6. Cek service startup:**
```bash
systemd-analyze blame | head -20
# List service paling lama
```

---

### Cek Network

**1. Cek interface:**
```bash
ip link show
# Harus ada interface (enp0s3, eth0, dll)
```

**2. Cek IP address:**
```bash
ip addr show
# Harus dapet IP dari DHCP
```

**3. Ping gateway:**
```bash
ip route | grep default
ping -c 4 <gateway-ip>
```

**4. Ping internet (IP):**
```bash
ping -c 4 1.1.1.1
# Test koneksi tanpa DNS
```

**5. Ping internet (domain):**
```bash
ping -c 4 google.com
# Test DNS + koneksi
```

**6. Test HTTPS:**
```bash
curl -I https://example.com
# Harus dapat response 200
```

**7. Cek DNS:**
```bash
resolvectl status
# Harus configure DNS (1.1.1.1)
```

---

### Cek Services

**1. NetworkManager:**
```bash
systemctl status NetworkManager
# Active (running)
```

**2. SSH Server:**
```bash
systemctl status sshd
# Active (running)
```

**3. Firewall:**
```bash
sudo firewall-cmd --list-all
# public zone, ssh allowed
```

**4. systemd-resolved:**
```bash
systemctl status systemd-resolved
# Active (running)
```

---

### Cek User & Permissions

**1. Current user:**
```bash
whoami
# arbor
```

**2. User groups:**
```bash
groups
# arbor wheel
```

**3. Test sudo:**
```bash
sudo whoami
# root
```

**4. List users:**
```bash
cat /etc/passwd | grep -E "arbor|root"
```

---

### Cek Filesystem

**1. Disk usage:**
```bash
df -h
# Root (/) harus < 5 GB
```

**2. Mount points:**
```bash
findmnt
# Check /, /home, /tmp
```

**3. Btrfs check:**
```bash
sudo btrfs filesystem show
# Harus ada btrfs filesystem
```

---

## 4. Test Checklist

Copy checklist ini pas testing:

```
BOOT TEST:
[ ] ISO boots successfully
[ ] Kernel loads without panic
[ ] systemd starts
[ ] Login prompt appears
[ ] Boot time < 8 seconds

LOGIN TEST:
[ ] Can login as arbor
[ ] Password "arbor" works
[ ] Shell prompt appears
[ ] sudo works

NETWORK TEST:
[ ] Network interface detected
[ ] IP address assigned (DHCP)
[ ] Can ping gateway
[ ] Can ping 1.1.1.1
[ ] Can ping google.com
[ ] curl https://example.com works
[ ] DNS resolution works

SYSTEM TEST:
[ ] systemctl status - green
[ ] RAM usage < 500 MB
[ ] Services running correctly
[ ] Logs accessible (journalctl)

SHUTDOWN TEST:
[ ] sudo systemctl poweroff - works
[ ] Clean shutdown (no errors)

REBOOT TEST:
[ ] sudo systemctl reboot - works
[ ] System boots again successfully
```

---

## 5. Performance Testing

### Boot Time Analysis

```bash
# Total boot time
systemd-analyze

# Service by service
systemd-analyze blame

# Critical path
systemd-analyze critical-chain

# Generate graph
systemd-analyze plot > boot.svg
```

**Target:** < 8 seconds total

### Memory Analysis

```bash
# Simple
free -h

# Detailed per-process
ps aux --sort=-%mem | head -20

# systemd cgroup usage
systemd-cgtop
```

**Target:** < 500 MB used idle

### Disk Performance

```bash
# Write test
dd if=/dev/zero of=/tmp/test bs=1M count=1024 oflag=direct

# Read test
dd if=/tmp/test of=/dev/null bs=1M iflag=direct

# Clean up
rm /tmp/test
```

### Network Performance

```bash
# Download speed test
curl -o /dev/null http://speedtest.tele2.net/10MB.zip

# Show network stats
ip -s link show
```

---

## 6. Troubleshooting

### Boot Gagal

**Symptom:** Kernel panic / boot loop

**Fix:**
1. Cek QEMU/VM RAM (min 1GB)
2. Cek BIOS/UEFI mode
3. Disable Secure Boot di VM
4. Coba boot legacy (BIOS) mode

### Network Tidak Jalan

**Symptom:** No IP address

**Debug:**
```bash
# Restart NetworkManager
sudo systemctl restart NetworkManager

# Check logs
journalctl -u NetworkManager -f

# Manual DHCP
sudo dhclient <interface>
```

### Login Gagal

**Symptom:** Password ditolak

**User:** `arbor`  
**Password:** `arbor` (lowercase)

Root login dari console:
- User: `root`
- Password: `arbor`

### SSH Tidak Bisa Connect

**Debug:**
```bash
# Cek SSH running
systemctl status sshd

# Cek firewall
sudo firewall-cmd --list-all

# Manual add SSH
sudo firewall-cmd --add-service=ssh --permanent
sudo firewall-cmd --reload
```

### Systemd Error

**Debug:**
```bash
# Check failed units
systemctl --failed

# Check specific service
journalctl -u <service-name>

# View boot messages
journalctl -b
```

---

## 7. Advanced Testing

### Network dari Host ke VM

**VirtualBox:**
1. Settings → Network
2. Adapter 1: NAT
3. Port Forwarding:
   - Host: 127.0.0.1:2222
   - Guest: 10.0.2.15:22

**Connect:**
```bash
ssh -p 2222 arbor@127.0.0.1
```

### QEMU dengan Network

```bash
qemu-system-x86_64 \
    -enable-kvm \
    -m 2G \
    -cdrom ArborOS-0.1.iso \
    -boot d \
    -net nic,model=virtio \
    -net user,hostfwd=tcp::2222-:22
```

**Connect:**
```bash
ssh -p 2222 arbor@localhost
```

---

## 8. Reporting Issues

Kalau ada bug, catat:

**System Info:**
```bash
uname -a
cat /etc/os-release
free -h
df -h
```

**Logs:**
```bash
journalctl -b > boot.log
dmesg > kernel.log
systemctl --failed > failed-services.txt
```

**Network:**
```bash
ip addr > network.txt
ip route >> network.txt
resolvectl status >> network.txt
```

Kirim file-file ini + deskripsi bug.

---

## 9. Cek Sukses

Phase 2 SUKSES kalau:

✅ ISO builds tanpa error  
✅ Boots di QEMU  
✅ Boots di VirtualBox/VMware  
✅ Login works (arbor/arbor)  
✅ sudo works  
✅ Network dapat IP  
✅ Ping internet works  
✅ Boot time < 8s  
✅ RAM usage < 500 MB  
✅ Shutdown/reboot works  

---

## 10. Next Steps

Kalau Phase 2 works:

**Immediate:**
- Change default password: `passwd`
- Test lebih banyak hardware
- Document hasil testing

**Future:**
- Phase 3: WiFi + Graphics + More Hardware
- Phase 4: Desktop Environment
- Phase 5: Applications

---

**Need Help?**

Check dokumentasi lengkap:
- `BASE_SYSTEM_IMPLEMENTATION.md` - Full details
- `PHASE_2_TEST_REPORT.md` - Test template
- `Code/system/*.md` - Component docs

---

**Document Version:** 1.0  
**Last Updated:** 2026-07-26
