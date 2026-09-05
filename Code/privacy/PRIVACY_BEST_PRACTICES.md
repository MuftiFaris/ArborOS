# Arbor Privacy Framework - Best Practices Guide

Practical recommendations for maximizing privacy and security using the Privacy Framework.

---

## Table of Contents

1. [Immediate Actions](#immediate-actions)
2. [Permission Strategy](#permission-strategy)
3. [Application Management](#application-management)
4. [Network Security](#network-security)
5. [Data Protection](#data-protection)
6. [Monitoring Habits](#monitoring-habits)
7. [Advanced Configuration](#advanced-configuration)
8. [Incident Response](#incident-response)

---

## Immediate Actions

### Initial Setup (First Time)

**Within First Hour:**

1. **Launch Privacy Dashboard**
   ```
   Applications > Privacy Dashboard
   ```

2. **Review Current Privacy Score**
   - Overview tab shows system baseline
   - Note current score for future comparison

3. **Check Active Applications**
   - Applications tab
   - Identify apps with high permission count
   - Look for unexpected permissions

4. **Configure Global Controls**
   - Quick Settings panel
   - Enable VPN requirement
   - Set camera/microphone defaults

5. **Review Recommendations**
   - Recommendations tab
   - Prioritize "Critical" recommendations
   - Apply immediate fixes

### First Week

**Day 1-2: Permission Audit**
- Review each installed application
- Revoke unnecessary permissions
- Check privacy scores

**Day 3-4: Activity Review**
- Watch Activity tab for patterns
- Identify suspicious applications
- Check DNS queries for anomalies

**Day 5-7: Policy Implementation**
- Whitelist trusted applications
- Blacklist suspicious paths
- Configure VPN requirements
- Export baseline privacy report

---

## Permission Strategy

### Permission Grant Philosophy

**Principle: Principle of Least Privilege**

Grant only the minimum permissions an app needs to function.

### Decision Framework

When app requests permission, ask:

1. **Is permission legitimate?**
   - Does app need this for core functionality?
   - Is this feature documented?
   - Expected for this category of app?

2. **Is granularity available?**
   - Can permission be limited to specific resource?
   - Can it be time-limited with "Allow Once"?
   - Can it be revoked after initial use?

3. **What are alternatives?**
   - Does app have optional feature?
   - Can user work around missing permission?
   - Are there safer alternative apps?

4. **What's the risk?**
   - How sensitive is the resource?
   - How trustworthy is the app?
   - What could happen if permission abused?

### Permission Categories

#### Always Deny
- **Microphone** - To unknown recording apps
- **Camera** - To apps without video features
- **System files** - To any app requesting `/etc`, `/root`, `/boot`
- **SSH keys** - To any app except SSH managers
- **Database files** - To apps without explicit database features

#### Cautiously Grant
- **Network** - To privacy-sensitive apps, require VPN
- **File access** - Start with "Allow Once"
- **Location** - Only when feature is actively used
- **Camera/Microphone** - Only for conferencing during call

#### Safe to Grant
- **App-specific directories** - Sandbox auto-created
- **Downloads** - For download managers, apps
- **Documents** - For productivity apps
- **Desktop** - For file managers

### Time-Based Permission Grants

**Best Practice: Use "Allow Once"** when:
- First time encountering app
- Accessing unusual resources
- During trial/evaluation period
- For security-sensitive operations

After observing safe behavior:
- Switch to permanent "Allow"
- Or continue using "Allow Once" for extra caution

### Permission Revocation

**Review quarterly:**
1. Privacy Dashboard > Permissions tab
2. Sort by "Last Used"
3. Revoke old permissions not used in 90 days
4. Revoke all permissions before uninstalling app

---

## Application Management

### Installation Best Practices

**Before Installing:**

1. **Research App**
   - Read reviews for privacy concerns
   - Check permission requests in app store
   - Search for "[app name] privacy"
   - Check developer reputation

2. **Verify Source**
   - Official app store or website only
   - Never side-load from unknown sources
   - Check digital signatures
   - Verify checksum if provided

3. **Check Privacy Policy**
   - Read privacy policy (yes, really)
   - Look for data collection statements
   - Check third-party sharing
   - Look for opt-out options

### Initial Permission Review

**When First Running App:**

1. **Note All Permissions Requested**
2. **Deny Unnecessary Ones Immediately**
3. **Use "Allow Once" for Questionable**
4. **Monitor Activity for First Week**

### Risk Classification

**Tier 1: High Risk (Deny Extra Permissions)**
- Unknown or new apps
- Apps from unknown developers
- Apps with many negative reviews
- Apps requesting unusual permissions

**Tier 2: Medium Risk (Grant Minimum)**
- Established apps with known privacy issues
- Apps that collect analytics
- Free apps with ads
- Cloud-based services

**Tier 3: Low Risk (Grant Normally)**
- Well-known privacy-focused apps
- Open source apps with code review
- Apps with published transparency reports
- Apps by privacy-conscious companies

### Application Monitoring

**Daily:**
- Check status indicators (mic/camera/network)
- Glance at Activity tab for anomalies
- Note any unexpected access patterns

**Weekly:**
- Full review of permissions granted
- Check privacy score trend
- Review new recommendations
- Check DNS queries for known trackers

**Monthly:**
- Full application audit
- Review file access patterns
- Check network statistics
- Verify VPN requirements

### Application Removal

**When Uninstalling App:**

1. Revoke all permissions first
2. Clear app data and cache
3. Remove sandbox directory (optional)
4. Reboot to ensure complete removal
5. Monitor for any residual access attempts

---

## Network Security

### DNS Privacy

**Best Practice: Minimize DNS Exposure**

1. **Enable VPN Before Connecting**
   - VPN before opening browser
   - Prevents ISP DNS query tracking

2. **Monitor DNS Queries**
   - Activity > Network filter
   - Look for known tracker domains
   - Check for unusual domains

3. **Block Known Trackers**
   - Identify tracking domains
   - Add to IP blacklist via firewall
   - Document for future reference

**Common Tracker Domains:**
- `analytics.google.com`
- `facebook.com`, `facebook.net`
- `doubleclick.net`
- `scorecardresearch.com`
- `mixpanel.com`

### VPN Configuration

**Recommended VPN Setup:**

1. **Per-App VPN Requirement**
   ```
   Dashboard > Recommendations > "Enable VPN for sensitive apps"
   ```
   - Banking apps: Always require VPN
   - Email/messaging: Always require VPN
   - Chat apps: Always require VPN
   - General browsing: Optional

2. **VPN Protocol Selection**
   - **WireGuard** - Fastest, most secure (recommended)
   - **OpenVPN** - Widely compatible, mature
   - **IKEv2** - Mobile-friendly, stateful
   - Avoid: PPTP (deprecated), L2TP (weak)

3. **VPN Provider Selection**
   ```
   Criteria:
   - No-logs policy (verified independently)
   - Kill switch enabled by default
   - Fast, stable connection
   - Good server coverage
   - Port forwarding support (optional)
   - DNS leak protection
   - Reputable security audits
   ```

### Network Access Control

**Whitelist Approach (Recommended):**

1. Open Activity tab
2. Identify unnecessary network access
3. Create IP whitelist for each app
4. Revoke network access to others

**Example for Firefox:**
```
Allow:
- 142.251.32.0/19 (Google)
- 104.16.0.0/13 (Cloudflare)
- 151.101.1.0/24 (Fastly)
- Your ISP DNS servers
```

### DNS-over-HTTPS (DoH) Considerations

**Risk:** Apps using DoH bypass system DNS filtering

**Mitigation:**
1. Monitor for DoH queries (unusual port 443 DNS)
2. Consider network-level DNS enforcing
3. Use firewall rules to block alternative DNS
4. Require VPN for apps using DoH

---

## Data Protection

### Sensitive Data Directories

**Home Directory Sensitive Paths:**

```
~/.ssh/                 - SSH private keys (PROTECT)
~/.aws/                 - AWS credentials (PROTECT)
~/.gnupg/               - GPG keys (PROTECT)
~/.kube/                - Kubernetes configs (PROTECT)
~/.local/share/pass/    - Password manager (PROTECT)
~/Documents/            - Personal documents (LIMIT)
~/Pictures/             - Personal photos (LIMIT)
~/Downloads/            - Downloaded files (MONITOR)
~/.config/              - App configuration (MONITOR)
~/.cache/               - App cache (LOW RISK)
```

**Protection Strategy:**

1. **PROTECT (Sensitive Credentials)**
   - Deny all app access
   - Only manual access via file manager
   - Monitor any access attempts
   - Use dedicated password manager

2. **LIMIT (Personal Data)**
   - Grant only to apps with legitimate need
   - Use "Allow Once" when possible
   - Monitor access patterns
   - Regular audit of access

3. **MONITOR (Application Data)**
   - Grant to related apps
   - Review access patterns
   - Audit quarterly
   - Alert on unusual access

### File Access Policy Templates

**Productivity Apps (LibreOffice, VS Code):**
```
Allow: ~/Documents, ~/Downloads, ~/Desktop, ~/Projects
Deny: ~/.ssh, ~/.aws, /etc, /root
```

**Media Apps (VLC, Krita, Audacity):**
```
Allow: ~/Media, ~/Pictures, ~/Downloads, ~/Videos
Deny: ~/.ssh, ~/.aws, /etc, /root
```

**Utilities (File Manager, Archive Manager):**
```
Allow: Full home directory read, selective write
Deny: /etc, /root, /boot, /sys, /proc
```

### Credential Management

**Best Practice for Credentials:**

1. **Use Dedicated Password Manager**
   - KeePass, Bitwarden, 1Password
   - Centralized credential storage
   - Grant minimal access to other apps
   - Regular backups

2. **SSH Key Protection**
   - Store in `~/.ssh/` with restricted permissions
   - Use ssh-agent for key management
   - Deny app access to `~/.ssh/`
   - Use ED25519 keys (modern, secure)

3. **API Keys/Tokens**
   - Store in password manager, not files
   - Use environment variables for runtime
   - Rotate regularly
   - Monitor access in Activity log

---

## Monitoring Habits

### Daily Routine (5 minutes)

```
1. Glance at status bar indicators
2. Check if unexpected processes active
3. Note any permission dialogs
4. Review Privacy Dashboard summary
```

### Weekly Deep Dive (30 minutes)

```
1. Open Privacy Dashboard
2. Review Applications tab
   - Check new apps' privacy scores
   - Revoke unexpected permissions
3. Check Activity tab
   - Filter by "Denied" to see blocked attempts
   - Look for unusual patterns
4. Review Recommendations
   - Implement critical items
5. Export activity summary for archive
```

### Monthly Security Audit (1 hour)

```
1. Privacy Dashboard > Overview
   - Note privacy score change
   - Review top violations

2. Applications tab
   - Full permissions audit
   - Privacy score per app
   - Identify outliers

3. Activity tab
   - Time period: Last 30 days
   - Filter by type for each category
   - Export for analysis

4. Network analysis
   - Review DNS queries
   - Check for known trackers
   - Identify unusual domains

5. File access review
   - Check for access to sensitive paths
   - Monitor for unusual patterns
   - Review /etc, /root access attempts

6. Generate privacy report
   - Export PDF for documentation
   - Archive for future reference
   - Compare against previous month
```

### Quarterly Comprehensive Review (2 hours)

```
1. Full system privacy audit
2. Permission policy update
3. Unused app removal
4. Credential rotation if suspected issue
5. Update firewall/VPN rules
6. Review and update threat models
7. Incident response plan review
8. Generate quarterly privacy report
```

### Anomaly Detection

**Red Flags to Watch:**

- App accessing unusual resources
- Network traffic spike from unexpected app
- DNS queries to known malware domains
- Repeated permission denials (possible brute force)
- File access to system directories
- Network access from background app

**Response:**

```
Immediate:
1. Revoke suspicious permissions
2. Block network access
3. Note timestamp for investigation
4. Export activity logs

Short-term:
1. Research app and permission
2. Update app to latest version
3. Monitor for repeated incidents
4. Review app privacy policy

Long-term:
1. Uninstall if legitimate need unclear
2. Update threat model
3. Document in incident log
4. Share findings with community
```

---

## Advanced Configuration

### Custom Permission Policies

**Banking/Finance App Policy:**
```json
{
  "permissions": {
    "microphone": "DENY",
    "camera": "DENY",
    "file_access": "DENY",
    "network": "ALLOW_VPN_REQUIRED",
    "location": "DENY"
  },
  "file_paths": {
    "whitelist": [],
    "blacklist": ["~/.ssh", "~/.aws", "/etc", "/root"]
  },
  "network": {
    "vpn_required": true,
    "blocked_domains": ["ads.google.com", "analytics.google.com"],
    "allowed_ips": ["banking_provider_ips"]
  }
}
```

**Development Tool Policy:**
```json
{
  "permissions": {
    "microphone": "ALLOW_ONCE",
    "camera": "ALLOW_ONCE",
    "file_access": "ALLOW",
    "network": "ALLOW",
    "location": "DENY"
  },
  "file_paths": {
    "whitelist": ["~/Projects", "~/Code", "~/.local/share/code"],
    "blacklist": ["~/.ssh", "~/.aws"]
  },
  "network": {
    "vpn_required": false,
    "allowed_domains": ["github.com", "npmjs.org"]
  }
}
```

### Firewall Rules Integration

**For Advanced Users:**

1. Export Privacy Framework logs
2. Generate firewall rules from DNS/network data
3. Implement in iptables/UFW
4. Monitor enforcement

**Example UFW Rules:**
```bash
# Block tracking domains
ufw deny out to 142.251.32.0/19 port 443  # Google
ufw deny out to 1.1.1.1 port 53           # Cloudflare DNS (unless allowed)
ufw allow out from app_uid to trusted_ip  # App-specific rules
```

### SELinux/AppArmor Policies

**Integration with Mandatory Access Control:**

Privacy Framework can generate AppArmor profiles:
```
/usr/bin/firefox {
  /etc/passwd r,           # Read only
  /home/@{HOME}/Documents rw,  # Read-write
  /home/@{HOME}/.ssh r,    # Read restricted credentials
  deny /root rwk,          # Deny root access
  deny /etc/shadow rwk,    # Deny system files
}
```

---

## Incident Response

### Privacy Incident Detection

**Possible Indicators:**

- Privacy score drops significantly
- Unknown app requests many permissions
- File access to sensitive directories
- Network traffic spike
- DNS queries to malware domains
- Multiple permission denials

### Investigation Process

**Step 1: Document & Preserve**
```
1. Take screenshots of Activity log
2. Export privacy data to external drive
3. Note exact timestamp of incident
4. Preserve log files
5. Do NOT clear history yet
```

**Step 2: Isolate**
```
1. Disconnect from network (if serious)
2. Revoke suspicious app permissions
3. Block app network access
4. Disable app from running
5. Consider reinstalling OS if severe
```

**Step 3: Analyze**
```
1. Review Activity tab for suspicious app
2. Check file access to sensitive paths
3. Analyze DNS queries for malware domains
4. Check network connections
5. Export activity log for forensics
```

**Step 4: Remediate**
```
1. Uninstall suspicious app
2. Rotate any potentially compromised credentials
3. Change passwords for accessed accounts
4. Review other system access logs
5. Update firewall rules
```

**Step 5: Recovery**
```
1. Monitor for recurrence
2. Update threat model
3. Document incident
4. Review and improve monitoring
5. Share anonymized findings if appropriate
```

### When to Reinstall OS

**Reinstall if:**
- Root compromise suspected
- Kernel module exploitation suspected
- Multiple unrelated malware detected
- UEFI/firmware compromise suspected
- Cannot determine extent of compromise

**Backup Safely:**
```
1. Only backup user documents, not executables
2. Verify files are clean
3. Use external drive
4. Scan all backups with antivirus
5. Consider not restoring if unsure
```

---

## Security Checklist

### Monthly Privacy Checklist

- [ ] Privacy Dashboard reviewed
- [ ] Recommendations implemented
- [ ] Unused permissions revoked
- [ ] New app permissions audited
- [ ] DNS queries reviewed for tracking
- [ ] VPN still functioning
- [ ] Sensitive files checked for unauthorized access
- [ ] Malware scan completed
- [ ] System updates installed
- [ ] Privacy report exported

### Quarterly Security Checklist

- [ ] All of monthly checklist
- [ ] Credentials rotated
- [ ] Firewall rules reviewed
- [ ] Application inventory audited
- [ ] Backup tested for integrity
- [ ] SSH keys rotated
- [ ] API tokens refreshed
- [ ] Privacy policy review completed
- [ ] Threat model updated
- [ ] Incident response plan reviewed

---

## Integration with Other Security Tools

### Antivirus/Malware Detection
- Privacy Framework + Antivirus = Defense in depth
- Antivirus detects malware presence
- Privacy Framework prevents what it accesses
- Use both, not either/or

### Firewall
- Privacy Framework monitors application level
- Firewall monitors network level
- Complement each other
- Rules can be derived from Privacy logs

### VPN
- Privacy Framework knows what to block
- VPN provides anonymity
- Combine for maximum security
- VPN should be required by sensitive apps

### Password Manager
- Privacy Framework protects password manager
- Password manager stores credentials securely
- Grant minimal permissions to password manager
- Use password manager to avoid credential files

---

## Tips & Tricks

### Reduce Notification Fatigue

**Problem:** Too many permission dialogs

**Solutions:**
1. Grant permissions more freely after initial audit
2. Use "Allow Once" strategically
3. Adjust notification frequency
4. Whitelist frequently-used apps

### Improving Privacy Score

**Quick Wins:**
1. Revoke camera/microphone from unnecessary apps
2. Enable VPN requirement for sensitive apps
3. Block known tracker domains
4. Disable network for offline apps
5. Implement file access restrictions

### Performance Optimization

**If Privacy Framework slows system:**
1. Disable real-time logging (use manual check)
2. Increase audit log auto-cleanup frequency
3. Reduce activity history retention
4. Disable unnecessary monitors
5. Restart privacy services

---

**Last Updated:** September 2026
**Version:** 1.0
**Maintained by:** Arbor OS Privacy Team
