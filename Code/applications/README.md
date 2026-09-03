# ArborOS Applications — Phase 6

Unified application suite for ArborOS desktop environment. All applications share a consistent design system and architecture.

## Project Structure

```
applications/
├── CMakeLists.txt                 # Root CMake build configuration
├── design-system/                 # Arbor Design System library
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── color-palette.h        # Color definitions + theme switching
│   │   ├── theme.h                # Typography, spacing, animation constants
│   │   └── style-provider.h       # QSS stylesheet management
│   ├── src/
│   │   ├── color-palette.cpp
│   │   ├── theme.cpp
│   │   └── style-provider.cpp
│   ├── stylesheets/
│   │   ├── arbor-dark.qss         # Dark theme stylesheet
│   │   └── arbor-light.qss        # Light theme stylesheet
│   └── data/
│       └── color-palette.json     # Color definitions as JSON
│
├── arbor-files/                   # File Manager
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── file-manager.h
│   │   ├── tag-manager.h
│   │   └── preview-handler.h
│   ├── src/
│   │   ├── main.cpp
│   │   ├── file-manager.cpp
│   │   ├── tag-manager.cpp
│   │   └── preview-handler.cpp
│   └── icons/
│       └── arbor-files.svg
│
├── arbor-terminal/                # Terminal Emulator
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── terminal-widget.h
│   │   └── terminal-window.h
│   ├── src/
│   │   ├── main.cpp
│   │   ├── terminal-widget.cpp
│   │   └── terminal-window.cpp
│   └── icons/
│       └── arbor-terminal.svg
│
├── arbor-settings/                # Settings / Control Center
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── settings-window.h
│   │   ├── settings-model.h
│   │   └── system-info.h
│   ├── src/
│   │   ├── main.cpp
│   │   ├── settings-window.cpp
│   │   ├── settings-model.cpp
│   │   └── system-info.cpp
│   └── icons/
│       └── arbor-settings.svg
│
├── arbor-software-center/         # App Store
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── store-window.h
│   │   ├── app-store-model.h
│   │   └── flatpak-manager.h
│   ├── src/
│   │   ├── main.cpp
│   │   ├── store-window.cpp
│   │   ├── app-store-model.cpp
│   │   └── flatpak-manager.cpp
│   └── icons/
│       └── arbor-software-center.svg
│
├── arbor-update-manager/          # Update Manager
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── update-window.h
│   │   ├── ostree-manager.h
│   │   └── deployment-model.h
│   ├── src/
│   │   ├── main.cpp
│   │   ├── update-window.cpp
│   │   ├── ostree-manager.cpp
│   │   └── deployment-model.cpp
│   └── icons/
│       └── arbor-update-manager.svg
│
├── arbor-system-monitor/          # System Monitor
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── system-monitor-window.h
│   │   ├── system-stats.h
│   │   ├── graph-renderer.h
│   │   └── process-model.h
│   ├── src/
│   │   ├── main.cpp
│   │   ├── system-monitor-window.cpp
│   │   ├── system-stats.cpp
│   │   ├── graph-renderer.cpp
│   │   └── process-model.cpp
│   └── icons/
│       └── arbor-system-monitor.svg
│
└── tests/                         # Unit tests
    ├── CMakeLists.txt
    ├── test-color-palette.cpp
    └── test-theme.cpp
```

## Build Requirements

### System Dependencies

- **C++20 compiler** (GCC 11+, Clang 13+, MSVC 2019+)
- **CMake 3.24+**
- **Qt6** (Core, Gui, Widgets, DBus, Network, Sql, Concurrent, Svg)

### Installation

#### Fedora 39+ (Package Install)

```bash
sudo dnf install gcc-c++ cmake qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtsvg-devel
```

#### Ubuntu 22.04+ (Apt Install)

```bash
sudo apt install build-essential cmake qt6-base-dev qt6-declarative-dev qt6-svg-dev
```

#### macOS (Homebrew)

```bash
brew install cmake qt@6
```

#### Windows (MSVC)

