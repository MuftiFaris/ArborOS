# PHASE 6 — CORE APPLICATIONS SPECIFICATION

**Phase:** 6 - Core Applications  
**Version:** 1.0  
**Status:** SPECIFICATION (Ready for Implementation)  
**Date:** 2026-09-02

---

## Overview

Phase 6 designs and implements the core application suite for ArborOS. These applications form the foundation of the user experience — replacing default LXQt apps with ArborOS-specific, design-consistent alternatives that embody the "Quiet Precision" philosophy.

**Design Goals:**
- Single coherent design system (Arbor Design System v1)
- Lightweight but polished (match/better LXQt performance)
- Sandboxed where applicable (Flatpak-compatible)
- Keyboard-first + mouse-friendly
- Accessibility native (AT-SPI2 / screen reader support)

---

## Architecture Principles

### 1. Design System Foundation

All applications share a unified design language rooted in:

**Visual Identity:**
- Color palette: Dark theme default (Surface Base `#0F1215`), Light theme optional
- Typography: Inter for UI, JetBrains Mono for code/terminal
- Components: Line-based icons (24x24px, 2px stroke), squircle app icons
- Motion: Spring physics (stiffness: 300, damping: 30) for all transitions
- Layout: 8px grid system, 16px padding baseline, 4px micro-spacing

**Interaction Paradigm:**
- Keyboard navigation always available (Tab, Arrow keys, Enter)
- Focus rings visible and high-contrast (`#2DD4BF` 2px outline)
- Right-click context menus for secondary actions
- Tooltips on hover (100ms delay minimum)
- Undo/Redo supported where applicable (Ctrl+Z / Ctrl+Shift+Z)

**Accessibility Baseline:**
- Full AT-SPI2 semantic tree (screen reader compatible)
- Minimum contrast ratio 4.5:1 (WCAG AA)
- Motion-reduced mode (respect `prefers-reduced-motion`)
- Keyboard-only operation (no mouse required)
- Focus trapping in modals

### 2. Technology Stack

**Framework:**
- **Primary:** Qt6 (LXQt foundation carries forward, but with custom Arbor theming)
- **Rationale:** Lightweight, consistent, matches desktop environment (LXQt from Phase 5)

**Alternative (Future):**
- GTK4 variant track for GNOME Shell compatibility (Phase 7+)
- Flatpak wrapping for sandboxed distribution

**Build & Packaging:**
- CMake build system (Qt6 standard)
- Flatpak metadata for sandboxing declarations
- Reproducible builds (deterministic timestamps, sorted file lists)

### 3. Architecture Layers

```
┌─────────────────────────────────────────────────────────────┐
│ PRESENTATION LAYER (Qt6 QML/QWidgets)                        │
│ ├─ Main Window / UI Components                              │
│ ├─ Theme Engine (Arbor Style Plugin)                        │
│ └─ Accessibility Bridge (AT-SPI2)                           │
├─────────────────────────────────────────────────────────────┤
│ APPLICATION LOGIC LAYER                                      │
│ ├─ Model/View separation (MVC pattern)                      │
│ ├─ Settings persistence (xdg-dirs, config files)            │
│ └─ Error handling + logging                                 │
├─────────────────────────────────────────────────────────────┤
│ SYSTEM INTEGRATION LAYER                                    │
│ ├─ D-Bus services (systemd, NetworkManager, udisks2)        │
│ ├─ XDG Desktop Portals (if Flatpak sandboxed)               │
│ └─ freedesktop standards (icon themes, mimetypes)           │
├─────────────────────────────────────────────────────────────┤
│ OS LAYER (systemd, PipeWire, NetworkManager, etc.)           │
└─────────────────────────────────────────────────────────────┘
```

---

## Core Applications

### 1. Arbor Files (File Manager)

**Current State:** PCManFM-Qt from LXQt (basic, functional)  
**Goal:** Polished, tag-based, preview-capable file manager

#### Design

**Primary Navigation:**
```
+─────────────────────────────────────────────────────┐
│ [◀] [▶] /home/user/Documents/Projects/    [🔍] [≡] │
├──────────────┬─────────────────────────────────────┤
│ PLACES       │ NAME             SIZE    MODIFIED    │
│ ⌂ Home       │ 📁 Project_A     --      2 min ago  │
│ 📥 Downloads │ 📄 Report.pdf    2.4MB  Yesterday  │
│ 📄 Documents │ 📄 Sketch.fig    1.1MB  3 days ago │
│              │                                     │
│ TAGS         │                                     │
│ ● Active     │                                     │
│ ● Review     │                                     │
│ ● Archive    │                                     │
└──────────────┴─────────────────────────────────────┘
```

**Key Features:**

