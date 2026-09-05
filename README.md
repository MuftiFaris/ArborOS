# ArborOS

```
          /\
         /  \          
        / /\ \         "Change anything. Break nothing."
       / /  \ \
      /_/    \_\
```

Privacy-focused, user-controlled Linux distribution built from the ground up.

## Project Status

**Current Phase:** Phase 7 - Privacy & User Control ✅ **COMPLETE (8/8 tasks)**  
**Latest Working ISO:** ArborOS-0.5.iso (Phase 5 - LXQt Desktop)  
**Latest Development:** Phase 7 Privacy & User Control (8/8 tasks complete, PrivacyManager, Dashboard, UI, Audited)

ArborOS is under active development. Seven phases completed with working desktop environment, core applications, and zero-trust privacy system.

## What is ArborOS?

ArborOS is a Linux-based operating system designed with three core principles:

1. **Privacy First** - User data stays with the user
2. **User Control** - Full transparency and control over system behavior  
3. **No Hidden Agendas** - Open development, clear intentions

## Repository Structure

```
ArborOS/
├── Code/                    # Implementation
│   ├── kernel/             # Kernel configuration
│   ├── system/             # System layout & init
│   ├── packages/           # Package management
│   ├── build/              # Build scripts
│   └── tests/              # Testing framework
└── Documentation/          # Design & planning
    ├── 01-Architecture/    # OS architecture blueprint
    ├── 02-UX-Design/       # User experience design
    ├── 03-Security/        # Security architecture
    ├── 04-Developer/       # Developer ecosystem
    ├── 05-Performance/     # Performance engineering
    ├── 06-Brand/           # Brand identity & specs
    └── 07-Implementation/  # Phase-by-phase implementation
```

## Development Phases

- ✅ **Phase 0** - Implementation Master Planning
- ✅ **Phase 1** - Repository Foundation  
- ✅ **Phase 2** - Base Linux System (Bootable, Network, 315MB RAM)
- ✅ **Phase 3** - Hardware Support (GPU, Audio, WiFi firmware, Power management)
- ⚠️ **Phase 4** - Installer (Calamares Configs & Branding Complete, Live ISO Package Build Pending)
- ✅ **Phase 5** - Desktop Environment (LXQt, 400MB RAM, Working)
- ✅ **Phase 6** - Core Applications (ALL COMPLETE: 11 tasks, 8,736 lines, flawless)
- ✅ **Phase 7** - Privacy & User Control (ALL COMPLETE: 8 tasks, PrivacyManager, Dashboard, UI, Audited)
- 📋 **Phase 8** - Update & Recovery
- 📋 **Phase 9** - Performance Optimization
- 📋 **Phase 10** - Final QA

## Current System Status

**ArborOS Phase 5 (Stable)** (`ArborOS-0.5.iso` - ~1.2-1.5 GB)  
**ArborOS Phase 6 & 7 (Complete)** - Branch: `feature/privacy-user-control`

**Working Features:**
- ✅ Bootable live system (Fedora 39 base)
- ✅ LXQt desktop environment with GUI
- ✅ LightDM display manager with autologin
- ✅ Network connectivity (Ethernet, WiFi firmware)
- ✅ Hardware detection (`arbor-hwinfo` tool)
- ✅ Audio support (PipeWire stack)
- ✅ GPU drivers (Mesa for Intel/AMD)
- ✅ Bluetooth support (BlueZ)
- ✅ Power management
- ✅ Memory efficient (400MB idle RAM)
- ✅ File manager, terminal, text editor
- ✅ ~600-700 packages installed

**Phase 5 Additions:**
- LXQt lightweight desktop environment
- LightDM display manager
- Openbox window manager
- PCManFM-Qt file manager
- QTerminal terminal emulator
- Featherpad text editor
- Network Manager GUI applet
- Breeze icon theme

**Phase 4 Status:**
- ⚠️ Calamares installer configs & branding complete; installer binary package pending RPM Fusion/custom build integration
- ✅ GUI & Desktop Environment runtime dependencies resolved in Phase 5 & 6

**Known Limitations:**
- GUI Installer package integration pending (Calamares package build deferred; live boot ISO available).
- Third-party web browser & office suite integration coming in future updates.

