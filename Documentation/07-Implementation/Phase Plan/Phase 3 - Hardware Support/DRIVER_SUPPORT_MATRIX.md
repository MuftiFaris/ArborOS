# ARBOR OS — DRIVER SUPPORT MATRIX

**Version:** 0.2  
**Last Updated:** 2026-07-30  
**Phase:** 3 - Hardware Support

---

## SUPPORT TIERS

**Verified** ✅ - Tested, works out-of-box, recommended  
**Supported** ⚠️ - Should work, not fully tested  
**Limited** ⚡ - Partial support, may require manual setup  
**Planned** 🔜 - Future support  
**Not Supported** ❌ - Won't work

---

## CPU SUPPORT

| Manufacturer | Model | Status | Driver | Notes |
|---|---|---|---|---|
| Intel | Core 2nd gen+ (2011+) | ✅ Verified | intel_pstate | Full features |
| Intel | Atom/Celeron | ✅ Verified | intel_pstate | Low power optimized |
| Intel | Xeon | ⚠️ Supported | intel_pstate | Server CPUs |
| AMD | Ryzen 1000+ (2017+) | ✅ Verified | amd_pstate | Full features |
| AMD | EPYC | ⚠️ Supported | amd_pstate | Server CPUs |
| AMD | FX series | ⚠️ Supported | acpi-cpufreq | Legacy |
| ARM | Raspberry Pi 4+ | 🔜 Planned | N/A | Phase 4/6 evaluation |
| ARM | Apple Silicon | 🔜 Planned | Asahi Linux | Research phase |

---

## GPU SUPPORT

### Integrated Graphics

| Manufacturer | Model | Status | Driver | Vulkan | Wayland |
|---|---|---|---|---|---|
| Intel | HD 4000+ (2012+) | ✅ Verified | i915 (Mesa) | Yes | Yes |
| Intel | Arc A-series | ✅ Verified | xe (Mesa) | Yes | Yes |
| AMD | Vega APU | ✅ Verified | amdgpu (Mesa) | Yes | Yes |
| AMD | Ryzen 7000+ iGPU | ✅ Verified | amdgpu (Mesa) | Yes | Yes |

### Discrete Graphics

| Manufacturer | Model | Status | Driver | Vulkan | Wayland |
|---|---|---|---|---|---|
| AMD | Radeon RX 5000+ | ✅ Verified | amdgpu (Mesa) | Yes | Yes |
| AMD | Radeon RX 400-500 | ✅ Verified | amdgpu (Mesa) | Yes | Yes |
| AMD | Radeon HD 7000+ | ⚠️ Supported | radeon/amdgpu | Partial | Yes |
| NVIDIA | RTX 40 series | ⚡ Limited | nouveau/proprietary | Prop only | Limited |
| NVIDIA | RTX 30 series | ⚡ Limited | nouveau/proprietary | Prop only | Limited |
| NVIDIA | RTX 20 series (Turing+) | ⚡ Limited | nouveau/open/prop | Prop only | Partial |
| NVIDIA | GTX 16/10 series | ⚡ Limited | nouveau/proprietary | Prop only | No |
| NVIDIA | GTX 900 series | ⚡ Limited | nouveau/proprietary | Prop only | No |
| NVIDIA | GTX 700 series or older | ❌ Not Supported | nouveau only | No | No |

**NVIDIA Notes:**
- Default: nouveau (basic display, no gaming)
- Optional: proprietary driver (requires manual install, may break on updates)
- Turing+ (RTX 20+): Open kernel modules available
- Wayland: Requires proprietary + explicit-sync (RTX 20+)

---

## NETWORK SUPPORT

### Ethernet

| Manufacturer | Chipset | Status | Driver | Speed |
|---|---|---|---|---|
| Intel | I219/I225/I226 | ✅ Verified | e1000e/igb | 1G/2.5G |
| Intel | X550/X710 | ⚠️ Supported | ixgbe | 10G |
| Realtek | RTL8111/8125 | ✅ Verified | r8169 | 1G/2.5G |
| Broadcom | NetXtreme | ⚠️ Supported | tg3/bnx2 | 1G |
| Aquantia | AQC107/113 | ⚠️ Supported | atlantic | 5G/10G |

### WiFi

| Manufacturer | Model | Status | Driver | WiFi 6/6E | WPA3 |
|---|---|---|---|---|---|
| Intel | AX200/AX201/AX211 | ✅ Verified | iwlwifi | Yes | Yes |
| Intel | AX210 (WiFi 6E) | ✅ Verified | iwlwifi | Yes | Yes |
| Intel | AC 9260/8265 | ✅ Verified | iwlwifi | No | Yes |
| Atheros | QCA6174/9377 | ⚠️ Supported | ath10k | No | Yes |
| Realtek | RTL8852AE/BE | ⚠️ Supported | rtw89 | Yes | Partial |
| Realtek | RTL8822CE | ⚠️ Supported | rtw88 | No | Partial |
| MediaTek | MT7921/MT7922 | ⚠️ Supported | mt76 | Yes | Yes |
| Broadcom | BCM4350/4360 | ⚡ Limited | brcmfmac | No | No |