1. **Quick Look Preview (Spacebar)**
   - Instant floating overlay preview for:
     - Images (JPEG, PNG, WebP, HEIC)
     - PDFs (page thumbnails, text extraction)
     - Videos (frame grabber, duration)
     - Markdown (rendered)
     - Code (syntax-highlighted)
     - Audio (waveform, duration)
   - Implementation: Leverage Qt's image/PDF libraries + custom MIME type handlers
   - UX: Non-modal, dismissible with ESC, arrow keys navigate files while preview open

2. **Tagging System (Non-Destructive Organization)**
   - Visual tag sidebar with colored dots + editable names
   - Drag files onto tags to tag (creates symlink/database entry, not file move)
   - Multiple tags per file supported
   - Right-click file → "Add Tag..." → color + name picker
   - Implementation: Simple SQLite database per-user (`~/.local/share/arbor-files/tags.db`), symlink structure optional for cross-app compatibility

3. **Cloud Storage Integration (Phase 6.2, optional for v1)**
   - Nextcloud/ProtonDrive/Google Drive mount in sidebar (via FUSE or native API)
   - Offline sync indicator badge
   - Implementation: Use existing FUSE drivers (NextCloud client) + custom sidebar integration

4. **Security Actions (Right-Click Context Menu)**
   - "Encrypt with Vault": Creates password-protected Btrfs subvolume or VeraCrypt container
   - "Shred File": Secure deletion (overwrite with random data 3x, then delete)
   - "Lock" / "Unlock" for encrypted volumes
   - Implementation: Leverage `shred` CLI + cryptsetup/veracrypt CLI as backends

5. **Search & Filter**
   - Real-time search across current directory + subdirectories
   - Filter by: name, size range, date range, file type, tag
   - Implementation: Qt's file system model + custom filter proxy

**Keyboard Shortcuts:**
- `Space` — Preview current file
- `Ctrl+T` — Add tag to selection
- `Ctrl+N` — New folder
- `Ctrl+X/C/V` — Cut/copy/paste
- `Del` — Delete (Trash)
- `Shift+Del` — Permanent delete
- `F2` — Rename
- `Ctrl+H` — Toggle hidden files

**Metrics:**
- Initial launch: <500ms
- Directory listing (1000 files): <200ms
- Search (10k files): <1s

---

### 2. Arbor Settings (System Settings)

**Current State:** LXQt Config (basic sliders and tabs)  
**Goal:** Unified, discoverable control center with permission dashboard

#### Design

**Two-Pane Layout:**
```
+──────────────────────────────────────────────────────┐
│ Settings      [ Search settings... Ctrl+K ]      [X] │
├────────────┬──────────────────────────────────────────┤
│ [System]   │ DISPLAY                                 │
│ Network    │ +───────────────────────────────────+   │
│ Bluetooth  │ | Primary: 3840×2160 @ 120Hz HDR   |   │
│ Display    │ +───────────────────────────────────+   │
│ Sound      │ Scaling: [100%] [125%] [150%] [175%]   │
│ Power      │ Night Light: [Toggle] Sunset-Sunrise   │
│            │ Refresh Rate: [120Hz ▼]                 │
│ [Security] │                                        │
│ Privacy    │ Color Profile:                         │
│ Firewall   │ [○] sRGB  [○] DCI-P3  [◉] Custom      │
│ Encryption │                                        │
│            │ [ Apply ]  [ Reset ]                    │
├────────────┼──────────────────────────────────────────┤
│ [Apps]     │ (dynamic content pane based on left     │
│ Defaults   │  selection — not shown in sketch)       │
│ Permissions│                                        │
│ Store      │                                        │
└────────────┴──────────────────────────────────────────┘
```

**Sections & Features:**

**[System]**
- **Network:** Wi-Fi list, connection status, DNS-over-HTTPS selector, WireGuard profiles
- **Bluetooth:** Device discovery, pairing, battery %
- **Display:** Resolution, scaling, refresh rate, night light, arrangement (for multi-monitor)
- **Sound:** Input/output device selector, per-app volume sliders, latency toggle (Pro Audio mode)
- **Power:** Power profile (Balanced/Performance/Power Saver), battery settings, auto-lock timeout

**[Security & Privacy]**
- **Privacy Dashboard:** Table showing every app, permissions granted (camera, mic, location, contacts, files), toggle to revoke
  - Audit log: "Spotify accessed Microphone — 3:41 PM" (timestamp + app + permission)
- **Firewall:** List of apps attempting outbound connections, per-app allow/block/ask toggles
  - Visual: showing which apps are "talking" helps users understand system behavior
