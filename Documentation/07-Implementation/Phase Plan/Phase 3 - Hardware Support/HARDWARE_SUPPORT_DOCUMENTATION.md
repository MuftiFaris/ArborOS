# PHASE 3 — HARDWARE SUPPORT DOCUMENTATION

**Status:** In Progress  
**Version:** ArborOS 0.2  
**Dependencies:** Phase 2 Complete (ArborOS-0.1.iso)  
**Timeline:** Months 6-14

---

## 1. OVERVIEW

Transform ArborOS-0.1 minimal bootable system into hardware-compatible distribution.

**Phase 2 Baseline:**
- Fedora 39 base
- Kernel 6.11.9
- 315MB RAM idle
- Basic networking

**Phase 3 Goals:**
- CPU support (Intel/AMD)
- GPU drivers (Intel/AMD/NVIDIA)
- WiFi/Bluetooth
- Audio (PipeWire)
- Laptop hardware
- Power management
- USB devices
- Firmware handling

---

## 2. CPU SUPPORT

### 2.1 Intel Processors

**Target:** Core 2nd gen+ (2011+)  
**Status:** Supported

**Kernel modules:**
- `intel_pstate` - CPU frequency scaling
- `intel_idle` - Power management
- `coretemp` - Temperature sensors
- `x86_pkg_temp_thermal` - Thermal management

**Features:**
- Turbo Boost
- SpeedStep (frequency scaling)
- Hyper-Threading
- AVX/AVX2/AVX-512

### 2.2 AMD Processors

**Target:** Ryzen 1st gen+ (2017+), FX series  
**Status:** Supported

**Kernel modules:**
- `amd_pstate` - CPU frequency
- `k10temp` - Temperature sensors
- `zenpower` - Enhanced monitoring (if available)

**Features:**
- Precision Boost
- Cool'n'Quiet
- SMT (Simultaneous Multithreading)

### 2.3 ARM Processors

**Target:** Raspberry Pi 4+, Pinebook Pro  
**Status:** Planned (Phase 4/6 evaluation)

---

## 3. GPU SUPPORT STRATEGY

### 3.1 Intel Graphics

**Target:** HD 4000+ (Ivy Bridge 2012+)  
**Status:** Full support (Tier: Verified)

**Driver:** Mesa `i915` / `xe` (Arc)

**Packages:**
```
mesa-dri-drivers
mesa-vulkan-drivers
intel-media-driver
libva-intel-driver
```

**Features:**
- Wayland native
- Hardware video decode (VAAPI)
- Vulkan support

### 3.2 AMD Graphics

**Target:** GCN 1.0+ (Radeon HD 7000 2012+)  
**Status:** Full support (Tier: Verified)

**Driver:** Mesa `amdgpu` / `radeonsi`

**Packages:**
```
mesa-dri-drivers
mesa-vulkan-drivers
mesa-vdpau-drivers
xorg-x11-drv-amdgpu
```

**Features:**
- Open source stack
- Wayland native
- FreeSync support
- Hardware decode (VAAPI/VDPAU)
- Vulkan 1.3

### 3.3 NVIDIA Graphics

**Target:** Maxwell+ (GTX 900 series 2014+)  
**Status:** Partial support (Tier: Limited)

**Strategy:** Auto-detect + optional proprietary

**Default driver:** `nouveau` (open source)
- Basic display
- No gaming performance
- Wayland support limited

**Optional driver:** Proprietary NVIDIA
- Install via curated RPM Fusion repo
- Auto-detected at install time
- Explicit user opt-in required
- Hard fallback to nouveau if failure
- **Must never brick boot**

**Turing+ (RTX 20 series+):**
- Open kernel modules available
- Better Wayland support

**Installation path:**
```bash
# Detection script runs at boot
# If NVIDIA detected: prompt user
arbor-nvidia-detect
# User chooses:
# 1. Keep nouveau (safe, basic)
# 2. Install proprietary (performance)
```

**Maintenance impact:**
- Kernel updates may break driver
- Requires DKMS rebuild
- Dedicated NVIDIA QA lane required

**User warning:**
"NVIDIA proprietary driver may break on kernel updates. Fallback to safe mode always available."

---

## 4. AUDIO SYSTEM

### 4.1 Architecture

**Stack:** ALSA → PipeWire → Apps

**Why PipeWire:**
- Low latency
- PulseAudio + JACK replacement
- Wayland native
- Better Bluetooth
- Per-app routing

### 4.2 Packages

```
pipewire
pipewire-alsa
pipewire-pulseaudio
pipewire-jack-audio-connection-kit
wireplumber
alsa-utils
```

