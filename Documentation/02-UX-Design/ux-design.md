Here is the complete UX/UI Design Specification and Design System for **Arbor OS**.

---

# ARBOR OS — UX/UI DESIGN SYSTEM & SPECIFICATION

---

## 1. BRAND IDENTITY

```
          /\
         /  \          ARBOR OS
        / /\ \         "Change anything. Break nothing."
       / /  \ \
      /_/    \_\

```

### OS Name Ideas

* **Arbor OS** *(Selected)* — Symbolizes a strong, immutable trunk with flexible, customizable branches.
* **Aether Linux** — Clean, weightless, focused on speed and invisibility of UI.
* **Verve Desktop** — Energetic, modern, high-precision.

### Design Philosophy

1. **Quiet Precision:** The interface stays out of your way until called. Unnecessary chrome, loud badges, and unnecessary lines are eliminated.
2. **Delightful Predictability:** Every interaction behaves consistently across system tools and third-party apps. Motion is functional, never gratuitous.
3. **Confident Ownership:** Give users total control without overwhelming them with raw configuration files unless they ask for them.

### Logo Direction

* **Visual Mark:** A minimalist, continuous-line geometric tree leaf that doubles as an abstract delta symbol ($\Delta$), representing evolution and branching snapshots.
* **Logotype:** Clean, tracking-expanded sans-serif typeface (*Inter Display*, semi-bold).

### Icon Style

* **System Icons:** Line-based (2px stroke), rounded caps, 24x24px grid system with strict optical alignment.
* **App Icons:** Squircle silhouette ($r = 22.5\%$ radius formula matching continuous curvature). Soft 3D lighting, subtle drop shadows (0px 8px 16px, 12% opacity), with rich, tactility-focused materials (frosted glass, brushed metal, soft-touch matte).

### Typography

* **Primary System Font:** `Inter` (UI elements, body text, subheads).
* **Display Font:** `Inter Display` (Headings, hero text, onboarding).
* **Monospace Font:** `JetBrains Mono` (Terminal, code editors, file details).

| Level | Size | Weight | Line Height | Letter Spacing |
| --- | --- | --- | --- | --- |
| **Display L** | 36px | Semi-Bold (600) | 44px | -0.02em |
| **Heading 1** | 24px | Semi-Bold (600) | 32px | -0.01em |
| **Heading 2** | 18px | Medium (500) | 24px | 0em |
| **Body / UI** | 14px | Regular (400) / Medium (500) | 20px | 0em |
| **Caption** | 12px | Regular (400) | 16px | +0.01em |
| **Code** | 13px | Regular (400) | 18px | 0em |

### Colors (Arbor Color Palette)

#### Dark Theme (Default)

* `Surface Base`: `#0F1215` (Deep Slate)
* `Surface Elev 1`: `#161B22` (Panel Background)
* `Surface Elev 2`: `#21262D` (Card / Window Background)
* `Accent Primary`: `#2DD4BF` (Arbor Teal)
* `Accent Hover`: `#5EEAD4`
* `Text Primary`: `#F0F6FC`
* `Text Muted`: `#8B949E`
* `Border Neutral`: `rgba(255, 255, 255, 0.08)`
* `Semantic Success`: `#3FB950`
* `Semantic Warning`: `#F0883E`
* `Semantic Danger`: `#F85149`

#### Light Theme

* `Surface Base`: `#F6F8FA`
* `Surface Elev 1`: `#FFFFFF`
* `Surface Elev 2`: `#EEF2F6`
* `Accent Primary`: `#0D9488`
* `Text Primary`: `#1F2328`
* `Text Muted`: `#67707E`
* `Border Neutral`: `rgba(0, 0, 0, 0.08)`

### Wallpapers

