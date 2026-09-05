# Phase 7 - Privacy & User Control Implementation

**Phase Goal:** Comprehensive privacy controls and user transparency framework for ArborOS  
**Status:** Planning  
**Target Output:** Privacy dashboard, permission management, user controls UI  
**Deliverables:** 8 tasks, ~3,500-4,000 lines of code

---

## Executive Overview

Phase 7 implements ArborOS core promise: user privacy and control. Not theoretical privacy, but practical, visible, controllable privacy in every system interaction.

**Core Principle:** Users own their data. Users control access. Users know what's happening.

---

## Tasks Breakdown

### Task #1: Privacy Architecture & Framework
**Objective:** Design permission system architecture  
**Deliverables:**
- `PrivacyManager` class (centralized privacy control)
- Permission categories (Microphone, Camera, Files, Network, Location, etc.)
- Permission states (Denied, Allowed, Ask-every-time, Allowed-once)
- D-Bus interface for system-wide permission queries
- Policy storage (QSettings + SQLite for audit trail)

**Files to Create:**
- `Code/privacy/include/privacy-manager.h` (header, ~300 lines)
- `Code/privacy/src/privacy-manager.cpp` (implementation, ~350 lines)
- `Code/privacy/include/permission-policy.h` (permission definitions)
- `Code/privacy/src/permission-policy.cpp` (policy enforcement)

**Key Classes:**
```cpp
class PrivacyManager {
    enum PermissionCategory { Microphone, Camera, Files, Network, Location, ... };
    enum PermissionState { Denied, AllowedAlways, AskEveryTime, AllowedOnce };
    
    bool requestPermission(const QString &appId, PermissionCategory);
    void setPermission(const QString &appId, PermissionCategory, PermissionState);
    QList<PermissionRecord> getAuditTrail(int days = 30);
};
```

---