### 4.3 Supported Hardware

**Internal audio:**
- Intel HDA
- AMD HD Audio
- Realtek codecs

**USB audio:**
- Class-compliant devices
- Audio interfaces
- USB headsets

**Bluetooth audio:**
- A2DP profile
- HSP/HFP profiles
- Low-latency codecs (aptX, LDAC)

### 4.4 Testing

```bash
# List devices
aplay -l
arecord -l

# Test speakers
speaker-test -c2

# PipeWire status
systemctl --user status pipewire pipewire-pulse wireplumber

# Volume control
wpctl status
```

---

## 5. NETWORK SUPPORT

### 5.1 Ethernet

**Supported chipsets:**
- Intel (e1000e, igb, ixgbe)
- Realtek (r8169)
- Broadcom (tg3, bnx2)
- Aquantia

**Driver:** Kernel built-in  
**Status:** Full support

### 5.2 WiFi

**Tier 1 (Verified):**
- Intel WiFi 6/6E/7 (iwlwifi)
- Intel WiFi 5 (iwlwifi)

**Tier 2 (Supported):**
- Atheros (ath9k, ath10k)
- Realtek (rtw88, rtw89)
- MediaTek (mt76, mt7921)

**Tier 3 (Limited):**
- Broadcom (brcmfmac) - requires firmware

**WPA3 support:** Yes (modern adapters)

**Firmware packages:**
```
linux-firmware
iwl*-firmware
```

### 5.3 Bluetooth

**Stack:** BlueZ 5.x

**Packages:**
```
bluez
bluez-tools
```

**Supported:**
- USB Bluetooth adapters
- Internal laptop Bluetooth
- Bluetooth 4.0+ LE

**Profiles:**
- A2DP (audio)
- HID (input devices)
- File transfer

**Testing:**
```bash
bluetoothctl
# In bluetoothctl:
# power on
# scan on
# pair XX:XX:XX:XX:XX:XX
```

---

## 6. LAPTOP HARDWARE

### 6.1 Input Devices

**Touchpad:**
- Synaptics
- ELAN
- FocalTech
- Precision touchpads (Windows)

**Driver:** libinput

**Packages:**
```
libinput
xorg-x11-drv-libinput
```

**Gestures:**
- 2-finger scroll
- 3-finger swipe (workspace)
- 4-finger pinch (overview)

**Keyboard:**
- Standard laptop keyboards
- Backlight control
- Function keys (Fn)

### 6.2 Display Features

**Brightness control:**
- Intel: `intel_backlight`
- AMD: `amdgpu_bl0`
- ACPI fallback

**Testing:**
```bash
# List backlight devices
ls /sys/class/backlight/

# Adjust brightness
echo 50 | sudo tee /sys/class/backlight/*/brightness
```

**External displays:**
- HDMI
- DisplayPort
- USB-C / Thunderbolt

### 6.3 Sensors

**Battery:**
- ACPI battery interface
- Charge thresholds (ThinkPad)

**Testing:**
```bash
upower -i /org/freedesktop/UPower/devices/battery_BAT0
```

**Accelerometer:**
- IIO subsystem
- Auto-rotate (tablets)

**Ambient light:**
- Auto-brightness

### 6.4 Special Hardware

**Fingerprint readers:**
- Supported via `fprintd`
- PAM integration

**Webcam:**
- UVC (USB Video Class)
- MIPI-CSI (Intel IPU6) - Tier: Limited
- Privacy LED support

**Smart card readers:**
- PC/SC support

---

## 7. POWER MANAGEMENT

### 7.1 CPU Frequency Scaling

**Governors:**
- `powersave` - Battery mode
- `performance` - AC mode
- `schedutil` - Dynamic (default)

**Intel P-State:**
```bash
# Check driver
cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_driver

# Current frequency
cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_cur_freq
```

**AMD P-State:**
- Similar interface
- EPP (Energy Performance Preference)

### 7.2 Suspend/Resume

**Supported states:**
- S0ix (Modern Standby) - Intel 6th gen+
- S3 (Suspend to RAM) - Traditional
- S4 (Hibernate) - Optional

**Testing:**
```bash
# Suspend
systemctl suspend

# Check sleep state
cat /sys/power/mem_sleep
# [s2idle] deep

# Prefer deep sleep if available
echo deep | sudo tee /sys/power/mem_sleep
```

**Hibernate:**
- Requires swap >= RAM
- LUKS complications
- Not enabled by default

### 7.3 Power Profiles

**Tool:** `power-profiles-daemon`

**Modes:**
- **Power Saver** - Max battery (scx_bpfland scheduler)
- **Balanced** - Default (scx_rustland)
- **Performance** - Max speed (scx_lavd + gaming)