**Firmware required:** All WiFi adapters need `/lib/firmware/` blobs

---

## AUDIO SUPPORT

| Type | Hardware | Status | Driver | Notes |
|---|---|---|---|---|
| Internal | Intel HDA | ✅ Verified | snd_hda_intel | Most laptops/desktops |
| Internal | AMD HD Audio | ✅ Verified | snd_hda_intel | Ryzen systems |
| Internal | Realtek codecs | ✅ Verified | snd_hda_codec_realtek | ALC series |
| USB | Class-compliant | ✅ Verified | snd_usb_audio | Most USB headsets |
| USB | Audio interfaces | ⚠️ Supported | snd_usb_audio | DACs, mixers |
| Bluetooth | A2DP | ✅ Verified | PipeWire/BlueZ | High-quality playback |
| Bluetooth | HSP/HFP | ⚠️ Supported | PipeWire/BlueZ | Call audio |

**Stack:** ALSA → PipeWire → Applications

---

## BLUETOOTH SUPPORT

| Type | Status | Driver | Profiles |
|---|---|---|---|
| USB Bluetooth adapters | ✅ Verified | btusb | All |
| Internal laptop BT | ✅ Verified | btusb/btintel | All |
| Bluetooth 4.0+ LE | ✅ Verified | BlueZ 5.x | BLE |
| Bluetooth 3.0 or older | ⚠️ Supported | BlueZ 5.x | Limited |

**Profiles supported:**
- A2DP (audio streaming)
- HID (keyboards, mice)
- OBEX (file transfer)
- HSP/HFP (headset)

---

## INPUT DEVICES

### Touchpads

| Manufacturer | Status | Driver | Gestures |
|---|---|---|---|
| Synaptics | ✅ Verified | libinput | Full |
| ELAN | ✅ Verified | libinput | Full |
| FocalTech | ⚠️ Supported | libinput | Full |
| Windows Precision | ✅ Verified | libinput | Full |

**Gestures:**
- 2-finger scroll
- 3-finger swipe
- 4-finger pinch
- Palm rejection

### Keyboards

| Type | Status | Notes |
|---|---|---|
| Standard laptop keyboards | ✅ Verified | Full support |
| Backlit keyboards | ✅ Verified | Brightness control |
| Function keys (Fn) | ⚠️ Supported | Vendor-specific |
| Mechanical keyboards | ✅ Verified | USB/Bluetooth |

---

## LAPTOP HARDWARE

### Battery & Power

| Feature | Status | Notes |
|---|---|---|
| Battery reporting | ✅ Verified | ACPI interface |
| AC adapter detection | ✅ Verified | ACPI |
| Charge thresholds | ⚡ Limited | ThinkPad supported |
| Battery health | ⚠️ Supported | Via `upower` |

### Display

| Feature | Status | Notes |
|---|---|---|
| Brightness control | ✅ Verified | Keyboard + software |
| External displays (HDMI) | ✅ Verified | Hot-plug |
| External displays (DP) | ✅ Verified | Hot-plug |
| USB-C displays | ⚠️ Supported | DP Alt Mode |
| Thunderbolt displays | ⚠️ Supported | Requires `bolt` |

### Sensors

| Sensor | Status | Driver | Notes |
|---|---|---|---|
| Accelerometer | ⚠️ Supported | iio | Tablets/convertibles |
| Ambient light | ⚡ Limited | iio | Auto-brightness |
| Gyroscope | ⚠️ Supported | iio | Convertibles |

### Biometrics

| Device | Status | Driver | Integration |
|---|---|---|---|
| Fingerprint (Synaptics) | ⚠️ Supported | fprintd | PAM |
| Fingerprint (Goodix) | ⚡ Limited | fprintd | Partial |
| Webcam privacy shutter | ✅ Verified | Hardware | N/A |

---

## STORAGE

| Type | Status | Driver | TRIM | Notes |
|---|---|---|---|---|
| NVMe SSD | ✅ Verified | nvme | Yes | Fastest |
| SATA SSD | ✅ Verified | ahci | Yes | Standard |
| SATA HDD | ✅ Verified | ahci | N/A | Legacy |
| eMMC | ⚠️ Supported | mmc_block | Partial | Chromebooks |
| USB storage | ✅ Verified | usb-storage | N/A | Flash drives |
| SD cards | ✅ Verified | mmc/sd | N/A | Card readers |

---

## USB DEVICES