- Visual Studio 2019+ with C++ tools
- Qt 6 from [qt.io/download](https://qt.io/download)

## Building

### Quick Build

```bash
cd Code/applications
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j$(nproc)
```

### Install to System

```bash
cmake --install . --prefix /usr/local
```

### Build Individual App

```bash
cmake --build . --target arbor-files
```

### Debug Build

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -j$(nproc)
```

### Run Tests

```bash
ctest --output-on-failure
```

## Architecture

### Design System Library (`arbor-design-system`)

Core library providing:
- **Color Palette:** Semantic colors (SurfaceBase, AccentPrimary, TextPrimary, etc.)
- **Typography:** Font definitions with Inter, JetBrains Mono
- **Spacing:** 8px grid system constants
- **Style Provider:** QSS stylesheet management + theme switching

All applications link against this library for consistent styling.

### Application Architecture (MVC Pattern)

Each application follows a consistent architecture:

```
┌─────────────────────────────────────────┐
│ View (Qt Widgets/QML)                   │
│ - Main Window                           │
│ - UI Components                         │
│ - Event Handlers                        │
└─────────────────────────────────────────┘
              ↕
┌─────────────────────────────────────────┐
│ Controller (Business Logic)             │
│ - Data validation                       │
│ - User interactions                     │
│ - System integration (D-Bus, etc.)      │
└─────────────────────────────────────────┘
              ↕
┌─────────────────────────────────────────┐
│ Model (Data Management)                 │
│ - File system access                    │
│ - Configuration files                   │
│ - Database queries                      │
└─────────────────────────────────────────┘
```

### Integration Points

**D-Bus Services:**
- NetworkManager (for network settings)
- systemd (for system info, power management)
- udisks2 (for storage management)

**File System:**
- `~/.config/arbor/` — Application config
- `~/.local/share/arbor/` — User data
- `/usr/share/arbor/` — System data

**Flatpak Sandboxing:**
- Each app declares required permissions
- Portal-based access to restricted resources

## Development Workflow

### Adding a New Feature

1. Create feature branch:
   ```bash
   git checkout -b feature/my-feature
   ```

2. Make changes and test locally:
   ```bash
   cd build
   cmake --build .
   ./bin/arbor-<app>
   ```

3. Run tests:
   ```bash
   ctest
   ```

4. Format code (if using clang-format):
   ```bash
   clang-format -i src/**/*.cpp include/**/*.h
   ```

5. Commit and push:
   ```bash
   git add <files>
   git commit -m "feature: description"
   git push origin feature/my-feature
   ```

### Code Style Guidelines

- **Naming:** CamelCase for classes, snake_case for functions/variables
- **Indentation:** 4 spaces
- **Comments:** Doxygen-style for public APIs
- **Formatting:** Clang-format with LLVM style preset

### Testing

- **Unit tests:** Test individual components (design system, data models)
- **Integration tests:** Test D-Bus interactions, file I/O
- **UI tests:** Manual testing for responsiveness, keyboard navigation

## Performance Targets

| Application | Metric | Target |
|---|---|---|
| Arbor Files | Launch time | <500ms |
| Arbor Terminal | Launch time | <200ms |
| Arbor Settings | Launch time | <300ms |
| Arbor Software Center | Launch time | <500ms |
| Arbor Update Manager | Launch time | <300ms |
| Arbor System Monitor | Memory footprint | <50MB |

## Accessibility

All applications are WCAG 2.1 AA compliant:
- ✅ AT-SPI2 screen reader support
- ✅ High contrast mode support
- ✅ Keyboard-only operation
- ✅ Visible focus indicators

## Troubleshooting

### CMake not finding Qt6

```bash
# Specify Qt6 path explicitly
cmake -DQt6_DIR=/usr/lib/cmake/Qt6 ..
```

### Missing dependencies

```bash
# Fedora
sudo dnf install qt6-qtbase-devel

# Ubuntu
sudo apt install qt6-base-dev
```

### Build errors

1. Clean build:
   ```bash
   rm -rf build
   mkdir build && cd build
   ```

2. Check compiler version:
   ```bash
   g++ --version  # Should be 11+
   ```

3. Enable verbose output:
   ```bash
   cmake --build . --verbose
   ```

## Contributing

See [CONTRIBUTING.md](../../CONTRIBUTING.md) for guidelines.

## License

ArborOS applications are licensed under the GPLv3+ license. See LICENSE file for details.

## References

- [Qt6 Documentation](https://doc.qt.io/qt-6/)
- [CMake Documentation](https://cmake.org/cmake/help/latest/)
- [Arbor OS Architecture](../../Documentation/01-Architecture/os-architecture-blueprint.md)
- [Arbor UX Design](../../Documentation/02-UX-Design/ux-design.md)
- [Phase 6 Specification](../../Documentation/07-Implementation/Phase%20Plan/Phase%206%20-%20Core%20Applications/CORE_APPLICATIONS_SPECIFICATION.md)

---

**Status:** Build system setup complete. Ready for individual app development.