**Packages:**
```
power-profiles-daemon
```

**Control:**
```bash
powerprofilesctl list
powerprofilesctl set power-saver
```

### 7.4 Battery Life Target

**Baseline:** Fedora 39 typical usage  
**Target:** +10-15% improvement

**Optimizations:**
- `CONFIG_HZ=1000` kernel
- zswap with LZ4
- `vm.swappiness=10`
- Aggressive device runtime PM
- WiFi power save
- PCIe ASPM

---

## 8. USB DEVICE SUPPORT

### 8.1 USB Controllers

**Supported:**
- xHCI (USB 3.x)
- EHCI (USB 2.0)
- UHCI/OHCI (USB 1.1)

**Features:**
- Hotplug
- Power management
- Fast charge

### 8.2 Common Devices

**Storage:**
- USB flash drives
- External HDDs/SSDs
- SD card readers

**Input:**
- Keyboards
- Mice
- Game controllers

**Other:**
- Printers (CUPS)
- Scanners (SANE)
- Webcams (UVC)
- Audio interfaces

**Testing:**
```bash
lsusb
usb-devices
```

---

## 9. FIRMWARE MANAGEMENT

### 9.1 Firmware Loading Strategy

**Location:** `/lib/firmware/`

**Source:** `linux-firmware` package

**Required firmware:**
- WiFi adapters (Intel, Realtek, Atheros)
- GPU (AMD)
- Bluetooth
- Some network cards

**Packages:**
```
linux-firmware
iwl*-firmware (Intel WiFi specific)
amd-gpu-firmware
```

**Licensing:**
- Most firmware is redistributable
- Some proprietary (NVIDIA, Broadcom)
- Bundled in base ISO

### 9.2 BIOS/UEFI Updates

**Tool:** `fwupd` (LVFS - Linux Vendor Firmware Service)

**Packages:**
```
fwupd
```

**Supported vendors:**
- Lenovo
- Dell
- HP
- Framework
- System76
- Many others

**Usage:**
```bash
# Check for updates
fwupdmgr get-updates

# Install updates
fwupdmgr update
```

**Benefits:**
- No Windows required
- Automatic check on boot
- Rollback support

---

## 10. HARDWARE DETECTION TOOLS

### 10.1 System Information

```bash
# CPU info
lscpu
cat /proc/cpuinfo

# Memory
free -h
dmidecode -t memory

# PCI devices (GPU, network, etc)
lspci -v

# USB devices
lsusb -v

# Block devices (disks)
lsblk
fdisk -l

# Network interfaces
ip link
nmcli device

# Kernel modules loaded
lsmod

# Hardware sensors
sensors
```

### 10.2 Diagnostic Scripts

Create `arbor-hwinfo` tool:
```bash
#!/bin/bash
# Arbor Hardware Information Collector

echo "=== ARBOR OS HARDWARE REPORT ==="
echo ""
echo "CPU:"
lscpu | grep -E 'Model name|CPU\(s\)|Thread|Core'
echo ""
echo "Memory:"
free -h
echo ""
echo "GPU:"
lspci | grep -i vga
lspci | grep -i 3d
echo ""
echo "Network:"
lspci | grep -i network
lspci | grep -i ethernet
echo ""
echo "Storage:"
lsblk -o NAME,SIZE,TYPE,FSTYPE,MOUNTPOINT
echo ""
echo "USB:"
lsusb
```

---

## 11. KERNEL CONFIGURATION

### 11.1 Hardware Support Modules

**Enable in kernel config:**

```
# CPU
CONFIG_X86_INTEL_PSTATE=y
CONFIG_X86_AMD_PSTATE=y

# Graphics
CONFIG_DRM=y
CONFIG_DRM_I915=m
CONFIG_DRM_AMDGPU=m
CONFIG_DRM_NOUVEAU=m

# Network
CONFIG_IWLWIFI=m
CONFIG_ATH9K=m
CONFIG_RTW88=m

# Audio
CONFIG_SND_HDA_INTEL=m
CONFIG_SND_HDA_CODEC_REALTEK=m

# USB
CONFIG_USB_XHCI_HCD=y
CONFIG_USB_STORAGE=m

# Input
CONFIG_INPUT_TOUCHSCREEN=y
CONFIG_TOUCHSCREEN_ELAN=m

# Power
CONFIG_CPU_FREQ_DEFAULT_GOV_SCHEDUTIL=y
CONFIG_SUSPEND=y
CONFIG_HIBERNATION=y
```

### 11.2 Module Loading

**Automatic:** `udev` + `systemd-modules-load`

