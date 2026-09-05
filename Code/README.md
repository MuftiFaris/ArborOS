# ArborOS Code Repository (Phases 1–7)

**Current Status:** Phase 7 - Privacy & User Control ✅ **COMPLETE**  
**Version:** 0.7 Pre-Alpha  
**Base Platform:** Fedora 39 / Linux 6.8+  

---

## 📂 Repository Structure

```
Code/
├── applications/             # Phase 6: 6 Qt6 Core Apps & Design System
│   ├── design-system/        # Semantic colors, typography, QSS stylesheets
│   ├── arbor-files/          # File Manager (Qt6)
│   ├── arbor-terminal/       # Terminal Emulator (Qt6)
│   ├── arbor-settings/       # Control Center (Qt6)
│   ├── arbor-software-center/# Flatpak App Store (Qt6)
│   ├── arbor-update-manager/ # OSTree Update Manager (Qt6)
│   ├── arbor-system-monitor/ # System Monitor (Qt6/Charts)
│   └── flatpak/              # Flatpak manifests & sandbox policies
│
├── privacy/                  # Phase 7: Privacy Subsystem & Dashboard
│   ├── src/                  # PrivacyManager daemon, hardware kill-switches, D-Bus
│   ├── include/              # C++ headers (Camera, Mic, File, Network controls)
│   ├── configs/              # D-Bus security policies (com.arbor.Privacy.conf)
│   └── test/                 # Privacy subsystem unit tests
│
├── system/                   # Phase 2: Base System & Init Configs
│   ├── boot-system.md        # systemd-boot / GRUB bootchain
│   ├── filesystem-layout.md  # Btrfs layout & subvolumes
│   ├── init-system.md        # systemd service definitions
│   └── networking.md         # NetworkManager & DNS setup
│
├── build/                    # ISO Build Pipeline (Phase 2-5)
│   ├── build-ubuntu.sh       # Docker-based ISO build (Tested & Working)
│   ├── build-final.sh        # LXQt live ISO build script (Linux)
│   ├── build-final.ps1       # PowerShell wrapper for ISO build
│   └── create_iso.sh         # Native Fedora ISO build script
│
├── installer/                # Phase 4: Calamares Installer Configs
│   ├── calamares/            # Module configs, partitioner, branding
│   └── install-calamares.sh  # Integration script (Package build pending)
│
├── kernel/                   # Kernel Configuration Strategy (Linux 6.8+)
├── packages/                 # Package lists (~600-700 core RPMs)
└── tests/                    # QEMU / system boot test scripts
```

---

## 🛠️ Build & Quick Start

### 1. Build Phase 6 Core Applications
```bash
cd Code/applications
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j$(nproc)
```

### 2. Build Phase 7 Privacy Subsystem & Dashboard
```bash
cd Code/privacy
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j$(nproc)
```

### 3. Build Live ISO Image (Phase 5 Desktop ISO)
```bash
cd Code/build
chmod +x build-ubuntu.sh
sudo bash build-ubuntu.sh
# Output: Code/build/output/ArborOS-0.5.iso
```

---

## 📄 Key Documentation

- [Code/IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md) — Master summary for Phases 1–7.
- [Code/applications/PHASE_6_COMPLETION_REPORT.md](applications/PHASE_6_COMPLETION_REPORT.md) — Phase 6 deliverables report.
- [Code/applications/FINAL_AUDIT_REPORT.md](applications/FINAL_AUDIT_REPORT.md) — Phase 6 code audit.
- [Code/privacy/PRIVACY_USER_GUIDE.md](privacy/PRIVACY_USER_GUIDE.md) — Phase 7 Privacy user manual.
