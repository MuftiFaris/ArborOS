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

**Current Phase:** Phase 4 - Installer (In Progress)  
**Latest Working ISO:** ArborOS-phase3.iso (Phase 3 - Hardware Support)

ArborOS is under active development. Three phases completed with working bootable system including hardware support.

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
- ⚠️ **Phase 4** - Installer (Config ready, implementation pending)
- 📋 **Phase 5** - Desktop Environment
- 📋 **Phase 6** - Core Applications
- 📋 **Phase 7** - Privacy & User Control
- 📋 **Phase 8** - Update & Recovery
- 📋 **Phase 9** - Performance Optimization
- 📋 **Phase 10** - Final QA

## Current System Status

**ArborOS Phase 3** (`ArborOS-phase3.iso` - 863MB)

**Working Features:**
- ✅ Bootable live system (Fedora 39 base)
- ✅ Console login (arbor/arbor)
- ✅ Network connectivity (Ethernet, WiFi firmware)
- ✅ Hardware detection (`arbor-hwinfo` tool)
- ✅ Audio support (PipeWire stack)
- ✅ GPU drivers (Mesa for Intel/AMD)
- ✅ Bluetooth support (BlueZ)
- ✅ Power management
- ✅ Memory efficient (317MB idle RAM)
- ✅ ~340+ packages installed

**Phase 3 Additions:**
- Hardware detection tool
- Audio stack (PipeWire + WirePlumber + ALSA)
- GPU drivers (Intel/AMD Mesa, NVIDIA nouveau)
- WiFi/Bluetooth firmware
- Power management tools
- Hardware utilities (lspci, lsusb, lshw)

**Phase 4 Progress:**
- ⚠️ Calamares installer configs ready
- ⚠️ Implementation blocked (package unavailable)
- ⏸️ Deferred to Phase 5+ (needs desktop first)

**Known Limitations:**
- Console-only (no GUI) - Phase 5
- Manual installation required
- No applications yet - Phase 6
- Fedora branding visible (ArborOS branding pending)

See detailed documentation in `Documentation/07-Implementation/Phase Plan/`

## Quick Start

**Latest ISO:** ArborOS-phase3.iso (Phase 3 - Hardware Support)

**Testing in VirtualBox:**
```bash
# Load ISO
# RAM: 2GB minimum
# Boot
# Login: arbor / arbor

# Test hardware detection
sudo arbor-hwinfo

# Test network
ping google.com

# Check memory
free -h
```

**Building ISO (Ubuntu + Docker):**
```bash
# See build documentation
Documentation/07-Implementation/Phase Plan/Phase 3 - Hardware Support/README.md
```

Full documentation: `Documentation/07-Implementation/Phase Plan/`

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