### Task #2: Microphone Control System
**Objective:** Fine-grained microphone access control  
**Deliverables:**
- Microphone permission UI (Settings app integration)
- Per-application microphone access control
- Visual indicator when microphone is active
- Option to disable microphone globally
- Audio level monitoring (what's being recorded)

**Files to Create:**
- `Code/applications/arbor-settings/include/microphone-control.h`
- `Code/applications/arbor-settings/src/microphone-control.cpp`
- `Code/privacy/include/audio-interceptor.h` (capture audio state)
- Tray indicator component

**Features:**
- Real-time microphone state display
- Per-app permission grant/deny/ask
- Global microphone kill-switch
- Audit log of microphone access
- Alert notifications when microphone used

---

### Task #3: Camera Control System
**Objective:** Similar to microphone, but for camera/webcam  
**Deliverables:**
- Camera permission UI
- Per-app camera access control
- Visual indicator (LED simulation if no hardware LED)
- Disable camera globally option
- Last access timestamp per app

**Files to Create:**
- `Code/applications/arbor-settings/include/camera-control.h`
- `Code/applications/arbor-settings/src/camera-control.cpp`
- `Code/privacy/include/camera-interceptor.h`

**Features:**
- Camera enumeration (list available cameras)
- Per-device control (multiple cameras)
- Permission persistence
- Access alerts and logging

---

### Task #4: File Access Control
**Objective:** Granular file system permissions  
**Deliverables:**
- File access permission UI (which apps can read/write/delete which directories)
- Sandbox directory concept (apps only access designated folders)
- Home directory protection
- System file protection
- Override controls for power users

**Files to Create:**
- `Code/privacy/include/file-access-control.h`
- `Code/privacy/src/file-access-control.cpp`
- `Code/privacy/include/sandbox-manager.h`

**Architecture:**
```
App Sandbox Directories:
├── ~/.local/share/app-id/          (app data, always accessible)
├── ~/Desktop/                      (opt-in, ask user)
├── ~/Documents/                    (opt-in, ask user)
├── ~/Downloads/                    (opt-in, ask user)
└── Custom paths (user whitelists)

System Restricted:
├── /etc/                           (deny)
├── /root/                          (deny)
├── /boot/                          (deny)
└── /sys/, /proc/                   (deny)
```

---

### Task #5: Network Permission Control
**Objective:** Visibility and control over network access  
**Deliverables:**
- Network permission UI (which apps can connect to internet)
- DNS query logging (what domains accessed)
- IP address filtering (block certain IPs/ranges)
- VPN integration (force traffic through VPN)
- Network traffic monitoring dashboard

**Files to Create:**
- `Code/privacy/include/network-control.h`
- `Code/privacy/src/network-control.cpp`
- `Code/privacy/include/dns-logger.h`
- `Code/applications/arbor-settings/src/network-privacy-panel.cpp`

**Monitoring:**
- Per-app network usage (bytes sent/received)
- DNS queries per app
- Failed connection attempts
- Network activity timeline

---

### Task #6: Privacy Dashboard
**Objective:** Central hub showing all privacy activity  
**Deliverables:**
- Dedicated Privacy Dashboard application
- Real-time permission requests
- Access history (what accessed what, when)
- Granular permission management
- Privacy score (how exposed is your system?)
- Recommendations (unused apps with permissions, etc.)

**Files to Create:**
- `Code/applications/arbor-privacy-dashboard/CMakeLists.txt`
- `Code/applications/arbor-privacy-dashboard/include/privacy-dashboard.h`
- `Code/applications/arbor-privacy-dashboard/src/privacy-dashboard.cpp`
- `Code/applications/arbor-privacy-dashboard/src/main.cpp`
- `Code/applications/arbor-privacy-dashboard/src/access-timeline.cpp`
- `Code/applications/arbor-privacy-dashboard/src/privacy-scoring.cpp`

**Dashboard Sections:**
1. **Real-Time Activity** - Currently active permissions being used
2. **Access History** - Timeline of all permission requests/uses
3. **App Permissions** - Per-app permission overview
4. **Privacy Score** - 0-100 rating of privacy exposure
5. **Recommendations** - Alerts and suggestions
6. **Audit Trail** - Detailed log (searchable, filterable)

**Privacy Score Algorithm:**
```
100 points base
-10 points per unnecessary permission granted to app
-5 points per network access by app
-15 points if microphone/camera permissions granted to suspicious apps
+5 points if VPN enabled
+10 points if all unnecessary permissions denied
= Privacy Score
```

---

### Task #7: User Transparency UI
**Objective:** Make privacy visible everywhere  
**Deliverables:**
- Permission request dialogs (clear explanations)
- Status bar indicators (microphone/camera/network active)
- Notification system for permission usage
- System tray applet for quick privacy access
- Permission explanation labels (why does app need this?)

**Files to Create:**
- `Code/privacy/include/permission-dialog.h`
- `Code/privacy/src/permission-dialog.cpp`
- `Code/privacy/include/status-indicators.h`
- `Code/applications/arbor-settings/src/privacy-tray.cpp`

**Permission Dialog Example:**
```
[App Name] wants to access your MICROPHONE

Why? To record audio for video calls

You previously:
- Allowed this
- Allowed once (last used: 2 days ago)

[ Deny ] [ Allow Once ] [ Always Allow ] [ Settings ]

□ Remember this choice
```

---

### Task #8: Privacy Documentation & User Guide
**Objective:** Help users understand and use privacy features  
**Deliverables:**
- `PRIVACY_USER_GUIDE.md` - How to use privacy controls
- `PRIVACY_FAQ.md` - Common questions
- `PRIVACY_THREATS_MITIGATED.md` - What specific threats are prevented
- In-app help tooltips
- Privacy best practices guide

**Documentation Files:**
- `Documentation/07-Implementation/Phase Plan/Phase 7 - Privacy and User Control/PRIVACY_USER_GUIDE.md`
- `Documentation/07-Implementation/Phase Plan/Phase 7 - Privacy and User Control/PRIVACY_FAQ.md`
- `Documentation/07-Implementation/Phase Plan/Phase 7 - Privacy and User Control/PRIVACY_THREATS_MITIGATED.md`

---

## Implementation Details

### Permission Request Flow

```
1. App requests permission (e.g., microphone)
   ↓
2. PrivacyManager checks stored policy
   ├─ If Denied → Return false, log denied attempt
   ├─ If AllowedAlways → Return true, log access
   ├─ If AskEveryTime → Show dialog
   └─ If AllowedOnce → Return true, reset to Ask
   ↓
3. If dialog shown:
   ├─ User chooses action
   └─ PrivacyManager updates policy, logs decision
   ↓
4. Result returned to app
   ↓
5. Access logged to audit trail
   ↓
6. Status indicators updated in real-time
```

### D-Bus Integration

```cpp
// PrivacyManager exposes D-Bus interface:
// org.arboros.PrivacyManager

// Methods:
bool RequestPermission(app_id: string, category: string) -> bool
void SetPermission(app_id: string, category: string, state: uint32)
Array<PermissionRecord> GetAuditTrail(days: int32)
uint32 GetPrivacyScore() -> score
Array<string> GetBlockedApps() -> apps

// Signals:
PermissionRequested(app_id: string, category: string)
AccessLogged(app_id: string, category: string, timestamp: int64)
```

### SQLite Audit Schema

```sql
CREATE TABLE permission_audits (
    id INTEGER PRIMARY KEY,
    timestamp INTEGER,
    app_id TEXT,
    permission_category TEXT,
    action TEXT,           -- "requested", "granted", "denied", "used"
    user_decision TEXT,    -- "allow_always", "allow_once", "deny", "system_deny"
    details TEXT,
    FOREIGN KEY(app_id) REFERENCES apps(id)
);

CREATE INDEX idx_audit_timestamp ON permission_audits(timestamp);
CREATE INDEX idx_audit_app ON permission_audits(app_id);
```

---

## Architecture Diagram

```
User Layer
├─ Privacy Dashboard (Arbor Privacy app)
├─ Settings → Privacy Panel
├─ Status Bar Indicators
└─ Permission Dialogs

Control Layer
├─ PrivacyManager (centralized)
├─ Permission Policies
└─ Audit Logger

Enforcement Layer
├─ Audio Interceptor (microphone control)
├─ Camera Interceptor (camera control)
├─ File Access Control (sandbox)
└─ Network Control (DNS/IP filtering)

Storage Layer
├─ QSettings (active policies)
├─ SQLite (audit trail)
└─ D-Bus (system integration)
```

---

## Security Considerations

### Threat Model

**What we protect against:**
- Apps accessing microphone without permission
- Rogue apps exfiltrating data through network
- Apps reading files outside their sandbox
- Hidden camera activation
- Silent data collection

**What we don't claim to protect against:**
- Local kernel exploits (require system-level patching)
- Hardware-based threats (physical monitoring)
- Infected system binary (defense-in-depth requires verified boot)

### Permission Hardening

- Default-deny for all permissions
- Require explicit user approval for dangerous permissions
- Audit trail immutable (append-only log)
- Permission changes require password confirmation for system-critical permissions
- Sandboxing enforced at filesystem level (with D-Bus, seccomp possible in Phase 9)

---

## Integration Points

### With Existing Systems

1. **Arbor Settings** - Privacy panel integration
   - New tab: "Privacy & Permissions"
   - Per-app permission management
   - Global privacy settings

2. **Arbor Applications** - All 6 Phase 6 apps need privacy audit
   - Do they need microphone? (only Terminal if needed)
   - Do they need camera? (Software Center: no, Files: no, etc.)
   - Minimal necessary permissions

3. **D-Bus Services** - Integration with systemd, NetworkManager
   - PrivacyManager → systemd: enforce resource limits
   - PrivacyManager → NetworkManager: DNS logging

4. **Flatpak** - Phase 7 Flatpak manifests updated
   - Permission sandboxing via Flatpak permissions
   - Network access control
   - File access sandboxing

---

## User Workflows

### Workflow 1: App Requests Microphone

```
User opens Video Chat App
  ↓
App requests microphone permission
  ↓
PrivacyManager shows permission dialog
  ↓
Dialog: "Video Chat wants microphone. Last used 3 days ago. [Deny] [Allow Once] [Always]"
  ↓
User clicks "Allow Once"
  ↓
App gets microphone access
Audit logged: app_id=video-chat, action=used, decision=allow_once
  ↓
Status bar shows "Microphone in use" indicator
  ↓
User finishes video call
  ↓
Microphone access revoked
```

### Workflow 2: Check Privacy Dashboard

```
User opens Arbor Privacy Dashboard
  ↓
Dashboard loads:
- Privacy Score: 78/100
- Real-time activity: Spotify using network
- Recent access: Files app (read ~/.local), Terminal (full filesystem)
  ↓
User clicks "Spotify" row
  ↓
Detailed view shows:
- Permissions: Network (always), No microphone, No camera
- Last 7 days: 142 network accesses, 0 denied
- Total data: 125 MB sent, 45 MB received
  ↓
User clicks "Restrict Network"
  ↓
PrivacyManager limits Spotify to specific domains (music.spotify.com, etc.)
  ↓
Changes saved, audit logged
```

### Workflow 3: Review Audit Trail

```
User opens Privacy Dashboard → "Audit Trail" tab
  ↓
Filters available:
- By app (filter dropdown)
- By permission (microphone, camera, network, files)
- By date range
- By action (requested, granted, denied, used)
  ↓
User filters: Category=Network, Last 24 hours
  ↓
Results show:
- 10:15 AM: Chrome requested network → Allowed (persistent)
- 10:45 AM: Firefox requested network → Allowed (persistent)
- 2:30 PM: Spotify used network (48 MB)
- 3:15 PM: Software Center requested network → Denied (not installed)
- 4:00 PM: Terminal used network (SSH to server)
  ↓
User exports report for backup
```

---

## Code Statistics Estimate

| Component | Files | Lines | Status |
|-----------|-------|-------|--------|
| PrivacyManager core | 4 | 700 | Core |
| Microphone control | 3 | 400 | Feature |
| Camera control | 3 | 350 | Feature |
| File access control | 3 | 450 | Feature |
| Network control | 4 | 500 | Feature |
| Privacy Dashboard app | 6 | 800 | UI |
| Transparency UI | 4 | 300 | UI |
| Documentation | 3 docs | 1,200 | Docs |
| **TOTAL** | **30** | **4,700** | |

---

## Testing Strategy

### Unit Tests
- PrivacyManager permission logic
- Policy enforcement rules
- Audit trail recording
- Permission state transitions

### Integration Tests
- D-Bus communication
- Settings persistence
- Flatpak permission mapping
- Network/file access enforcement

### User Acceptance Tests
- Permission dialogs show correctly
- Status indicators update in real-time
- Dashboard displays accurate information
- Audit trail searchable and complete

### Security Tests
- Attempt unauthorized access (should fail)
- Verify sandboxed apps can't escape
- Check audit trail for tampering
- Validate permission state changes

---

## Deliverables Checklist

- [ ] PrivacyManager class (core framework)
- [ ] Permission policy system
- [ ] D-Bus interface for privacy control
- [ ] Microphone control UI & enforcement
- [ ] Camera control UI & enforcement
- [ ] File access control & sandboxing
- [ ] Network control & DNS logging
- [ ] Arbor Privacy Dashboard application
- [ ] Permission request dialogs
- [ ] Status bar indicators
- [ ] System tray applet
- [ ] Audit trail UI (search, filter, export)
- [ ] Privacy scoring algorithm
- [ ] Settings panel integration
- [ ] SQLite audit schema
- [ ] D-Bus service definitions
- [ ] Documentation (user guide, FAQ, threat model)
- [ ] Tests (unit, integration, security)

---

## Success Criteria

✅ Users can see what permissions apps have  
✅ Users can deny any permission (nothing required)  
✅ Users can see what their apps are doing (real-time)  
✅ Users can review history of app access  
✅ Users get alerts when apps access sensitive resources  
✅ Privacy is transparent and understandable (not technical jargon)  
✅ All Phase 6 apps audit-pass (no unnecessary permissions)  
✅ Audit trail is immutable and searchable  
✅ Privacy score gives users actionable feedback  
✅ No performance regression (privacy controls < 5% overhead)

---

## Next Steps (Phase 8+)

- **Phase 8 (Update & Recovery):** Backup encryption with privacy controls
- **Phase 9 (Performance):** Optimize privacy enforcement (seccomp, AppArmor)
- **Phase 10 (QA):** Full security audit by external firm

---

## References

- OWASP Permission Model: https://owasp.org/
- Android Privacy Model (reference architecture)
- macOS Privacy Controls (Transparency, Consent, Control)
- Linux AppArmor/SELinux policies
- GDPR requirements (user data transparency)

---

**Phase 7 Status:** Ready for implementation  
**Estimated Duration:** 6-8 weeks (full-time, one developer)  
**Complexity:** High (security-critical code)  
**Review Required:** Security audit before Phase 8

---

End of PRIVACY_IMPLEMENTATION.md
