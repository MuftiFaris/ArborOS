# ArborOS Phase 6 - Flatpak Deployment Guide

**Target:** Package all 6 core applications as Flatpak bundles  
**Base Runtime:** KDE Platform 5.15  
**Status:** Manifests ready, build instructions provided

---

## Overview

Flatpak provides:
- Sandboxed environment for security
- Automatic updates
- Single-command installation
- Cross-distribution compatibility (Fedora, Ubuntu, Debian, Arch, openSUSE)

---

## 1. Flatpak Manifests

All apps use KDE Platform 5.15 runtime (stable, well-tested).

### Application Mappings

| App | App ID | Manifest | Permissions |
|-----|--------|----------|-------------|
| Arbor Files | org.arboros.Files | org.arboros.Files.json | Home, /mnt, /media |
| Arbor Terminal | org.arboros.Terminal | org.arboros.Terminal.json | Home, network |
| Arbor Settings | org.arboros.Settings | org.arboros.Settings.json | D-Bus (systemd, NetworkManager) |
| Arbor Software Center | org.arboros.SoftwareCenter | org.arboros.SoftwareCenter.json | Network, Flatpak D-Bus |
| Arbor Update Manager | org.arboros.UpdateManager | org.arboros.UpdateManager.json | Network, PackageKit |
| Arbor System Monitor | org.arboros.SystemMonitor | org.arboros.SystemMonitor.json | /proc, /sys access |

### Permission Analysis

#### org.arboros.Files
```json
"--share=ipc"                    // Shared memory with other apps
"--socket=x11"                   // X11 display
"--socket=wayland"               // Wayland display
"--device=dri"                   // GPU access for rendering
"--share=network"                // Network (optional, for network filesystems)
"--filesystem=home"              // Full home directory access
"--filesystem=/mnt"              // Mount points
"--filesystem=/media"            // USB drives
```

#### org.arboros.Terminal
```json
"--filesystem=home"              // Full home access (terminal needs it)
"--share=network"                // Network support
// Inherit display permissions from X11/Wayland
```

#### org.arboros.Settings
```json
"--system-talk-name=org.freedesktop.NetworkManager"  // WiFi settings
"--system-talk-name=org.freedesktop.UDisks2"        // Storage settings
"--system-talk-name=org.freedesktop.timedate1"      // Date/time
"--system-talk-name=org.freedesktop.login1"         // Power settings
"--talk-name=org.gnome.SettingsDaemon.Power"        // Power management
"--talk-name=org.gnome.SettingsDaemon.Color"        // Color management
```

#### org.arboros.SoftwareCenter
```json
"--share=network"                // Download apps
"--system-talk-name=org.flatpak.Flatpak"  // Install/uninstall Flatpaks
```

#### org.arboros.UpdateManager
```json
"--share=network"                // Download updates
"--system-talk-name=org.freedesktop.systemd1"  // System services
"--talk-name=org.freedesktop.PackageKit"       // Package management
```

#### org.arboros.SystemMonitor
```json
"--filesystem=host-os"           // Access /proc, /sys for metrics
// Read-only access to system information
```

---

## 2. Building Flatpak Bundles

### Prerequisites

```bash
# Install Flatpak tools
sudo apt install flatpak flatpak-builder  # Ubuntu/Debian
sudo dnf install flatpak flatpak-builder  # Fedora
sudo zypper install flatpak flatpak-builder  # openSUSE

# Add Flathub remote
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo

# Install KDE Runtime
flatpak install flathub org.kde.Platform//5.15 org.kde.Sdk//5.15
```

### Build Process

```bash
cd Code/applications/flatpak

# Build individual apps
flatpak-builder --user --force-clean build org.arboros.Files.json
flatpak-builder --user --force-clean build org.arboros.Terminal.json
flatpak-builder --user --force-clean build org.arboros.Settings.json
flatpak-builder --user --force-clean build org.arboros.SoftwareCenter.json
flatpak-builder --user --force-clean build org.arboros.UpdateManager.json
flatpak-builder --user --force-clean build org.arboros.SystemMonitor.json

# Flags:
# --user: Install for current user only
# --force-clean: Clean build directory
# --finish-args: Override finish arguments
# --share=network: Allow network access
```

### Build Output

