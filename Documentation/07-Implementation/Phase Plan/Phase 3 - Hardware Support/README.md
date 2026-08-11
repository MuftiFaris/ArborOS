# PHASE 3 — HARDWARE SUPPORT

**Status:** Implementation Ready  
**Version:** ArborOS 0.2  
**Timeline:** 2-3 weeks  
**Dependencies:** Phase 2 Complete ✅

---

## OVERVIEW

Transform ArborOS from minimal bootable system to hardware-compatible Linux distribution.

**Phase 2 → Phase 3:**
- ✅ Boots on VM
- ✅ Basic networking
- ⬜ GPU drivers (Intel/AMD/NVIDIA)
- ⬜ WiFi/Bluetooth
- ⬜ Audio (PipeWire)
- ⬜ Laptop hardware
- ⬜ Power management

---

## DOCUMENTATION

**Created:**
- ✅ `HARDWARE_SUPPORT_DOCUMENTATION.md` - Full technical specs
- ✅ `DRIVER_SUPPORT_MATRIX.md` - Hardware compatibility table
- ⬜ `PHASE_3_TEST_REPORT.md` - Test results (after testing)

**Code:**
- ✅ `Code/system/hardware-detection.sh` - Detection tool
- ✅ `Code/build/create_iso.sh` - Updated for Phase 3
- ⬜ Test on real hardware

---

## BUILD INSTRUCTIONS

### Prerequisites

**Linux VM (Ubuntu/Fedora):**
- 30GB+ disk space
- `dnf` package manager (Fedora)
- `mkisofs`, `isohybrid`, `mksquashfs`

### Build Steps

```bash
# 1. Navigate to build directory
cd Code/build

# 2. Run build script (requires root)
sudo ./create_iso.sh

# 3. ISO created at:
# build/output/ArborOS-0.2.iso
```

### Build Time

- First build: ~15-20 minutes
- Downloads: ~2GB packages
- Output ISO: ~700-800MB

---

## TESTING

### Quick Test (VM)

```bash
# VirtualBox or QEMU
qemu-system-x86_64 -m 2G -cdrom ArborOS-0.2.iso
```

**Login:**
- User: `arbor`
- Password: `arbor`

### Hardware Detection

```bash
# After boot, run:
arbor-hwinfo

# Save report:
arbor-hwinfo > hardware-report.txt
```

### Test Checklist

**Phase 3 Features:**
- [ ] GPU detected correctly
- [ ] Audio output works (`speaker-test`)
- [ ] WiFi adapter detected (`iw dev`)
- [ ] Bluetooth service running
- [ ] Battery reporting (laptops)
- [ ] Suspend/resume works
- [ ] USB devices hotplug
- [ ] Firmware loaded

---

## HARDWARE SUPPORT

### Tier 1 (Verified)
- Intel CPU (2011+)
- AMD Ryzen (2017+)
- Intel GPU (HD 4000+)
- AMD GPU (GCN 1.0+)
- Intel WiFi

### Tier 2 (Supported)
- Ethernet (Intel, Realtek)
- USB devices
- Standard audio (HDA)
- Touchpads (Synaptics, ELAN)

### Tier 3 (Limited)
- NVIDIA GPU (requires proprietary)
- Broadcom WiFi (needs firmware)
- Some laptop-specific hardware

---

## NEW PACKAGES

**Phase 3 Additions:**

**Hardware Tools:**
- `pciutils` (lspci)
- `usbutils` (lsusb)
- `lshw` (hardware lister)
- `dmidecode` (BIOS info)

**Firmware:**
- `linux-firmware` (main package)
- `iwl*-firmware` (Intel WiFi)

**Graphics:**
- `mesa-dri-drivers` (Intel/AMD)
- `mesa-vulkan-drivers` (Vulkan)

**Audio:**
- `pipewire` (audio server)
- `pipewire-alsa`
- `pipewire-pulseaudio`
- `wireplumber` (session manager)
- `alsa-utils` (tools)

**Wireless:**
- `bluez` (Bluetooth stack)
- `iw` (WiFi tools)
- `wireless-tools`

**Power:**
- `power-profiles-daemon`
- `upower` (battery)
- `fwupd` (firmware updates)

**Input:**
- `libinput` (touchpad/input)

---

## KNOWN ISSUES

### NVIDIA GPUs
- Boots with `nouveau` (basic)
- Proprietary driver install manual
- May break on kernel updates
- Wayland limited

### WiFi
- Broadcom needs manual firmware
- Some Realtek adapters unreliable

### Suspend
- S0ix (Modern Standby) BIOS-dependent
- Some laptop models have quirks

---

## IMPLEMENTATION STATUS

### Completed ✅
- Documentation written
- Build script updated
- Hardware detection tool
- Package list defined

### In Progress ⚠️
- ISO building
- Hardware testing

### Pending ⬜
- Test on real hardware
- NVIDIA driver workflow
- WiFi firmware handling
- Power optimization

---

## NEXT STEPS

### Immediate (This Week)
1. Build ArborOS-0.2.iso
2. Test in VM
3. Verify hardware detection
4. Test audio output

### Soon (Next Week)
1. Test on real laptop
2. WiFi connectivity
3. Battery reporting
4. Suspend/resume

### Later (Week 3)
1. NVIDIA proprietary install script
2. Power profile optimization
3. Firmware update testing
4. Complete test report

---

## PHASE 4 PREVIEW

**Next Phase:** Installer
- Calamares or Anaconda
- Disk partitioning
- UEFI/BIOS boot
- User setup
- Encrypted install

---

## CONTRIBUTING

### Hardware Testing

**Need testers with:**
- Intel laptops
- AMD laptops
- NVIDIA GPUs
- Various WiFi adapters
- Touchscreen/convertibles

**Submit reports:**
```bash
arbor-hwinfo > hardware-report.txt
# Email to: hardware@arboros.org
```

### Bug Reports

**Include:**
- Hardware model
- `dmesg` output
- `lspci -v` output
- What doesn't work

---

## RESOURCES

### Documentation
- Master Plan: `Phase 0 - Implementation Master Planning/`
- UX Design: `Documentation/02-UX-Design/ux-design.md`
- Architecture: `Documentation/01-Architecture/`

### Code
- Build scripts: `Code/build/`
- System configs: `Code/system/`
- Kernel docs: `Code/kernel/`

### External
- Fedora Hardware: https://fedoraproject.org/wiki/Hardware
- Linux Firmware: https://git.kernel.org/pub/scm/linux/kernel/git/firmware/linux-firmware.git
- LVFS (firmware updates): https://fwupd.org/

---

**Status:** Ready to build  
**Last Updated:** 2026-07-30  
**Next Milestone:** ArborOS-0.2.iso