* **Default ("Conifer Dawn"):** Generative, high-resolution organic gradient vector art featuring abstract fog over pine ridges. Shifts dynamically based on the system time (Dawn, Noon, Sunset, Midnight).
* **Minimal Series:** Solid tactile textures (Milled Aluminum, Deep Charcoal, Soft Obsidian).

### Sound Identity

* **Boot Sound:** A low 3-note ambient acoustic swell (E-Major chord with high spatial resolution).
* **System Alerts:** Gentle, soft wood-tap tones (non-intrusive frequency bands, ~400Hz–800Hz).
* **Snapshot Sound:** Subtle mechanical camera shutter click (soft-pitched).

---

## 2. DESKTOP EXPERIENCE

### Desktop Layout

A clean canvas with zero desktop clutter by default (icons disabled by default, optional via toggle).

* **Top Bar:** Floating, translucent panel (8px floating margin).
* **Bottom Center:** Dynamic Smart Dock.

```
+-------------------------------------------------------------------------+
| [Leaf] Applications   09:41 AM Mon    [Wifi] [Vol] [Power] [Privacy: Green] |
|-------------------------------------------------------------------------|
|                                                                         |
|                                                                         |
|                                WORKSPACE                                |
|                                                                         |
|                                                                         |
|                +---------------------------------------+                |
|                |  [Files] [Terminal] [Browser] [Store] |  <-- Dock      |
|                +---------------------------------------+                |
+-------------------------------------------------------------------------+

```

### Top Panel

* **Height:** 36px.
* **Material:** Frosted Glass Backdrop Blur (`backdrop-filter: blur(20px)`), `Surface Elev 1` at 70% opacity.
* **Left Section:**
* **Arbor Logo / Launcher Icon:** Click opens **Overview / App Launcher**.
* **Active App Title:** Bolded app name + active document/page context.


* **Center Section:**
* **Clock & Calendar Widget:** Clicking toggles Notification Center + Agenda.


* **Right Section (Status Pill):**
* Integrated pill container holding Network status, Audio volume, Battery level, Privacy indicator dot (turns Green when Camera/Microphone active).
* Clicking opens **Quick Control Center**.



### Dock

* **Position:** Bottom floating (12px offset from edge).
* **Behavior:** Auto-hide with smart edge collision, fluid magnification on hover (smooth sine scale curve).
* **Content:** Pinned apps, active unpinned apps, running app indicators (glowing LED bar under icon), and a dedicated **Trash / Snapshots** stack at the far right separated by a subtle vertical rule.

### Window System

* **Window Chrome:** Headerbar architecture (unified titlebar + toolbar).
* **Window Controls (Top Right or Top Left via toggle):** Close (`✕`), Minimize (`—`), Expand (`⤢`).
* **Borders & Shadows:** 1px subtle glowing border (`Border Neutral`), 24px drop shadow with ambient occlusion for window hierarchy separation.
* **Snapping (Arbor Snap):** Dragging window to edges triggers dynamic preview tiles:
* Left/Right halves (50/50)
* Quarter corners (25/25/25/25)
* Center focus mode (60% width centered)
* *Keyboard Shortcut:* `Super + Arrow Keys`



### Workspace System

* Horizontal workspace strip activated via 3-finger swipe up or `Super + Tab`.
* Dynamic creation: Dragging a window past the last workspace creates a new one automatically.
* Multi-monitor handling: Independent workspace switching per display or synchronized workspaces (selectable in display settings).

### Animations & Gestures

* **Spring Physics:** All animations run on a damped spring physics curve (`stiffness: 300, damping: 30`).
* **Transitions:**
* App Launch: Soft scale-up from dock icon (200ms).
* Window Switch: Fluid 3D depth glide.


* **Touchpad Gestures (1:1 Tracking):**
* `3-Finger Swipe Up`: Open Overview / Workspaces.
* `3-Finger Swipe Left/Right`: Switch between active workspaces.
* `4-Finger Pinch`: Show All Applications Grid.



---

## 3. CONTROL CENTER