Each build produces:
- `.flatpak` file (binary bundle, ~100-200 MB each)
- Checksum file for verification
- Build logs for debugging

```
build-dir/
├── arbor-files.flatpak
├── arbor-terminal.flatpak
├── arbor-settings.flatpak
├── arbor-software-center.flatpak
├── arbor-update-manager.flatpak
└── arbor-system-monitor.flatpak
```

---

## 3. Installation

### Install Single App

```bash
flatpak install --user arbor-files.flatpak
flatpak list --app  # Verify installation
```

### Run App

```bash
# Via Flatpak
flatpak run org.arboros.Files

# Via .desktop file (automatic after install)
arbor-files
```

### Update App

```bash
flatpak update org.arboros.Files
```

### Uninstall App

```bash
flatpak uninstall org.arboros.Files
```

---

## 4. Repository Setup (Flathub)

### Prepare for Flathub Submission

1. **Create GitHub repository:**
   ```
   ArborOS/org.arboros.Files
   ArborOS/org.arboros.Terminal
   ArborOS/org.arboros.Settings
   ArborOS/org.arboros.SoftwareCenter
   ArborOS/org.arboros.UpdateManager
   ArborOS/org.arboros.SystemMonitor
   ```

2. **Add manifest:** `flatpak/org.arboros.*.json` in repo root

3. **Create release:** Tag version (e.g., v0.5.0)

4. **Verify metadata:**
   ```bash
   flatpak run --command=flatpak-builder-lint manifest org.arboros.Files.json
   ```

### Submit to Flathub