- **Encryption:** TPM status indicator, LUKS key management, per-user `systemd-homed` status
- **Security Audit:** Secure Boot status, SELinux mode toggle (enforcing/permissive), periodic audit report link

**[Personalization]**
- **Appearance:** Theme selector (Dark/Light/Auto), accent color grid (Teal/Blue/Amber/Rose)
- **Fonts:** System font selector, text size slider
- **Wallpaper:** Built-in + custom upload, time-based rotation
- **Animations:** Toggle "Reduce Motion" (respects `prefers-reduced-motion` system setting)

**[Applications]**
- **Default Apps:** File manager, browser, mail, terminal, text editor — clickable to reassign
- **Permissions:** Grant/revoke sandbox permissions per Flatpak app (detailed Flatpak manifest viewer)
- **Store Settings:** Curated vs. unverified app sources toggle

**[System]**
- **About:** OS version, hardware info (CPU, RAM, GPU, drives), uptime
- **Snapshots & Rollback:** Timeline of deployments, one-click rollback button
- **Keyboard:** Layout selector, compose keys
- **Accessibility:** Screen reader enable, magnification, high contrast, font scaling

**Search Integration (Cmd+K):**
- Real-time fuzzy search across all settings panels
- Shows matching setting + breadcrumb ("Power → Sleep Timeout")
- Navigate with arrow keys, Enter to jump to panel

**Keyboard Shortcuts:**
- `Ctrl+K` — Open search
- `Escape` — Close search
- `Tab` — Cycle through UI sections (left pane → right pane content)

**Metrics:**
- Launch: <300ms
- Search 1000 settings: <100ms
- Apply changes: instant (write to config files in background)

---

### 3. Arbor Terminal

**Current State:** QTerminal from LXQt (basic, functional)  
**Goal:** Modern, feature-rich terminal with tabs, splits, and Arbor theming

#### Design

**Layout:**
```
┌─────────────────────────────────────────────────────┐
│ ~ — Terminal                                    [_][▢][X]
├─────────────────────────────────────────────────────┤
│ [Tab 1: ~]  [Tab 2: Projects] [+]                   │
├──────────────┬─────────────────────────────────────┤
│ $ ls -la     │ $ cargo build                       │
│ total 48     │ Compiling project v0.1.0 ...       │
│ drwxr-xr-x   │ Finished release [optimized]...    │
│ -rw-r--r--   │                                    │
│              │ $                                  │
│              │                                    │
├──────────────┴─────────────────────────────────────┤
│ [Search: (up)] Results: 3/10 [Close]               │
└─────────────────────────────────────────────────────┘
```

**Key Features:**

1. **Multi-Tab & Split Pane Support**
   - Tabs at top (create with `Ctrl+T` or + button)
   - Vertical splits with `Ctrl+|`, horizontal with `Ctrl+-`
   - Drag tabs to reorder or create new window
   - Visual indicators (favicon-style) for active/inactive panes