### Layout & Navigation

Two-pane layout with a persistent search bar at the top left.

* **Left Sidebar (240px):** Categorized list with icons, search filter at the top.
* **Right Content Area:** Responsive multi-card layout displaying context settings.

```
+-------------------------------------------------------------------+
| Settings            [ Search Settings... (Ctrl+K) ]            [X]|
+-----------------------+-------------------------------------------+
| [System]              |  DISPLAY                                  |
|   Network             |  +-------------------------------------+  |
|   Bluetooth           |  | Primary Display                     |  |
|   Display             |  | 3840 x 2160 @ 120Hz (HDR Enabled)   |  |
|   Sound               |  +-------------------------------------+  |
|                       |                                           |
| [Personalization]     |  Scaling                                  |
|   Appearance          |  ( ) 100%   (*) 125%   ( ) 150%          |
|   Applications        |                                           |
|                       |  Night Light                              |
| [Security & Privacy]  |  [ Toggle: ON ] Sunset to Sunrise         |
|   Privacy Dashboard   |                                           |
|   Security & Lock     |                                           |
|   Updates & Rollback  |                                           |
+-----------------------+-------------------------------------------+

```

### Core Panel Specifications

#### Network

* Wi-Fi network list with signal strength, security type (WPA3), instant QR code share option.
* Integrated WireGuard / OpenVPN profile manager with 1-click connect toggle.
* Built-in **DNS-over-HTTPS** selector (Default: Arbor Shield Privacy DNS, Cloudflare, Quad9, Custom).

#### Bluetooth

* Instant scan grid with device category icons (Headphones, Keyboards, Phones).
* Quick battery percentage indicators for connected peripherals.

#### Display

* Visual arrangement canvas for multi-monitor setups (drag to arrange orientation).
* Fractional scaling slider (100%, 125%, 150%, 175%, 200%).
* Refresh rate selector with VRR (Variable Refresh Rate / FreeSync / G-Sync) toggles.
* Color profile management (ICC profiles, sRGB / DCI-P3 toggle).

#### Sound

* Per-app volume controls with live stream meters.
* Output/Input device selector with high-latency/low-latency (PipeWire Pro Audio) switcher.

#### Privacy

* **Global Permission Matrix:** Camera, Microphone, Location, Screen Recording, Filesystem Access.
* Audit log timeline showing exact time apps accessed sensitive peripherals.

#### Security

* TPM status, Secure Boot status, LUKS Key management.
* **Arbor Shield Firewall:** Interactive app network access log with one-click toggles to block outbound traffic for specific Flatpaks.

#### Applications

* Default application associations.
* Flatpak sandbox permission inspector and override controls.

#### Updates & Snapshots

* Atomic system deployment status ("System is on Arbor OS 1.0.4 — Healthy").
* One-click system rollback button to prior OSTree deployments.
* Snapshot timeline viewer.

#### Accounts

* Local accounts, `systemd-homed` portable identity exports, SSH key manager, Fingerprint enrollment.

---

## 4. FIRST BOOT EXPERIENCE (ONBOARDING)

The onboarding flow runs in a frameless, centered modal window with a dark, blurred backdrop.

```
+-------------------------------------------------------------------+
|                                                                   |
|                         Welcome to Arbor                          |
|                 Let's setup your secure workspace.                |
|                                                                   |
|   Step 3 of 6: Disk Encryption                                    |
|   +-----------------------------------------------------------+   |
|   | [X] Encrypt System Drive (LUKS2)                          |   |
|   |     Uses TPM 2.0 for seamless unlock at boot.             |   |
|   |                                                           |   |
|   | Passphrase: [ ************************ ]                  |   |
|   | Confirm:    [ ************************ ]                  |   |
|   +-----------------------------------------------------------+   |
|                                                                   |
|   [ Back ]                                           [ Continue ] |
|                                                                   |
+-------------------------------------------------------------------+

```

