# Arbor Privacy Framework - Frequently Asked Questions

## General Questions

### Q: What is the Arbor Privacy Framework?

**A:** The Arbor Privacy Framework is a comprehensive system for managing and monitoring application access to sensitive system resources (microphone, camera, files, network). It provides fine-grained permission control, real-time activity monitoring, and transparency through the Privacy Dashboard.

### Q: Does the Privacy Framework slow down my system?

**A:** No. The framework is highly optimized with:
- Minimal CPU overhead (~<1% when idle)
- Efficient database queries using SQLite with proper indexing
- Background permission checking runs every 500ms
- Non-blocking audit logging

### Q: Can I disable the Privacy Framework?

**A:** We recommend keeping it enabled for security. However, you can:
1. Disable global controls (though per-app permissions still enforced)
2. Turn off activity logging (through Settings)
3. Disable notifications (through system notification settings)

System protection remains active regardless.

### Q: Does Privacy Framework send data to external servers?

**A:** No. All privacy data remains on your device:
- Activity logs stored locally in SQLite database
- No cloud sync or external reporting
- No data collection or telemetry
- Complete privacy of your privacy data

---

## Permissions

### Q: What happens if I deny all permissions to an app?

**A:** The app can still run but with restricted capabilities:
- Cannot access microphone, camera, or protected files
- Cannot connect to network (if network permission denied)
- May display warnings about missing permissions
- Some features may not work

### Q: Can I set different permissions for the same app on different devices?

**A:** Yes. Permissions are device-specific and stored locally. Each device has independent permission configurations.

### Q: What's the difference between "Allow" and "Allow Once"?

**A:**
- **Allow** - Permission granted permanently (until you revoke)
- **Allow Once** - Permission granted for this single use only, next request requires re-approval
- **Deny** - Permission rejected, app cannot access resource

### Q: How do I know if an app is using permissions?

**A:** Multiple indicators:
- Status bar shows colored indicators (red=mic, orange=camera, blue=network)
- Activity tab shows real-time access attempts
- Tray icon shows active resources
- App-specific privacy score shows access patterns

### Q: Can I set permissions for multiple apps at once?

**A:** Currently per-app only. For bulk operations:
1. Use the Dashboard "Recommendations" to apply suggested policies
2. Configure global toggles (affects all apps equally)
3. Export and import permission policies (future feature)

---

## File Access

### Q: What files can applications access?

**A:** By default, apps can access:
- Their own sandbox directory (`~/.local/share/arbor-sandbox/{app-id}/`)
- User directories: `~/Desktop`, `~/Documents`, `~/Downloads`

Blocked paths (protected):
- `/etc/`, `/root/`, `/boot/`, `/sys/`, `/proc/`, `/dev/`, `/lib/`, `/bin/`, `/usr/bin/`, etc.

### Q: How do I give an app access to a specific file?

**A:** When app requests file access:
1. Permission dialog appears with file path
2. Review the reason
3. Click "Allow" or "Deny"
4. Check "Remember this choice" to avoid repeated requests

### Q: Can an app access my personal documents?

**A:** Yes, but with permission:
- First request triggers permission dialog
- You approve or deny
- If approved, app can access `~/Documents` directory
- You can revoke access anytime from Permissions tab

### Q: What is a sandbox directory?

**A:** A sandbox is an isolated directory for each app:
- Located at `~/.local/share/arbor-sandbox/{app-id}/`
- Only that app can access it (plus system admin)
- Perfect for app-specific data, cache, settings
- Automatically created when app first requests

### Q: Can I move files to/from sandbox?

**A:** Yes, through file manager:
1. Open file manager
2. Navigate to `~/.local/share/arbor-sandbox/{app-id}/`
3. Copy/move files as needed
4. Or use the app's built-in import/export

---

## Network & DNS

### Q: What information is logged about network access?

**A:** Privacy Framework logs:
- Timestamp of DNS query
- Application making the request
- Domain name requested
- Query type (A record, AAAA, MX, etc.)
- Resolution result (success/failure/blocked)
- Total bytes sent/received
- Network interface used

**Not logged:**
- Actual network traffic content
- Passwords or authentication tokens
- Private user data

### Q: Why is my app accessing unexpected domains?