2. **Search in Terminal (Ctrl+F)**
   - Regex search highlighting in scrollback
   - Prev/Next navigation, case-sensitive toggle
   - Results counter ("3/10 matches")
   - Highlight color: `Accent Primary` (#2DD4BF)

3. **Copy/Paste Smart Detection**
   - `Ctrl+C` context-aware: if text selected, copy; if not, send signal
   - Mouse triple-click selects line (standard)
   - Auto-copy to clipboard on selection (Linux standard, not macOS)

4. **Theming**
   - Base theme: Arbor Dark (matches OS color palette)
   - Syntax highlighting: Based on JetBrains Mono + terminal color palette
   - Cursor styles: Block, underline, bar (configurable)
   - Bell notification: Visual flash + audio beep (optional)

5. **Font Management**
   - Font selector dropdown: JetBrains Mono (default), Monospace alternatives
   - Size slider: 10px–24px (default 13px)
   - Live preview while adjusting

6. **Configuration**
   - Settings panel: keybindings, scrollback lines, audible bell, cursor style
   - Right-click → Preferences (vs. separate app)

**Keyboard Shortcuts:**
- `Ctrl+T` — New tab
- `Ctrl+W` — Close tab
- `Ctrl+Tab` / `Ctrl+Shift+Tab` — Next/previous tab
- `Ctrl+|` — Vertical split
- `Ctrl+-` — Horizontal split
- `Ctrl+F` — Search
- `Ctrl+L` — Clear screen (sends `clear` command)
- `Ctrl+Shift+V` — Paste
- `Ctrl++` / `Ctrl+-` — Zoom font size

**Metrics:**
- Launch: <200ms
- Render 1000 lines: <50ms
- Search in 10k lines: <100ms

---

### 4. Arbor Software Center (App Store)

**Current State:** None (LXQt has no app store)  
**Goal:** Curated, privacy-focused app distribution with tiered verification

#### Design

**Main Layout:**
```
┌──────────────────────────────────────────────────────┐
│ Arbor Store      [ Search apps, packages... ]   [=]  │
├──────────────────────────────────────────────────────┤
│  FEATURED                                            │
│  ┌─────────────────────────────────────────────────┐ │
│  | [Hero Image] Obsidian                          | │
│  | Powerful knowledge base on top of markdown     | │
│  | [Install Flatpak] (Security: VERIFIED ✓)      | │
│  └─────────────────────────────────────────────────┘ │
├──────────────────────────────────────────────────────┤
│ CATEGORIES                                           │
│ [ Development ] [ Productivity ] [ Creative ]...     │
├──────────────────────────────────────────────────────┤
│ TOP APPS THIS WEEK                                   │
│ ┌──────────┐ ┌──────────┐ ┌──────────┐             │
│ │[Icon]    │ │[Icon]    │ │[Icon]    │ ...          │
│ │VS Code   │ │Blender   │ │Steam     │             │
│ │Editor    │ │3D Model  │ │Gaming    │             │
│ └──────────┘ └──────────┘ └──────────┘             │
└──────────────────────────────────────────────────────┘
```

**App Detail Page:**
```
┌──────────────────────────────────────────────────────┐
│ [◀] VS Code                                     [X]  │
├──────────────────────────────────────────────────────┤
│ [Icon] VS Code — Code Editor                        │
│ ⭐⭐⭐⭐⭐ (1,234 reviews)  Download: 250MB         │
│ [Install] [Already Installed] [Open]               │
│                                                    │
│ SECURITY BADGE                                     │
│ ✅ Arbor Verified — Reviewed, Reproducible         │
│ 🔐 Sandboxed (Flatpak)                             │
│ 🌐 Network Access: Enabled                         │
│ 📂 File Access: ~/Documents, ~/Downloads           │
│                                                    │
│ DESCRIPTION                                        │
│ VS Code is a lightweight but powerful source code  │
│ editor for Windows, Mac and Linux...               │
│                                                    │
│ DEVELOPER                                          │
│ Microsoft [https://github.com/microsoft/vscode]   │
│ Maintainer: Flathub                                │
│                                                    │
│ RELEASES                                           │
│ v1.92.0 — 2 weeks ago                              │
│ v1.91.1 — 3 weeks ago                              │
│                                                    │
│ USER REVIEWS (sorted by helpful)                  │
│ ⭐⭐⭐⭐⭐ "Perfect IDE" by user123                 │
│ "Works great on Arbor, no issues..."               │
│ [Helpful] [Report]                                 │
└──────────────────────────────────────────────────────┘
```

**Key Features:**

1. **Tiered Trust Badges**
   - 🟢 **Arbor Verified:** Reviewed by Arbor security, reproducible build confirmed
   - 🟡 **Flathub Verified:** Upstream Flathub's verification only
   - ⚪ **Community:** Unverified, sandboxed but not reviewed
   - Visual hierarchy clearly communicates trust level

2. **Privacy Label (Auto-Generated)**
   - Network access: Yes/No
   - File system access: Scope shown (~/Documents, ~/Downloads, or Full)
   - Camera: Yes/No
   - Microphone: Yes/No
   - Generated from Flatpak manifest analysis + developer self-report
   - Community flag button for inaccurate claims

3. **One-Click Install**
   - [Install] button changes to "Installing..." → Progress bar → "Open" or "Installed"
   - No separate download → installation steps
   - Error states handled gracefully (network failure, storage full, etc.)

4. **Search & Browse**
   - Search bar (real-time search across app names + descriptions)
   - Category browse grid
   - Featured collection carousels
   - "What's New" feed (new apps, major updates)

5. **Source Management**
   - Settings toggle: "Curated Apps Only" (default) vs. "Include Unverified (Flathub)"
   - Option to add custom Flatpak repositories (advanced)
   - All sources clearly labeled ("Arbor Store" vs. "Flathub")

6. **Update Management**
   - Auto-update toggle (default: ON, with daily schedule)
   - Manual "Update All" button in toolbar
   - Update history view (show what changed)

**Implementation:**
- **Backend:** Flatpak repo aggregation (Flathub API + Arbor's own repo)
- **Frontend:** Qt6 or WebView (Chromium Embedded Framework / QtWebEngine)
- **Sandboxing:** If Store runs as Flatpak, uses portal for file/network access

**Metrics:**
- Launch: <500ms
- Search 1000 apps: <200ms
- Install 100MB app: ~1-2 min (depends on network)

---

### 5. Arbor Update Manager

**Current State:** Integrated into Settings (basic rpm-ostree info)  
**Goal:** Standalone, transparent update experience with atomic deployment visualization

#### Design

**Main Window:**
```
┌──────────────────────────────────────────────────────┐
│ Arbor System Updates                            [X]  │
├──────────────────────────────────────────────────────┤
│ SYSTEM STATUS                                        │
│ ✅ System is Up to Date                              │
│ Running: Arbor OS 0.6.0 (Deployment 12)              │
│ Latest:  Arbor OS 0.6.1 (Available)                  │
│                                                    │
│ VERSION DETAILS                                     │
│ Current:  0.6.0                                      │
│ Build:    2026-09-01 14:23:44 UTC                    │
│ Kernel:   6.11.0 (arbor-optimized)                   │
│ Base:     Fedora 39 (Atomic)                         │
│                                                    │
│ [Check for Updates]  [Download & Install]           │
│ [ Auto-Update: ON ] (Check daily at 2 AM)           │
├──────────────────────────────────────────────────────┤
│ DEPLOYMENT TIMELINE                                 │
│ • [●] 0.6.0 (Current) — Jun 1, 2026 ← active       │
│ • [○] 0.5.9 (Previous)  — May 25, 2026              │
│ • [○] 0.5.8              — May 18, 2026              │
│                                                    │
│ [Rollback to 0.5.9]  [Delete Old Deployment]        │
└──────────────────────────────────────────────────────┘
```

**Update Flow (When New Version Available):**

1. **Check & Download**
   - "Check for Updates" → fetches metadata from Arbor servers
   - If new version available: "Download & Install" button appears
   - Click → downloads full image in background (shows progress %)
   - Can use system while downloading

2. **Install & Reboot Prompt**
   - Download complete → "Install and Reboot?" prompt
   - User chooses: "Reboot Now" / "Later" / "Never (remind next session)"
   - Countdown timer visible if "Reboot Now" selected (10s)

3. **Update At Boot**
   - Boot loader switches to new deployment (atomic operation)
   - Kernel + rootfs from new image load
   - systemd-boot's boot-counting: if boot fails N times, fallback to previous deployment automatically

4. **Post-Update Notification**
   - After successful boot into new version, notification: "Arbor OS updated to 0.6.1 — Release notes [link]"

**Deployment Timeline Visualization:**
- Vertical timeline showing deployments (newest on top)
- Current deployment marked with filled circle (●)
- Previous deployments with hollow circles (○)
- Hover shows: version, build date, kernel version, size
- Right-click: "Rollback to this version" (triggers reboot into that OSTree deployment)

**Configuration:**
- Auto-update toggle (daily schedule configurable: 2 AM default, other times supported)
- Beta channel toggle (if enabled, receives pre-release versions)
- Notification frequency: "Always notify", "Only for major versions", "Never"

**Implementation:**
- **Backend:** D-Bus service for update status, rpm-ostree interaction
- **Frontend:** Qt6 window with custom timeline rendering

**Metrics:**
- Launch: <300ms
- Check for updates: <2s (with network latency)
- Download 1.2GB image: ~5-15 min (depends on network speed, typically 1–5 Mbps residential speeds)

---

### 6. Arbor System Monitor

**Current State:** None (LXQt provides system tools separately)  
**Goal:** Unified system performance dashboard with real-time graphs

#### Design

**Main Window (Tabbed Interface):**
```
┌──────────────────────────────────────────────────────┐
│ System Monitor                                  [X]  │
├──────────┬────────────────────────────────────────────┤
│ [System] │ CPU: 2%   │ Memory: 1.2/8GB (15%)         │
│ [CPU]    │ Disk I/O: 0 MB/s (read), 0 MB/s (write)  │
│ [Memory] │ Network: ↓ 0.5 Mbps ↑ 0.1 Mbps           │
│ [Disk]   │                                          │
│ [Network]│ PROCESSES (sorted by CPU %)              │
│          │ App           CPU%  Memory   State       │
│          │ VS Code       8%    120 MB   Running     │
│          │ Firefox       4%    400 MB   Running     │
│          │ systemd       0.5%  10 MB    Running     │
└──────────┴────────────────────────────────────────────┘

   [CPU Tab]
   ┌──────────────────────────────────────────────────┐
   | CPU Usage — Last 60 min                          │
   |                                                  │
   | 100% ┤       ┌─┐                                 │
   |  80% ┤   ┌───┘ └───┐                            │
   |  60% ┤   │         │ ┌──                        │
   |  40% ┼───┘         └─┘  ├── Core 0 (50%)       │
   |  20% ┤                  ├── Core 1 (30%)       │
   |   0% ┤___________________├── Core 2 (10%)       │
   |      └──────────────────────── Core 3 (5%)      │
   | Cores (4): ████░░░░░░░░░░░░░░░░  Avg: 23%       │
   | Freq: 2.8 GHz (Max: 4.2 GHz)                    │
   | Temp: 52°C                                      │
   | Power: 15W                                      │
   └──────────────────────────────────────────────────┘

   [Memory Tab]
   ┌──────────────────────────────────────────────────┐
   | Memory Usage — Last 60 min                       │
   |                                                  │
   | 8 GB ┤                                           │
   | 6 GB ┤         ┌──────────┐                      │
   | 4 GB ┼─────────┘          └─                    │
   | 2 GB ┤──┐                                        │
   | 0 GB ┤──┴─────────────────────────────           │
   |      └──────────────────────────────── Used: 1.2 GB
   | Pressure: 0 (Good) [Memory Pressure Stall Info]│
   | Swap: 0 / 2 GB                                 │
   └──────────────────────────────────────────────────┘

   [Disk Tab]
   ┌──────────────────────────────────────────────────┐
   | Disk Devices                                    │
   |                                                  │
   | /dev/sda (NVME Samsung 970 EVO 500GB)            │
   | ████████████████░░░░░░░░░░░░░░░░░░ 350/500 GB  │
   | Read: 50 MB/s | Write: 30 MB/s                  │
   |                                                  │
   | /dev/sdb (WD Blue 2TB HDD)                       │
   | ████░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 500/2000 GB│
   | Read: 100 MB/s | Write: 80 MB/s                 │
   └──────────────────────────────────────────────────┘

   [Network Tab]
   ┌──────────────────────────────────────────────────┐
   | Network Usage — Last 60 min                      │
   |                                                  │
   |  10 Mbps ┤                                       │
   |   8 Mbps ┤   ┌─┐                                │
   |   6 Mbps ┤   │ │     ┌─┐                        │
   |   4 Mbps ┼───┘ └─────┘ └───── Download         │
   |   2 Mbps ┤                    Upload            │
   |   0 Mbps ┤__________________________________    │
   |          └──────────────────────────────────     │
   | Interfaces: wlan0 (connected), eth0 (off)       │
   | wlan0: ↓ 5.2 Mbps (rx), ↑ 0.8 Mbps (tx)        │
   | Signal: ▁▂▃▄ (-50 dBm), BSSID: AA:BB:CC:DD    │
   └──────────────────────────────────────────────────┘
```

**Key Features:**

1. **Real-Time Graphs**
   - CPU, Memory, Disk I/O, Network usage over 60 min (scrollable history)
   - Line graphs with grid, colored series per core/device
   - Smooth animations (no flickering)
   - Time axis: labeled with 5/10 min intervals

2. **CPU Tab Details**
   - Per-core usage bars + global average %
   - CPU frequency (current, max, base)
   - Temperature (°C) if sensor available
   - Power draw (W) if available
   - Load average (1min, 5min, 15min)

3. **Memory Tab Details**
   - Used / Total bar chart
   - Memory Pressure Stall Info (if kernel supports PSI) — early warning of swapping
   - Swap usage (if configured)
   - Page cache size (informational)

4. **Disk Tab Details**
   - Per-device list with capacity bar
   - I/O performance: read/write speeds (MB/s)
   - SMART health status (if available, shows warning)
   - Mount point + filesystem type

5. **Network Tab Details**
   - Download/upload speeds (Mbps)
   - Per-interface detail: signal strength, BSSID (Wi-Fi), ESSID
   - Interface list with connect/disconnect toggles
   - DNS info (if curious, expand option)

6. **Processes Tab (System Tab)**
   - Sortable table: App Name, CPU %, Memory, State
   - Right-click menu: "Kill Process", "Send Signal", "Show Details"
   - Sorting: default by CPU%, can sort by Memory, Name
   - Filter: "All", "My Apps", "System Apps"
   - Kill button with confirmation

**Configuration (Preferences):**
- Update interval: 1s (default) — 5s (high performance) — 10s (low power)
- History length: 30min / 60min / 120min / unlimited (limited by memory)
- Show/hide graph series (per-core CPU display toggle)
- Sidebar visibility toggle
- Alarm thresholds (highlight red if CPU>80%, Memory>90%, etc.)

**Keyboard Shortcuts:**
- `1/2/3/4/5` — Switch to System/CPU/Memory/Disk/Network tabs
- `Ctrl+Q` — Quit
- `Escape` — Clear selection

**Implementation:**
- **Data Source:** `/proc/stat`, `/proc/meminfo`, `/proc/diskstats`, `netdev` interface + optional systemd journal for power/thermal
- **Graph Rendering:** Qt Graphics Framework (QGraphicsView) for smooth animation
- **Refresh Loop:** 1 Hz (default, configurable)

**Metrics:**
- Launch: <300ms
- CPU overhead: <1% system resource usage
- Memory footprint: <50 MB resident

---

## Implementation Schedule (Phase 6)

### Phase 6.0 — Core Apps Foundation (Months 0–4)

**Deliverables:**
- Arbor Design System v1 (color palette, typography, component specs)
- Qt6 theme engine + Arbor style plugin
- Arbor Files v1.0 (file manager with tags + preview)
- Arbor Terminal v1.0 (tabs, splits, search)
- Arbor Settings v1.0 (full control center)

**Tasks:**
1. Design system implementation (theme colors, fonts, CSS/stylesheet)
2. File manager: core browsing + tag database
3. Terminal: multi-tab + split pane rendering
4. Settings: two-pane layout + all panels listed above
5. Integration testing: theme consistency, keyboard nav, accessibility

### Phase 6.1 — Extended Apps (Months 2–6)

**Deliverables:**
- Arbor Software Center v1.0 (app store with Flatpak integration)
- Arbor Update Manager v1.0 (deployment timeline, auto-update)
- Arbor System Monitor v1.0 (real-time graphs, process list)

**Tasks:**
1. App Store: Flatpak repo aggregation, app detail pages, review system
2. Update Manager: rpm-ostree D-Bus integration, timeline UI
3. System Monitor: procfs parsing, real-time graphing, process management

### Phase 6.2 — Polish & Release (Months 4–6)

**Deliverables:**
- All apps at v1.0 (release-ready)
- Flatpak manifests for each app (sandboxed deployment)
- Full accessibility audit (AT-SPI2 testing)
- Documentation + man pages
- Translation (en, de, fr, es, ja, zh, pt, ru at minimum)

**Tasks:**
1. Accessibility: screen reader testing, keyboard-only nav verification
2. Localization: string extraction, professional translation
3. Packaging: Flatpak metainfo, GNOME integration
4. Documentation: user guides, keyboard shortcut references
5. QA: comprehensive testing matrix (hardware + OS combinations)

---

## Design System Colors (Arbor Palette v1)

### Dark Theme (Default)

| Element | Color | Code | Usage |
|---------|-------|------|-------|
| Surface Base | Deep Slate | `#0F1215` | Window background, main surface |
| Surface Elev 1 | Panel Bg | `#161B22` | Panels, sidebars, elevated surfaces |
| Surface Elev 2 | Card Bg | `#21262D` | Card backgrounds, window backgrounds |
| Accent Primary | Arbor Teal | `#2DD4BF` | Links, highlights, primary actions |
| Accent Hover | Teal Light | `#5EEAD4` | Hover state for accent elements |
| Text Primary | Light Text | `#F0F6FC` | Main text, labels |
| Text Muted | Gray Text | `#8B949E` | Secondary text, placeholders |
| Border Neutral | Subtle Border | `rgba(255,255,255,0.08)` | Dividers, UI edges |
| Semantic Success | Green | `#3FB950` | Success states, valid input |
| Semantic Warning | Orange | `#F0883E` | Warnings, caution states |
| Semantic Danger | Red | `#F85149` | Errors, destructive actions |

### Light Theme (Optional)

| Element | Color | Code | Usage |
|---------|-------|------|-------|
| Surface Base | Off White | `#F6F8FA` | Main background |
| Surface Elev 1 | White | `#FFFFFF` | Elevated surfaces |
| Surface Elev 2 | Light Gray | `#EEF2F6` | Card backgrounds |
| Accent Primary | Teal Dark | `#0D9488` | Links, highlights |
| Text Primary | Dark Text | `#1F2328` | Main text |
| Text Muted | Medium Gray | `#67707E` | Secondary text |
| Border Neutral | Subtle Gray | `rgba(0,0,0,0.08)` | Dividers |

---

## Accessibility Baseline

### WCAG 2.1 AA Compliance

- **Color Contrast:** All text ≥4.5:1 ratio (normal text)
- **Focus Visible:** 2px outline ring (`#2DD4BF` on dark, `#0D9488` on light)
- **Keyboard Navigation:** Every UI element accessible via Tab/Arrow keys/Enter
- **Motion Reduced:** Respect `prefers-reduced-motion` system setting (disable spring animations)
- **Screen Reader:** Full AT-SPI2 semantic tree; all UI labeled with descriptive text

### AT-SPI2 Integration

- Window titles and hierarchies properly registered
- Button labels, link text, form field labels semantic
- Progress indicators announce updates ("Installing: 45%")
- Notifications read aloud with priority level

### Keyboard Shortcuts (Universal)

- `Tab` — Navigate forward, `Shift+Tab` — backward
- `Enter` — Activate button/link
- `Space` — Toggle checkbox/button
- `Arrow Keys` — Navigate within lists/tabs
- `Escape` — Close modal/dismiss menu
- `Ctrl+Q` — Quit app (all apps)
- `F1` — Open help

---

## Quality Metrics & Testing

### Performance Targets

| Application | Metric | Target |
|---|---|---|
| **Arbor Files** | Launch time | <500ms |
| **Arbor Files** | List 1000 files | <200ms |
| **Arbor Terminal** | Launch time | <200ms |
| **Arbor Terminal** | Render 1000 lines | <50ms |
| **Arbor Settings** | Launch time | <300ms |
| **Arbor Settings** | Search 1000 items | <100ms |
| **Arbor Software Center** | Launch time | <500ms |
| **Arbor Update Manager** | Launch time | <300ms |
| **Arbor System Monitor** | Launch time | <300ms |
| **Arbor System Monitor** | Memory footprint | <50MB |

### Test Coverage

- **Unit tests:** >80% code coverage (Qt model/logic layers)
- **Integration tests:** All keyboard shortcuts, D-Bus calls, file I/O
- **Hardware compatibility:** Test on reference hardware (Intel/AMD, NVIDIA/AMD/Intel GPU, 8GB–16GB RAM)
- **Accessibility:** Manual AT-SPI2 testing with Orca screen reader
- **Internationalization:** Translation completeness check for all UI strings

---

## Security & Sandboxing

### Flatpak Metadata

Each application ships with a Flatpak manifest declaring minimal permissions:

**Arbor Files (example):**
```yaml
[Context]
shared=network;ipc
sockets=fallback-x11;wayland
filesystems=home

[Context.Environment]
GTK_THEME=Arbor
```

**Arbor Software Center (example):**
```yaml
[Context]
shared=network;ipc
sockets=fallback-x11;wayland
system-bus-talk=org.freedesktop.PackageKit

[Context.Environment]
GTK_THEME=Arbor
```

### Privilege Escalation

- Settings changes requiring root: use Polkit dialogue (unified system experience)
- Update Manager: D-Bus service runs as system user, UI as regular user (privilege separation)
- Terminal: runs as user (no privilege escalation by default; `sudo` available for user-initiated commands)

---

## Dependencies & System Integration

### Core Dependencies

- **Qt6** (libraries: QtCore, QtGui, QtWidgets, QtDBus)
- **D-Bus** (system bus for systemd, NetworkManager, udisks2 communication)
- **freedesktop** standards (XDG Base Directory, icon themes, MIME types)
- **systemd** (user session integration, service activation)

### Optional Dependencies

- **Flatpak** (if packaged as sandboxed apps)
- **PipeWire** (sound device enumeration in Arbor Terminal/Settings)
- **NetworkManager** (network list in Settings)
- **udisks2** (disk info in System Monitor + File Manager)

---

## Documentation Deliverables

### User Documentation

1. **Arbor Files User Guide**
   - File navigation, tagging workflow, keyboard shortcuts
   - Screenshots + animated GIFs for complex features

2. **Arbor Terminal Cheat Sheet**
   - Common commands, split pane workflow, keyboard shortcuts

3. **Arbor Settings Reference**
   - Feature explanation for each panel (Network, Display, Privacy, etc.)
   - Troubleshooting guide

4. **App Store Guide**
   - How to search, install, rate apps
   - Understanding trust badges + permissions
   - Managing auto-updates

### Developer Documentation

1. **Arbor Design System**
   - Component library (Qt stylesheets + design tokens)
   - Usage guide for third-party developers

2. **Flatpak Integration**
   - How to package applications for Arbor Store
   - Permission guidelines + best practices

---

## Success Criteria (Phase 6 Complete)

- ✅ All 6 core applications launch and function correctly
- ✅ Design system applied consistently (typography, colors, spacing)
- ✅ All keyboard shortcuts implemented + working
- ✅ Accessibility audit passed (WCAG AA compliance)
- ✅ Performance metrics met (all apps launch <500ms)
- ✅ Flatpak sandboxing verified (each app deployable as Flatpak)
- ✅ Full user documentation available
- ✅ Translation into 7+ languages

---

## Conclusion

Phase 6 establishes ArborOS as a coherent, polished desktop environment. The core application suite is designed from first principles to embody "Quiet Precision" — powerful, consistent, and unobtrusive. Each application seamlessly integrates with the OS, sharing a design system and keyboard-first interaction model while maintaining lightweight performance.

**Status:** ✅ **SPECIFICATION COMPLETE**  
**Next Phase:** Phase 6 Implementation (4–6 months estimated)

