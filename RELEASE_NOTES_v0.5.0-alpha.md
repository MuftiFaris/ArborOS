# ArborOS v0.5.0-alpha - Phase 5: Desktop Environment

**Release Date:** August 26, 2026  
**Status:** Pre-alpha - Early testing phase

## Overview

ArborOS v0.5.0-alpha brings a complete, functional desktop environment to the live system. This release completes Phase 5 of the ArborOS roadmap and demonstrates a complete graphical OS experience.

**Download:** `ArborOS-0.5.0-alpha.iso` (1057 MB)

## What's New in v0.5.0

### Desktop Environment ✅
- **LXQt** - Lightweight Qt-based desktop environment
- **Openbox** - Minimalist window manager
- **LightDM** - Display manager with autologin
- **Auto-login** - User `arbor` boots directly to desktop

### Applications ✅
- **PCManFM-Qt** - File manager with GUI
- **QTerminal** - Terminal emulator
- **Featherpad** - Text editor
- **HTTop** - System monitor
- **Network Manager** - WiFi/Ethernet GUI
- **Settings tools** - LXQt Config, Brightness control

### System Features ✅
- **Memory Efficient** - 400MB idle RAM usage
- **Hardware Support** - GPU, Audio, WiFi, Bluetooth, Power management
- **Live Boot** - Full desktop from ISO (no installation)
- **Network Ready** - Ethernet + WiFi support
- **PipeWire Audio** - Modern audio stack

## System Requirements

**Minimum:**
- Processor: 64-bit x86-64 (any CPU)
- RAM: 2GB (1GB minimum, slow)
- Storage: USB 2.0+ (write ISO to USB)
- Display: VGA/HDMI compatible

**Recommended:**
- RAM: 4GB+
- Processor: Modern multi-core
- Storage: SSD for better performance

## Known Limitations

- Manual installation required (Calamares installer coming in Phase 6+)
- Limited applications (web browser, office apps planned for Phase 6)
- Fedora 39 branding (ArborOS branding in Phase 7)
- No swap partition configured
- Temporary dev credentials (`arbor:arbor`)

## Testing This Release

**VirtualBox:**
```bash
# 1. Create new VM (Linux / Fedora 64-bit)
# 2. Allocate 2GB+ RAM
# 3. Load ArborOS-0.5.0-alpha.iso
# 4. Boot and enjoy desktop!
```

**Physical USB:**
```bash
# Linux/Mac:
dd if=ArborOS-0.5.0-alpha.iso of=/dev/sdX bs=4M status=progress

# Windows:
# Use Rufus or similar USB writer tool
```

**QEMU:**
```bash
qemu-system-x86_64 -m 2G -cdrom ArborOS-0.5.0-alpha.iso
```

## Verification

Verify ISO integrity using SHA256:

```bash
sha256sum -c ArborOS-0.5.0-alpha.iso.sha256
```

**Expected checksum:**
```
CE985122B171CA0266CEF66FA5A8569F4C5592EC8830AEDBB5ED7B5720A07001  ArborOS-0.5.0-alpha.iso
```

## What's Included

**Phase 5 Features:**
- ✅ Bootable live ISO
- ✅ LXQt desktop environment
- ✅ Hardware detection tool (`arbor-hwinfo`)
- ✅ Network connectivity
- ✅ Audio support (PipeWire)
- ✅ GPU drivers (Intel/AMD)
- ✅ WiFi + Bluetooth support
- ✅ Power management
- ✅ ~600+ packages

**Completed Phases:**
- Phase 0: Master planning
- Phase 1: Repository foundation
- Phase 2: Base Linux system (bootable, 315MB)
- Phase 3: Hardware support (GPU, audio, power)
- Phase 4: Installer config (deferred)
- Phase 5: Desktop environment ✅

## What's Next (Phase 6+)

- 📋 Core Applications (browser, office, media)
- 📋 Privacy & User Control features
- 📋 Update & Recovery system
- 📋 Performance optimization
- 📋 Final QA & release polish

## Building from Source

**Requirements:**
- Ubuntu/Debian VM with Docker
- 10GB+ free disk space
- ~30-45 minutes build time

**Build:**
```bash
cd Code/build
chmod +x build-ubuntu.sh
sudo bash build-ubuntu.sh
```

See `Code/build/README.md` for detailed build instructions.

## Feedback & Issues

This is **pre-alpha software**. Issues and limitations are expected.

- **Report issues:** https://github.com/MuftiFaris/ArborOS/issues
- **Discussions:** https://github.com/MuftiFaris/ArborOS/discussions
- **Email:** muftifaris@github.com

## Credits

**ArborOS Development Team:**
- Architecture & Design: Full team
- Implementation: Multiple contributors
- Testing & Feedback: Community

**Based on:** Fedora 39, LXQt, Openbox, systemd

## Disclaimer

⚠️ **This is pre-alpha software under active development.** 

- NOT recommended for production use
- Data loss possible during testing
- Security features not yet finalized
- Use at your own risk

## License

MIT License - See LICENSE file for details

---

**Download:** ArborOS-0.5.0-alpha.iso (1057 MB)  
**SHA256:** CE985122B171CA0266CEF66FA5A8569F4C5592EC8830AEDBB5ED7B5720A07001

**Happy testing! 🌳**