### Step-by-Step Flow

1. **Language & Region:** Grid of flags and language titles; auto-detects based on geo-IP if connected.
2. **Accessibility First:** Large toggle bar to enable Screen Reader, High Contrast, or Text Magnification *before* proceeding.
3. **Account Creation:** Full Name, Username, and Password. Includes option to enroll Fingerprint/Hardware Security Key (YubiKey).
4. **Security & Encryption:**
* LUKS2 Encryption enabled by default.
* Option to link unlock key to system TPM 2.0 chip for passwordless secure boot.


5. **Privacy Configuration:**
* Toggle for automatic encrypted crash reporting (Default: Off).
* Toggle for Network MAC Address Randomization (Default: On).


6. **Theme Selection:** Side-by-side live preview cards for **Dark Slate**, **Light Pearl**, and **Dynamic Auto-Switch**. Accent color picker (Teal, Blue, Amber, Rose, Emerald).

---

## 5. FILE MANAGER ("ARBOR FILES")

### Navigation & Layout

Clean 3-column architecture: Sidebar (locations/tags), Main View (Grid/List), Detail Inspector Panel (toggleable).

```
+-----------------------------------------------------------------------+
| [<] [>]  /home/user/Documents/Projects/              [ Search... ] [=]|
+---------------+---------------------------------------+---------------+
| LOCATIONS     | Name             Size     Modified    | PROJECT SPEC  |
| [x] Home      | [F] Client_Brief 12 KB    10 min ago  | PDF Document  |
| [x] Documents | [F] Blueprint    2.4 MB   Yesterday   | 2.4 MB        |
| [x] Downloads | [D] Assets       --       3 days ago  | Tag: #Active  |
|               |                                       |               |
| TAGS          |                                       | [ Preview ]   |
| (*) Red       |                                       |               |
| (*) Teal (#)  |                                       | [ Snapshot ]  |
+---------------+---------------------------------------+---------------+

```

### Key Features

* **Instant Preview (Quick Look):** Pressing `Spacebar` opens a zero-latency floating preview overlay for images, PDFs, videos, markdown, code files, and audio.
* **Tagging System:** Drag files onto colored or custom-named tags in the sidebar for non-destructive cross-directory organization.
* **Cloud Integration:** Native Nextcloud, Proton Drive, Google Drive, and S3/Restic mounts directly in the sidebar with offline sync status overlays.
* **Security Actions:**
* Right-click context menu item: **"Encrypt with Vault"** (creates a sandboxed password-protected folder).
* Right-click context menu item: **"Shred File"** (secure overwriting deletion).



---

## 6. APP STORE ("ARBOR STORE")

### Store Interface Layout

Hero banner carousel showcasing curated open-source apps, followed by editorial collections ("Essential Dev Tools", "Privacy Shield Apps", "Creative Studio").

```
+-------------------------------------------------------------------+
| Arbor Store          [ Search apps, flatpaks, packages... ]   [=] |
+-------------------------------------------------------------------+
| HERO: Obsidian — Complete Markdown Knowledge Base                 |
| [ Install Flatpak ]  (Verified Security Check: PASSED)            |
+-------------------------------------------------------------------+
| TOP CATEGORIES                                                    |
| [ Development ]  [ Productivity ]  [ Creative ]  [ Gaming ]       |
+-------------------------------------------------------------------+
| FEATURED APPLICATIONS                                             |
| +-------------------+  +-------------------+  +-----------------+ |
| | [Icon] VS Code    |  | [Icon] Blender    |  | [Icon] Steam    | |
| | Dev Environment   |  | 3D Modeling       |  | Gaming Platform | |
| +-------------------+  +-------------------+  +-----------------+ |
+-------------------------------------------------------------------+

```

### App Detail Page Features