**Manual:**
```bash
# Load module
modprobe module_name

# Blacklist module
echo "blacklist nouveau" > /etc/modprobe.d/blacklist-nouveau.conf
```

---

## 12. TESTING STRATEGY

### 12.1 Virtual Machine Testing

**Platform:** QEMU/KVM, VirtualBox

**Tests:**
- Boot with different CPU configs
- Virtual GPU (VirtIO, VMSVGA)
- Network (virtio-net)
- USB passthrough

### 12.2 Desktop Hardware Testing

**Required test systems:**

**Intel Desktop:**
- CPU: Core i5/i7
- GPU: Intel UHD
- Network: Ethernet
- Storage: NVMe

**AMD Desktop:**
- CPU: Ryzen 5/7
- GPU: Radeon RX
- Network: Ethernet + WiFi
- Storage: SATA SSD

### 12.3 Laptop Hardware Testing

**Required test laptops:**

**Intel Laptop:**
- ThinkPad or Dell XPS
- Intel WiFi
- Touchpad
- Battery
- Suspend/resume

**AMD Laptop:**
- Framework or ASUS
- AMD Radeon
- WiFi
- Touchpad
- Battery

**NVIDIA Laptop (optional):**
- Hybrid graphics
- Optimus test

### 12.4 Test Checklist

For each platform:

**Boot:**
- [ ] Cold boot
- [ ] Warm reboot
- [ ] UEFI boot
- [ ] Legacy BIOS boot

**Hardware Detection:**
- [ ] CPU recognized
- [ ] RAM amount correct
- [ ] Storage detected
- [ ] GPU identified
- [ ] Network adapters listed

**Functionality:**
- [ ] Display works
- [ ] Network connects
- [ ] Audio plays
- [ ] USB devices mount
- [ ] Touchpad responsive

**Laptop-specific:**
- [ ] Battery reporting accurate
- [ ] Brightness control works
- [ ] Suspend successful
- [ ] Resume successful
- [ ] WiFi connects
- [ ] Bluetooth pairs
- [ ] Function keys work

**Performance:**
- [ ] No excessive heat
- [ ] Fan control reasonable
- [ ] Battery life acceptable

---

## 13. TROUBLESHOOTING

### 13.1 No Display

**Check:**
```bash
# Framebuffer
cat /sys/class/graphics/fb0/name

# DRM
ls /dev/dri/

# X log (if applicable)
cat /var/log/Xorg.0.log
```

**Solutions:**
- Boot with `nomodeset`
- Update mesa/firmware
- Fallback GPU driver

### 13.2 No Network

**Check:**
```bash
ip link
dmesg | grep firmware
lspci | grep -i network
```

**Solutions:**
- Install missing firmware
- Load driver module
- Check rfkill

### 13.3 No Audio

**Check:**
```bash
aplay -l
systemctl --user status pipewire
pactl info
```

**Solutions:**
- Unmute in mixer
- Select correct device
- Restart pipewire

### 13.4 Suspend Fails

**Check:**
```bash
journalctl -b -u systemd-suspend
cat /sys/power/mem_sleep
```

**Solutions:**
- Update BIOS
- Kernel parameters
- Driver blacklist

---

## 14. DELIVERABLES

### 14.1 Updated ISO

**ArborOS-0.2.iso**

**Changes from 0.1:**
- Enhanced kernel modules
- Firmware packages
- PipeWire audio
- Power management
- libinput
- Hardware detection tools

### 14.2 Documentation

**Created:**
- ✅ HARDWARE_SUPPORT_DOCUMENTATION.md (this file)
- [ ] DRIVER_SUPPORT_MATRIX.md
- [ ] PHASE_3_TEST_REPORT.md

### 14.3 Code Updates

**Repository:**
```
Code/
├── build/
│   ├── create_iso.sh (updated)
│   └── hardware_test.sh (new)
├── kernel/
│   └── hardware-modules.conf (new)
├── configs/
│   └── firmware.conf (new)
└── system/
    └── hardware-detection.md (new)
```

---

## 15. NEXT STEPS

**Phase 3 Implementation:**
1. Update kernel config for hardware modules
2. Add firmware packages to ISO
3. Integrate PipeWire
4. Add power management tools
5. Create hardware detection script
6. Test on real hardware
7. Document results
8. Build ArborOS-0.2.iso

**Phase 4 Preview:**
- Installer (Calamares/Anaconda)
- Disk partitioning
- User setup wizard
- Bootloader installation

---

**Status:** Documentation complete, implementation pending  
**Next:** Begin kernel configuration updates  
**Timeline:** 2-3 weeks for full Phase 3