**Phase 6 & Phase 7 Documentation:**
- `Code/applications/PHASE_6_COMPLETION_REPORT.md` - Phase 6 Core Applications report
- `Code/applications/FINAL_AUDIT_REPORT.md` - Phase 6 Audit report
- `Code/privacy/PRIVACY_USER_GUIDE.md` - Phase 7 Privacy framework user guide
- `Code/privacy/PRIVACY_BEST_PRACTICES.md` - Phase 7 Security best practices
- `Documentation/07-Implementation/Phase Plan/` - Comprehensive phase-by-phase documentation

## Quick Start

**Latest ISO:** ArborOS-0.5.iso (Phase 5 - Desktop Environment)

**Testing in VirtualBox:**
```bash
# Create VM:
# - Type: Linux / Fedora (64-bit)
# - RAM: 2GB minimum
# - Load ArborOS-0.5.iso

# Boot to desktop (auto-login)
# Login: arbor / arbor (if needed)

# Test desktop:
# - Open file manager
# - Launch terminal
# - Test network connectivity
```

**Building ISO:**

**Ubuntu/Debian VM (Recommended - Working):**
```bash
cd Code/build
chmod +x build-ubuntu.sh
sudo bash build-ubuntu.sh
```
- Uses Docker (Ubuntu/Fedora compatible)
- Builds in ~30-45 minutes
- Output: `Code/build/output/ArborOS-0.5.iso`
- **Status:** ✅ Tested & working

**Alternative: Fedora (Native)**
```bash
cd Code/build
chmod +x create_iso.sh
sudo bash create_iso.sh
```
- Native Fedora build (no Docker needed)
- Output: `Code/build/output/ArborOS-0.5.iso`
- **Status:** ⚠️ Available, not tested

Full documentation: `Code/build/README.md`

## Documentation

Comprehensive documentation covers:
- Architecture design and rationale
- Security model and implementation
- Performance optimization strategies
- Developer ecosystem and tooling
- Phase-by-phase implementation plans

Start with `Documentation/06-Brand/FINAL_OS_SPECIFICATION_V2.md` for complete OS overview.

## Development Branches

- `main` - Stable Phase 5 (LXQt desktop environment)
- `feature/desktop-environment` - Phase 6 core applications (16 commits, ready for merge)
- `feature/hardware-support` - Phase 3 hardware support
- `feature/installer` - Phase 4 installer configuration

## Contributing

ArborOS is in early development. See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

**Testing reports welcome!** 
- Hardware compatibility feedback improves support
- Phase 6 application testing and feedback appreciated
- Accessibility testing (screen readers, keyboard navigation) needed

## License

MIT License - see [LICENSE](LICENSE) file for details.

## Project Links

- **Repository:** https://github.com/MuftiFaris/ArborOS
- **Issues:** https://github.com/MuftiFaris/ArborOS/issues
- **Releases:** https://github.com/MuftiFaris/ArborOS/releases

## Contact

**Maintainer:** Mufti Faris  
**GitHub:** [@MuftiFaris](https://github.com/MuftiFaris)

For questions or discussion, open an issue.

---

## Phase 6 & 7 Quickstart

To build and test **Phase 6 Core Applications** and **Phase 7 Privacy Subsystem & Dashboard**:

### Building Phase 6 Core Applications
```bash
cd Code/applications
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j$(nproc)

# Run apps:
./arbor-files/arbor-files
./arbor-terminal/arbor-terminal
./arbor-settings/arbor-settings
./arbor-software-center/arbor-software-center
./arbor-update-manager/arbor-update-manager
./arbor-system-monitor/arbor-system-monitor
```

### Building Phase 7 Privacy Framework & Dashboard
```bash
cd Code/privacy
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j$(nproc)

# Run Privacy Subsystem Tests & Dashboard:
./test/privacy_tests
./src/arbor-privacy-dashboard
```

**Requirements:**
- Qt6 development libraries (Core, Gui, Widgets, Sql, Network, DBus, Charts)
- CMake 3.24+
- C++20 compatible compiler (GCC 11+, Clang 13+)
- Linux with X11/Wayland & D-Bus system bus

---

**Disclaimer:** This is pre-alpha software under active development. Not recommended for production use. Use at your own risk.