1. Fork [Flathub repository](https://github.com/flathub/flathub)
2. Add new directory: `new-apps/org.arboros.Files/`
3. Submit PR with manifest
4. Wait for review (typically 1-2 weeks)
5. After approval, app available on Flathub

---

## 5. Quality Assurance

### Testing Checklist

#### Installation
- [ ] `flatpak install` succeeds without errors
- [ ] App appears in application menu
- [ ] .desktop file created
- [ ] Icon displays correctly
- [ ] App launches from menu

#### Functionality
- [ ] All core features work
- [ ] No crashes on startup
- [ ] File access works (Files app)
- [ ] Network requests work (Software Center)
- [ ] D-Bus calls work (Settings app)

#### Permissions
- [ ] No unnecessary permissions requested
- [ ] Sandbox working (tested with `--verbose`)
- [ ] No files accessed outside granted paths
- [ ] Network access only when needed

#### Performance
- [ ] App launches in <3 seconds
- [ ] Memory usage reasonable (<200 MB)
- [ ] CPU usage low at idle
- [ ] No memory leaks

#### Updates
- [ ] Updates install cleanly
- [ ] No data loss during update
- [ ] Version number bumps work
- [ ] Rollback possible

### Test Commands

```bash
# Verbose output (see sandbox violations)
flatpak run --verbose org.arboros.Files

# Monitor D-Bus calls
busctl monitor org.arboros.Files

# Check resource usage
flatpak run --devel org.arboros.Files

# Test file access limits
flatpak run --filesystem=home org.arboros.Files  # Should work
flatpak run --filesystem=  org.arboros.Files     # Should fail to access files

# Check for leaks
flatpak run --env=G_DEBUG=gc-friendly \
            --env=MALLOC_CHECK_=3 \
            org.arboros.Files
```

---

## 6. Continuous Integration

### GitHub Actions Workflow

```yaml
name: Build Flatpak
on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      
      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install flatpak flatpak-builder
          flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
          flatpak install -y flathub org.kde.Platform//5.15 org.kde.Sdk//5.15
      
      - name: Build Flatpaks
        run: |
          for manifest in flatpak/org.arboros.*.json; do
            flatpak-builder --user --force-clean build "$manifest"
          done
      
      - name: Upload artifacts
        uses: actions/upload-artifact@v2
        with:
          name: flatpak-builds
          path: build/
```

### Version Management

```bash
# Semantic versioning
VERSION="0.5.0"

# Update manifests
sed -i "s/\"version\": \".*\"/\"version\": \"$VERSION\"/" flatpak/org.arboros.*.json

# Tag release
git tag v$VERSION
git push origin v$VERSION

# Build for release
for app in Files Terminal Settings SoftwareCenter UpdateManager SystemMonitor; do
  flatpak-builder --user --force-clean build flatpak/org.arboros.$app.json
done
```

---

## 7. Troubleshooting

### Build Failures

**"Runtime not found"**
```bash
# Install KDE runtime
flatpak install flathub org.kde.Platform//5.15 org.kde.Sdk//5.15
```

**"Manifest validation failed"**
```bash
# Validate manifest syntax
flatpak run --command=json.tool org.gnome.jsonrpc < org.arboros.Files.json
```

**"Build timeout"**
```bash
# Increase timeout
flatpak-builder --build-shell=org.arboros.Files build flatpak/org.arboros.Files.json
```

### Runtime Issues

**"Permission denied" errors**
```bash
# Check allowed permissions
flatpak info org.arboros.Files | grep "Permissions"

# Update permissions in manifest
flatpak override --user --filesystem=home org.arboros.Files
```

**"D-Bus method not found"**
```bash
# Verify D-Bus service available
gdbus call --system --dest org.freedesktop.NetworkManager \
           --object-path /org/freedesktop/NetworkManager \
           --method org.freedesktop.DBus.Properties.Get \
           org.freedesktop.NetworkManager State

# Check manifest has permission
grep "system-talk-name" flatpak/org.arboros.Settings.json
```

**"File not accessible"**
```bash
# Check filesystem permissions
flatpak info org.arboros.Files | grep "Filesystem"

# Override for testing
flatpak run --filesystem=/tmp org.arboros.Files
```

---

## 8. Distribution

### Hosting Options

| Option | Cost | Maintenance | Discovery |
|--------|------|-------------|-----------|
| Flathub | Free | Low | ✅ Built-in |
| Personal server | $5-20/mo | Medium | Via website |
| GitHub Releases | Free | Low | Via GitHub |
| Archlinux repos | Free | Community-maintained | Repository search |

### Recommended: Flathub

- Largest Flatpak repository (~2000 apps)
- Automatic discovery on app stores
- Built-in update infrastructure
- Community review process

---

## 9. Maintenance

### Release Cycle

```
Phase 6.0 (current) → Phase 6.1 (bugfixes) → Phase 7.0 (next features)
    ↓
  Flatpak builds → Flathub submission → Community testing → Release
```

### Update Strategy

```bash
# Patch release (0.5.0 → 0.5.1)
# - Bugfixes, translations, minor improvements
# - No manifest changes needed
# - Auto-update via Flatpak

# Minor release (0.5.0 → 0.6.0)
# - New features, new apps, new UI
# - May need permission updates
# - Manual Flathub PR review

# Major release (0.5.0 → 1.0.0)
# - Significant changes, new architecture
# - Runtime upgrade possible (5.15 → 6.0)
# - Careful testing required
```

---

## 10. Security Considerations

### Sandbox Isolation

Flatpak provides:
- Read-only `/usr` (system files)
- Sandboxed home directory (optional)
- Restricted network access
- No D-Bus access by default
- Seccomp filtering

### Best Practices

1. **Request minimal permissions:** Only request what's needed
2. **Review D-Bus access:** D-Bus methods can be powerful
3. **Keep runtime updated:** Security patches in runtime
4. **Test sandbox:** Verify app still works with --nofilesystem
5. **Document permissions:** Explain why each permission needed

### Example: Minimal File Manager

```json
"--filesystem=home",     // Needs full home access
"--filesystem=/mnt",     // Mount points
"--filesystem=/media"    // USB/removable media
```

---

## Checklist: Phase 6 Flatpak Deployment

- [x] Create 6 app manifests
- [x] Define permissions matrix
- [ ] Build test bundles
- [ ] Test on multiple distributions
- [ ] Verify permissions working
- [ ] Performance benchmarks
- [ ] Documentation complete
- [ ] Submit to Flathub
- [ ] Set up CI/CD pipeline
- [ ] Create release notes
- [ ] Monitor feedback

---

## References

- [Flatpak Documentation](https://docs.flatpak.org/)
- [Flatpak Manifest Format](https://docs.flatpak.org/en/latest/manifests.html)
- [Flathub Submission Guide](https://docs.flathub.org/docs/for-app-authors/)
- [D-Bus Access Control](https://docs.flatpak.org/en/latest/sandbox-more-concepts.html#d-bus)

---

**Status:** Ready for build testing  
**Next:** Task #11 - QA testing and verification