| Device Type | Status | Notes |
|---|---|---|
| USB 3.2 Gen 2 (10 Gbps) | ✅ Verified | xHCI controller |
| USB 3.0/3.1 (5 Gbps) | ✅ Verified | xHCI controller |
| USB 2.0 | ✅ Verified | EHCI/xHCI |
| USB-C | ✅ Verified | Data + display |
| USB hubs | ✅ Verified | Powered recommended |
| USB keyboards/mice | ✅ Verified | HID |
| USB webcams (UVC) | ✅ Verified | Standard |
| USB audio | ✅ Verified | Class-compliant |
| USB printers | ⚠️ Supported | CUPS |

---

## PRINTERS & SCANNERS

| Type | Status | Driver | Notes |
|---|---|---|---|
| IPP (driverless) | ✅ Verified | CUPS | Modern printers |
| HP printers | ⚠️ Supported | HPLIP | Requires setup |
| Brother printers | ⚠️ Supported | CUPS + PPD | Manual driver |
| Canon printers | ⚠️ Supported | CUPS + PPD | Manual driver |
| Epson printers | ⚠️ Supported | CUPS + PPD | Manual driver |
| Network scanners (eSCL) | ⚠️ Supported | SANE | Driverless |
| USB scanners | ⚡ Limited | SANE | Vendor-specific |

---

## POWER MANAGEMENT

| Feature | Status | Notes |
|---|---|---|
| CPU frequency scaling | ✅ Verified | intel_pstate/amd_pstate |
| Suspend to RAM (S3) | ✅ Verified | Traditional laptops |
| Modern Standby (S0ix) | ⚠️ Supported | Intel 6th gen+ |
| Hibernate (S4) | ⚡ Limited | Requires swap partition |
| Runtime PM | ✅ Verified | Device-level power save |
| Power profiles | ✅ Verified | power-profiles-daemon |

---

## FIRMWARE UPDATES

| Vendor | Status | Method | Notes |
|---|---|---|---|
| Lenovo | ✅ Verified | fwupd/LVFS | ThinkPad, ThinkCentre |
| Dell | ✅ Verified | fwupd/LVFS | XPS, Latitude, Precision |
| HP | ⚠️ Supported | fwupd/LVFS | EliteBook, ProBook |
| Framework | ✅ Verified | fwupd/LVFS | Framework Laptop |
| System76 | ✅ Verified | fwupd/LVFS | Native Linux vendor |
| ASUS | ⚡ Limited | fwupd/LVFS | Partial coverage |
| Acer | ⚡ Limited | fwupd/LVFS | Partial coverage |
| MSI | ⚡ Limited | Manual | Limited LVFS |

---

## TESTING STATUS

| Hardware Category | Test Coverage | Status |
|---|---|---|
| Intel Desktop | ✅ Tested | VirtualBox |
| AMD Desktop | ⚠️ Pending | Need hardware |
| Intel Laptop | ⚠️ Pending | Need hardware |
| AMD Laptop | ⚠️ Pending | Need hardware |
| NVIDIA GPU | ⚠️ Pending | Need hardware |
| WiFi adapters | ⚠️ Pending | Need hardware |
| Touchpad | ⚠️ Pending | Need hardware |
| Battery/Suspend | ⚠️ Pending | Need hardware |

---

## MINIMUM HARDWARE REQUIREMENTS

**Standard Mode:**
- CPU: x86_64, 2 cores, 2011 or newer
- RAM: 8 GB
- Storage: 64 GB (NVMe/SSD recommended)
- GPU: Any with kernel driver
- Network: Ethernet or supported WiFi

**Developer Mode:**
- CPU: 4 cores recommended
- RAM: 16 GB
- Storage: 256 GB SSD
- GPU: Intel/AMD for best experience

**Recommended:**
- CPU: Intel Core i5/i7 or AMD Ryzen 5/7
- RAM: 16 GB
- Storage: 512 GB NVMe
- GPU: Intel/AMD integrated or discrete
- WiFi: Intel AX200+

---

## KNOWN ISSUES

**NVIDIA:**
- Proprietary driver may break on kernel updates
- Wayland limited without explicit-sync
- nouveau has poor gaming performance

**WiFi:**
- Broadcom adapters need manual firmware install
- Some Realtek adapters unreliable

**Suspend:**
- S0ix (Modern Standby) needs BIOS support
- Some Dell/HP models have quirks

**Touchpad:**
- Some precision touchpads need calibration

---

## REPORTING ISSUES

**Hardware not listed:**
1. Boot ArborOS live ISO
2. Run: `arbor-hwinfo > hardware-report.txt`
3. Submit to: hardware@arboros.org

**Hardware not working:**
1. Collect logs: `dmesg > dmesg.log`
2. List devices: `lspci -v > lspci.log`
3. Submit both files with hardware model

---

**Last Updated:** 2026-07-30  
**Next Update:** After Phase 3 testing on real hardware