**A:** Common reasons:
- **Tracking services** - Analytics, ads, telemetry
- **Content delivery** - Images, videos, media
- **Updates** - Automatic software updates
- **APIs** - Third-party service integrations
- **Redirects** - Following URL redirects

You can:
1. Review DNS queries in Activity tab
2. Block domains via IP filtering
3. Require VPN for the app
4. Revoke network permission

### Q: Can I block specific websites/domains?

**A:** Partially, through IP filtering:
1. Identify IP addresses for domain (use DNS lookup)
2. Dashboard > Permissions > Add IP Filter
3. Set as "Blacklist"
4. Connections to that IP are blocked

For better domain blocking, use system-wide DNS filtering or custom firewall rules.

### Q: What is VPN requirement?

**A:** Apps can be configured to require VPN:
- If VPN not connected, app cannot access network
- Forces use of secure VPN tunnel
- Useful for banking, email, messaging apps
- Can be set per-application

**To enable:**
1. Dashboard > Recommendations
2. Find app in list
3. Set "Require VPN: Yes"
4. App will now require active VPN

### Q: How much data does Privacy Framework log?

**A:** Configured retention limits:
- Access history: Last 5000 records
- DNS queries: Last 5000 records
- Network usage: Last 10000 records
- Audit trails: Configurable retention (default 30 days)

You can manually clear logs through Settings > Clear History.

---

## Privacy Dashboard

### Q: What does the privacy score mean?

**A:** Privacy score (0-100) indicates your system's overall privacy protection:

- **0-25:** Critical concerns, immediate action needed
  - Too many permissions granted
  - Unusual access patterns
  - Suspicious network activity

- **26-50:** Significant issues, should improve
  - Several risky apps with many permissions
  - Moderate network activity
  - Some unreviewed access requests

- **51-75:** Acceptable with caution
  - Reasonable permission grants
  - Normal access patterns
  - Regular privacy reviews recommended

- **76-100:** Strong privacy protection
  - Minimal permissions granted
  - Clean access patterns
  - Excellent privacy practices

### Q: How is the privacy score calculated?

**A:** Score formula:
- 20% - Microphone/camera access patterns
- 20% - File access security (sandbox compliance)
- 30% - Network activity and DNS queries
- 15% - Total permission count
- 15% - Access denial rate (security)

### Q: Why did my privacy score drop?

**A:** Possible reasons:
- Installed new app with many permissions
- Approved request for new resource access
- Detected unusual access pattern
- Network activity increased
- Recommendation not implemented

**To improve:**
1. Review "Recommended Actions"
2. Check Applications tab for suspicious behavior
3. Revoke unnecessary permissions
4. Update problematic apps
5. Configure app-specific policies

### Q: Can I export my privacy report?

**A:** Yes, from Permissions tab:
1. Click "Export Report"
2. Choose format: PDF or CSV
3. Select save location
4. Report includes all permissions and activity

### Q: How often should I review my privacy?

**A:** Recommended schedule:
- **Daily:** Quick glance at status indicators
- **Weekly:** Review Activity tab for anomalies
- **Monthly:** Full Privacy Dashboard review
- **Quarterly:** Deep audit and policy adjustment
- **After installing apps:** Immediate permission review

---

## Security & Safety

### Q: What threats does Privacy Framework protect against?

**A:** See "PRIVACY_THREATS_MITIGATED.md" for complete list. Common threats:

**Prevented:**
- Unauthorized microphone recording
- Hidden camera access
- Sensitive file theft
- DNS hijacking
- IP-based tracking
- Unauthorized network access

**Mitigated:**
- Malware spreading via file access
- Data exfiltration attempts
- Spyware activity
- Privacy-invading analytics
- Unauthorized system access

### Q: Is this a replacement for antivirus?

**A:** No. Privacy Framework complements antivirus:
- **Antivirus:** Detects and removes malware
- **Privacy Framework:** Controls what malware can access even if it runs

Use both for complete protection.

### Q: What if an app I trust suddenly requests suspicious permissions?

**A:** This is suspicious and warrants investigation:
1. Check app changelog - did latest version add features?
2. Search online - is this legitimate request?
3. Contact app developer
4. Consider alternative apps
5. Use "Allow Once" instead of "Allow" until verified

### Q: Can the Privacy Framework be bypassed?