* **Security & Privacy Badges:**
* `Verified`: Signed build, tested sandbox profile.
* `Network Isolation`: Indicator showing if the app has network access permissions.
* `Sandboxed`: Confined via Bubblewrap/Flatpak portals.


* **Community Reviews:** 5-star ratings, user reviews filtered by version number, flagged review detection.
* **One-Click Installation:** Progress bar directly integrated into the install button.
* **Package Details:** Build commit hash, maintainer identity, source repository link, system permission checklist.

---

## 7. NOTIFICATION & QUICK SETTINGS CONTROL CENTER

Activated by clicking the Top Right Status Pill or `Super + N`. Drops down a combined, glassmorphism overlay.

```
+----------------------------------------------------+
|  QUICK SETTINGS                                    |
|  [ (Wifi) On ]    [ (Bluetooth) On ]  [ (Do Not Disturb) ]|
|  [ (VPN) Connected ] [ (Dark Mode) ]  [ (Performance) ]|
|                                                    |
|  Volume  : [========|--------] 60%                 |
|  Brightness: [=============|----] 80%              |
|----------------------------------------------------|
|  NOTIFICATIONS                                     |
|  [Icon] Arbor Backup · 2m ago                     |
|  System snapshot created successfully.             |
|  [ View Snapshot ] [ Dismiss ]                     |
|----------------------------------------------------|
|  [Icon] Slack · 15m ago                            |
|  Sarah: Hey, check out the updated wireframes!     |
|  [ Reply Box: Type reply...                  ]     |
+----------------------------------------------------+

```

* **Interactive Action Buttons:** Inline reply fields for messages, direct action buttons for background tasks (e.g., "View Snapshot", "Eject Drive").
* **Grouped Notifications:** Notifications clustered by application with collapsible stacks.

---

## 8. ACCESSIBILITY

Arbor OS makes accessibility a core structural pillar rather than an afterthought toggle:

* **Screen Reader Integration:** Native Orca screen reader bindings mapped to system UI trees via AT-SPI2, with clear semantic labels for all screen elements.
* **System Magnifier:** Smooth vector-based zoom engine bound to `Super + Alt + Scroll Wheel` with full-screen or lens zoom options.
* **Voice Control:** Offline, local speech-to-command engine powered by Whisper.cpp for basic voice-navigated actions ("Open Terminal", "Close Window").
* **Keyboard-Only Navigation:**
* Full UI operable without a mouse.
* Pressing `Alt` highlights explicit key accelerators across all controls.
* Visible, high-contrast focused state rings (`#2DD4BF` 2px outline) around active UI inputs.



---

## 9. USER JOURNEYS (EXEMPLARS)

### Journeys

#### 1. Beginner User (Switching from Windows)

* *Goal:* Install a web browser and stream music.
* *Journey:* Opens Arbor Store → Clicks "Brave Browser" → Clicks "Install" → App launches in 2 seconds. Opens Quick Settings to connect Bluetooth headphones with 1 click. Zero terminal usage required.

#### 2. Developer User

* *Goal:* Setup a Python/Rust development environment without polluting system dependencies.
* *Journey:* Opens Terminal → Executes `distrobox create -n dev-env --image fedora:latest` → Integrates container seamlessly with Arbor Files and VS Code. System host remains completely clean and immutable.

#### 3. Gamer User

* *Goal:* Play a modern AAA Windows game via Steam/Proton with optimal GPU performance.
* *Journey:* Launches Steam (pre-installed via Flatpak) → Enables Proton → Game launches → Arbor OS automatically switches power profile to **Performance Mode** and disables desktop compositing latency for maximum FPS.

#### 4. Professional User (Creative/Privacy Focused)

* *Goal:* Perform system updates safely before an important client presentation.
* *Journey:* Initiates OS update in Settings → System updates in background via OCI container deployment → Prompted to restart → Boot fails due to a custom driver regression → System detects failure and automatically rolls back to the previous deployment in 8 seconds. Presentation goes off without a hitch.