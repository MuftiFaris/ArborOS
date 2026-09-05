# ArborOS Privacy & User Control - Frequently Asked Questions (FAQ)

### Q1: How does ArborOS handle app permissions compared to Ubuntu or Windows?
**A:** Standard desktop OSes grant desktop applications broad user-level access by default. ArborOS uses a centralized `PrivacyManager` engine that intercepts hardware (mic/camera), file access, and network requests, enforcing default-deny or ask-on-first-use policies.

### Q2: What happens when I enable the Global Microphone Kill Switch?
**A:** All incoming microphone streams are instantly blocked, active recording buffers are cleared, and `requestPermission()` immediately returns `false` to all requesting applications regardless of their individual permission settings.

### Q3: How is the Privacy Score calculated?
**A:** The score starts at 100 base points. Points are deducted for unnecessary permissions granted to applications (-15 for dangerous sensors like camera/mic, -5 for unneeded file/network access), while bonus points (+5 to +10) are awarded for active global kill switches and VPN enforcement.

### Q4: Is the Audit Trail secure from application tampering?
**A:** Yes. The SQLite audit log (`audit.db`) is stored in protected system space with restricted file access permissions (`0600`) and managed exclusively by the `PrivacyManager` system process.

### Q5: Can I restrict an app from accessing specific folders like ~/Documents while allowing app data storage?
**A:** Yes! The `FileAccessControl` subsystem automatically sandboxes apps to their designated directory (`~/.local/share/<app-id>`) while blocking access to system locations (`/etc`, `/root`) and prompting you before allowing access to user folders (`~/Documents`, `~/Desktop`).

### Q6: Does DNS logging record my browsing content?
**A:** No. DNS logging only records domain resolution requests (e.g., `api.example.com`), query types (A/AAAA), and timestamped application identifiers to give you visibility into network connections. No payload or page content is inspected.

### Q7: Can I backup or restore my privacy policies?
**A:** Yes. Individual or global policies can be exported as JSON via `PrivacyManager::exportPolicy()` and re-imported on another ArborOS installation.