**A:** Technically difficult without system access:
- Admin account can modify permissions
- Kernel drivers could potentially bypass (not practical)
- Most bypasses require device physical access

**To protect:**
- Use strong admin password
- Keep system updated with security patches
- Monitor for unauthorized access
- Regularly review permission logs

### Q: What happens if system is compromised?

**A:** Privacy Framework will show evidence:
- Unusual app permission grants
- Unexpected network connections
- File access to protected paths
- Permission dialog bypasses

**If suspected compromise:**
1. Review Privacy Dashboard Activity tab
2. Export audit log for evidence
3. Look for unauthorized SSH/network access
4. Consider system recovery/reinstall
5. Contact system administrator

---

## Technical Questions

### Q: Where are permission settings stored?

**A:** Multiple locations:
- `~/.local/share/arbor-privacy/` - Per-app permissions
- SQLite database - Audit logs and history
- QSettings/config files - Dashboard preferences
- System keyring - Sensitive configurations

### Q: Can I back up and restore privacy settings?

**A:** Yes:
1. Privacy Dashboard > Settings > Export Settings
2. Saves configuration to encrypted file
3. Restore via > Settings > Import Settings
4. Useful when reinstalling OS

### Q: How do I reset all privacy settings?

**A:** 
1. Privacy Dashboard > Settings > Advanced
2. Click "Reset All Privacy Settings"
3. Confirm warning (deletes all history)
4. Services restart with factory defaults

### Q: What's the database file format?

**A:** SQLite3 database:
- Location: `~/.local/share/arbor-privacy/audit.db`
- Append-only design (immutable audit trail)
- Auto-vacuum after 30 days
- Encrypted with system keyring

### Q: Can I query the database directly?

**A:** Not recommended, but possible with:
```bash
sqlite3 ~/.local/share/arbor-privacy/audit.db
```

Database schema documented in privacy system code.

---

## Performance & Troubleshooting

### Q: Privacy Dashboard uses too much memory

**A:** Try:
1. Clear old audit logs (Settings > Clear History)
2. Reduce retention period (default 30 days)
3. Restart Privacy Dashboard
4. Disable real-time refresh (use manual)

### Q: Permission dialogs appear too frequently

**A:** Solutions:
1. Click "Remember this choice" when approving
2. Check for app bugs (report to developer)
3. Verify permission is legitimate
4. Consider "Allow Once" policy

### Q: Activity logging stops working

**A:** Troubleshooting:
1. Check disk space (database needs space)
2. Verify file permissions on `~/.local/share/arbor-privacy/`
3. Restart privacy services: `systemctl restart arbor-privacy`
4. Check for database corruption: Run system repair tool

### Q: Tray icon not showing

**A:** Try:
1. Verify tray is enabled (system settings)
2. Restart Privacy Manager service
3. Log out and back in
4. Check if privacy service is running

---

## Updates & Compatibility

### Q: Will Privacy Framework updates affect my settings?

**A:** Generally no:
- Settings preserved during updates
- Backward compatible with old configs
- Auto-migration if database schema changes
- You're notified of any breaking changes

### Q: Is Privacy Framework compatible with third-party apps?

**A:** Yes, if apps respect standard Linux permissions:
- Permission requests standard (via D-Bus)
- File permissions standard (Linux ACLs)
- Network standard (iptables/firewall)

Non-standard apps may not fully cooperate.

### Q: Can I use Privacy Framework with VPN software?

**A:** Yes, fully compatible:
- VPN detection automatic
- DNS logs capture VPN traffic
- Network monitoring works with VPN
- Can require VPN per-app

---

## Getting Help

### Q: Where do I report privacy bugs?

**A:** 
1. Privacy Dashboard > Help > Report Issue
2. Or visit: https://arbor-os.org/privacy/report
3. Include: System version, app name, detailed steps

### Q: How do I request privacy features?

**A:** 
1. GitHub Issues: https://github.com/arbor-os/privacy
2. Community forums: https://forums.arbor-os.org
3. Feature request form: https://arbor-os.org/privacy/features

### Q: Where is the privacy source code?

**A:**
- GitHub: https://github.com/arbor-os/privacy
- License: GPL v3
- Community contributions welcome

---

**Last Updated:** September 2026
**FAQ Version:** 1.0
