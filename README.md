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

**Current Phase:** Phase 5 - Desktop Environment ✅ **COMPLETE**  
**Latest Working ISO:** ArborOS-0.5.iso (Phase 5 - LXQt Desktop)

ArborOS is under active development. Five phases completed with working desktop environment.

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
- ⚠️ **Phase 4** - Installer (Config ready, deferred to Phase 6+)
- ✅ **Phase 5** - Desktop Environment (LXQt, 400MB RAM, Working)
- 📋 **Phase 6** - Core Applications
- 📋 **Phase 7** - Privacy & User Control
- 📋 **Phase 8** - Update & Recovery
- 📋 **Phase 9** - Performance Optimization
- 📋 **Phase 10** - Final QA

## Current System Status

**ArborOS Phase 5** (`ArborOS-0.5.iso` - ~1.2-1.5 GB)

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
- ⚠️ Calamares installer configs ready
- ⏸️ Deferred to Phase 6+ (desktop now available)

**Known Limitations:**
- Manual installation required (installer coming in Phase 6+)
- Limited applications (browser, office coming in Phase 6)
- Fedora branding visible (ArborOS branding in Phase 7)

See detailed documentation in `Documentation/07-Implementation/Phase Plan/`

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

**Windows:**
```powershell
cd Code\build
.\build-final.ps1
```

**Linux:**
```bash
cd Code/build
chmod +x build-final.sh
sudo bash build-final.sh
```

Full documentation: `Code/build/README.md`

## Documentation

Comprehensive documentation covers:
- Architecture design and rationale
- Security model and implementation
- Performance optimization strategies
- Developer ecosystem and tooling
- Phase-by-phase implementation plans

Start with `Documentation/06-Brand/FINAL_OS_SPECIFICATION_V2.md` for complete OS overview.

## Contributing

ArborOS is in early development. See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

**Testing reports welcome!** Hardware compatibility feedback helps improve support.

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

**Disclaimer:** This is pre-alpha software under active development. Not recommended for production use. Use at your own risk.
