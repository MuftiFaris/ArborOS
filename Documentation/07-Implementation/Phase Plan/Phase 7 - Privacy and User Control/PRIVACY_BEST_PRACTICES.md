# ArborOS Developer & Administrator Privacy Best Practices

## Guidelines for Developers

1. **Principle of Least Privilege:** Request only the minimum required permission categories for your application.
2. **Provide Clear Justification:** Always pass a human-readable `reason` string when invoking `PrivacyManager::requestPermission()`.
3. **Handle Permission Denials Gracefully:** Applications must handle `false` returns without crashing or blocking the UI thread.
4. **Use App Sandboxing:** Store all application configuration and persistent data inside `~/.local/share/<app-id>`.

## Guidelines for System Administrators

1. **Enforce Global Mute on Sensitive Workstations:** Use `PrivacyManager::setGlobalMicrophoneEnabled(false)` and `setGlobalCameraEnabled(false)` for secure environments.
2. **Review Audit Logs Periodically:** Inspect `getAuditTrail()` records for denied or unexpected access attempts.
3. **Mandate VPN for Network Apps:** Use `setAppsRequireVPN()` to restrict high-risk applications to encrypted tunnels.
