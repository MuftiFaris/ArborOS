# ArborOS Phase 7 - Privacy & User Control User Guide

## Overview

ArborOS puts you in absolute control of your digital privacy. Unlike conventional desktop operating systems that grant broad permissions silently, ArborOS operates under a **Strict Zero-Trust Privacy Model**. 

Every application—from web browsers to media players—must request explicit permission to access sensitive hardware, system files, or network resources.

---

## 1. Core Privacy Architecture

ArborOS features a centralized **PrivacyManager** subsystem that manages:
- **15 Granular Permission Categories:** Microphone, Camera, Files, Network, Location, Clipboard, Contacts, Calendar, Photos, Bluetooth, USB, Printing, Screenshots, System Monitor, Audio Output.
- **Permission States:**
  - `Denied` - Access permanently blocked.
  - `Allowed Always` - Access granted continuously for the application.
  - `Ask Every Time` - Prompts you with an interactive permission dialog upon each access request.
  - `Allowed Once` - Access granted for the current session, automatically resetting to "Ask Every Time".
  - `System Denied` - Overridden by global system safety policy.
- **Immutable SQLite Audit Trail:** Complete timestamped ledger of every permission request, grant, denial, and resource usage.

---

## 2. Using the Privacy Dashboard (`arbor-privacy-dashboard`)

Launch the **Arbor Privacy Dashboard** from the Application Menu or System Tray icon.

### Key Sections:
1. **Overview Tab:**
   - **System Privacy Score (0–100):** Real-time rating calculated based on current permission exposure, global kill-switches, and application risk factors.
   - **Privacy Exposure Gauge & Summary Card:** Displays active applications, 24-hour denial stats, and real-time status.
2. **Applications Tab:**
   - Comprehensive app list showing privacy rating, granted permissions, access counts, and denial records per app.
3. **Activity & Audit Trail Tab:**
   - Filterable timeline of all privacy events. Search by application, date range, or resource type. Export audit logs to CSV/PDF.
4. **Recommendations Tab:**
   - Smart privacy suggestions (e.g., revoking unused microphone access or enabling global restrictions).
5. **Permissions Tab:**
   - Centralized grid where permissions can be inspected or revoked with a single click.

---

## 3. Sensor & Network Controls

### Microphone Control & Mute Kill-Switch
- Real-time visual tray indicator (`🎤`) when microphone is active.
- Integrated audio monitoring level indicator.
- **Global Kill Switch:** Toggle off "Global Microphone Access" in Settings to hardware-silence all recording streams system-wide.

### Camera Control & Virtual LED Indicator
- Enumerates all attached V4L2 video devices (`/dev/video*`).
- Active camera indicator (`📷`) illuminates immediately upon capture initialization.
- **Global Camera Kill Switch:** Disables all video input streams system-wide.

### File Access Control & Sandboxing
- Enforces application data isolation (`~/.local/share/<app-id>`).
- Protects critical system directories (`/etc`, `/root`, `/sys`, `/proc`, `/boot`) from unauthorized read/write attempts.
- Customizable path whitelisting per application.

### Network Permission & DNS Inspector
- Per-application internet connectivity control.
- **DNS Query Inspector:** View every domain queried by each application.
- **IP Range Filtering:** Blacklist or whitelist custom IP ranges.
- **VPN Requirement Enforcement:** Force specific applications to route traffic exclusively through active VPN tunnels.

---

## 4. Quick Settings & System Tray Integration

The System Tray Privacy Applet provides one-click access:
- Instant status indicators for Microphone, Camera, and Network activity.
- Quick toggles for Global Mute and Camera Disable.
- Quick link to launch full Privacy Dashboard.
