# Arbor Privacy User Guide

Complete guide to understanding and managing privacy settings in ArborOS.

## Table of Contents

1. [Overview](#overview)
2. [Getting Started](#getting-started)
3. [Privacy Dashboard](#privacy-dashboard)
4. [Permission Management](#permission-management)
5. [Access Controls](#access-controls)
6. [Network Privacy](#network-privacy)
7. [Best Practices](#best-practices)
8. [Troubleshooting](#troubleshooting)

---

## Overview

ArborOS provides comprehensive privacy controls to protect your data and device. The Privacy Framework includes:

- **Microphone Control** - Monitor and control microphone access per application
- **Camera Control** - Manage camera/webcam access with visual indicators
- **File Access Control** - Sandbox directories with system path protection
- **Network Control** - DNS logging, IP filtering, VPN integration
- **Privacy Dashboard** - Real-time activity monitoring and privacy scoring
- **Transparency UI** - Clear notifications and permission dialogs

### Privacy Score

Your system privacy score (0-100) is calculated based on:
- **20%** - Microphone/Camera access patterns
- **20%** - File access security
- **30%** - Network activity and DNS queries
- **15%** - Permission count
- **15%** - Access denial rate

**Score Ranges:**
- 0-25: Critical privacy concerns
- 26-50: Significant privacy issues
- 51-75: Acceptable with caution
- 76-100: Strong privacy protection

---

## Getting Started

### Launching Privacy Dashboard

1. Open the Applications menu
2. Search for "Privacy Dashboard"
3. Click to launch
4. Dashboard appears with Overview tab selected

### Accessing Quick Controls

Quick privacy controls are available via system tray:

1. Click the Privacy icon in system tray (top-right corner)
2. Select "Quick Settings" or right-click for context menu
3. Toggle microphone, camera, or network access globally
4. View current privacy score

### Checking Current Activity

The status bar shows real-time privacy activity:
- 🔴 **Red indicator** - Microphone active
- 🟠 **Orange indicator** - Camera active
- 🔵 **Blue indicator** - Network activity

---

## Privacy Dashboard

### Overview Tab

Displays system-wide privacy metrics:

- **System Privacy Score** - Overall privacy rating
- **Active Applications** - Number of running apps with permissions
- **Permissions Granted** - Total active permissions
- **Access Denied (24h)** - Permission requests that were denied
- **Top Privacy Violations** - Apps with most suspicious access patterns
- **Recommended Actions** - Suggested privacy improvements

### Applications Tab

Manage permissions and monitor privacy per application:

**Features:**
- Search applications by name or ID
- View privacy score per application
- See permission count and access statistics
- View last activity timestamp

**For each application:**
1. Click to select
2. View detailed information panel
3. See privacy score breakdown
4. Click "Manage Permissions" to adjust controls
5. Click "View Full History" for detailed access logs

### Activity Tab

Real-time access history with filtering:

**Filters Available:**
- **Type** - All, Microphone, Camera, File Access, Network, Denied
- **Time Range** - Today, Last 7 days, Last 30 days, All time

**Information Shown:**
- Timestamp of access request
- Application name
- Access type (microphone, camera, file, network)
- Action (requested, granted, denied, blocked)
- Details of the action
- Result (allowed/blocked)

### Recommendations Tab

Smart suggestions for privacy improvements:

**Recommendation Types:**
- **Critical** - Security-critical actions required
- **High** - Strong privacy recommendations
- **Medium** - Suggested improvements
- **Low** - Optional enhancements

**Acting on Recommendations:**
1. Review the recommendation title and description
2. Click "Apply Recommendation" to implement
3. Or click "Dismiss" to skip
4. Dashboard updates with new privacy score

### Permissions Tab

Master list of all granted permissions:

**Features:**
- Filter by category (Microphone, Camera, File Access, Network, Location)
- View last used timestamp
- Quick revoke action for any permission
- Export permission report to PDF or CSV

---

## Permission Management

### Understanding Permissions

**Microphone Access**
- Allows app to record audio and access microphone input
- Grant only to trusted apps (video calls, voice chat, recording)
- Indicator shows when app is actively recording

**Camera Access**
- Allows app to capture video and images
- Grant only to trusted apps (video conferencing, photo apps)
- LED indicator shows when camera is active

**File Access**
- Allows app to read, modify, or delete files
- Grant only to apps that legitimately need file access
- Protected system directories are always blocked

**Network Access**
- Allows app to send/receive data over internet
- DNS queries are logged (what domains accessed)
- VPN can be required for sensitive apps

### Managing Permissions

**To Grant Permission:**
1. App requests permission
2. Permission dialog appears with explanation
3. Choose "Allow", "Allow Once", or "Deny"
4. Optionally check "Remember this choice"

**To Revoke Permission:**
1. Open Privacy Dashboard
2. Navigate to Permissions tab
3. Find the application and permission
4. Click "Revoke Permission"
5. Permission is immediately revoked

**To Set Global Controls:**
1. Open Quick Settings from tray
2. Toggle "Global Microphone Access"
3. Toggle "Global Camera Access"
4. Toggle "Global Network Access"
5. All apps blocked from that resource globally

---

## Access Controls

### File Access Sandboxing

**Sandbox Directories** (per-application isolated storage):
- `~/.local/share/arbor-sandbox/{app-id}/` - Primary sandbox
- `~/Desktop` - User approved
- `~/Documents` - User approved
- `~/Downloads` - User approved

Apps are restricted to their sandbox by default. Additional directory access requires user approval.

**Protected System Paths** (always denied):
- `/etc/` - System configuration
- `/root/` - Root home directory
- `/boot/` - Boot files
- `/sys/` - System information
- `/proc/` - Process information
- `/dev/` - Device files
- `/lib/`, `/lib64/` - System libraries
- `/bin/`, `/sbin/` - System binaries
- `/usr/bin/`, `/usr/sbin/` - User binaries

**Allowing File Access:**

If an app needs access to a directory:
1. Dialog appears: "App requests access to {directory}"
2. Review the reason
3. Click "Allow" or "Deny"
4. Optionally "Remember this choice"

**Home Directory Access:**

Apps can access home subdirectories by default:
- `~/Documents/` - For productivity apps
- `~/Downloads/` - For downloaded files
- `~/Desktop/` - For desktop files
- Personal directories via explicit user approval

---

## Network Privacy

### DNS Query Logging

All DNS queries (domain name lookups) are logged and attributed to applications:

**Benefits:**
- Understand what domains apps are accessing
- Detect suspicious or unauthorized connections
- Identify tracking services and advertisements
- Review network activity history

**Viewing DNS Logs:**
1. Open Privacy Dashboard
2. Go to Activity tab
3. Filter by "Network" type
4. View all domain lookups

**Exported DNS Information:**
- Timestamp
- Application making the query
- Domain name
- Query type (A record, AAAA, MX, etc.)
- Resolution status (succeeded, failed, blocked)

### IP Filtering

Block or allow specific IP address ranges:

**Blacklist Mode** (deny specific IPs):
1. Dashboard > Permissions > Network
2. Add IP filter: Start IP - End IP
3. Mark as "Blacklist"
4. Connections to that range are blocked

**Whitelist Mode** (allow only specific IPs):
1. Dashboard > Permissions > Network
2. Add IP filter: Start IP - End IP
3. Mark as "Whitelist"
4. Only connections to whitelisted ranges allowed

### VPN Integration

**Requiring VPN for Apps:**

For sensitive applications (banking, messaging, email):
1. Dashboard > Recommendations
2. Find "Enable VPN for sensitive apps"
3. Configure which apps require VPN
4. Apps cannot access network without active VPN connection

**Supported Protocols:**
- OpenVPN
- WireGuard
- IKEv2
- Custom VPN solutions

**VPN Status:**
- Connected: Green indicator
- Disconnected: Red indicator
- Connecting: Yellow indicator

---

## Best Practices

### Daily Privacy Habits

1. **Review Recommendations** - Check Privacy Dashboard daily for suggestions
2. **Monitor Active Permissions** - Notice indicator lights for active resources
3. **Audit Recent Activity** - Weekly review of application access patterns
4. **Update Applications** - Keep apps updated for security patches
5. **Clear Old Audit Logs** - Periodically clear old access records

### Configuring New Applications

When installing a new application:

1. **Run Privacy Dashboard** - Check the app's initial permission requests
2. **Review Privacy Score** - See if app is accessing unexpected resources
3. **Adjust Permissions** - Revoke unnecessary permissions immediately
4. **Monitor First Week** - Watch for unusual access patterns
5. **Set Custom Policy** - Configure whitelist/blacklist for sensitive directories

### Protecting Sensitive Data

**For Banking/Finance Apps:**
1. Require VPN for all network access
2. Revoke file access permissions
3. Enable camera/microphone blocking
4. Monitor for unusual DNS queries

**For Communication Apps:**
1. Grant microphone/camera only when needed
2. Monitor network access and DNS queries
3. Review contact/calendar file access
4. Set up DNS filtering for ads

**For Work Productivity:**
1. Restrict file access to work directories
2. Monitor network activity for data exfiltration
3. Block unnecessary camera/microphone
4. Keep detailed audit trail

### Network Security

1. **Enable VPN on Public WiFi** - Always use VPN in public networks
2. **Monitor DNS Queries** - Review what domains apps access
3. **Block Tracking IPs** - Identify and blacklist ad/tracking networks
4. **Review Network Usage** - Check for unexpected data transfers
5. **Restrict App Network Access** - Deny network to apps that don't need it

---

## Troubleshooting

### Common Issues

**Q: App keeps requesting microphone permission**
- A: Check if app requires persistent recording
- A: Try "Allow Once" instead of "Allow"
- A: Revoke permission and restart app

**Q: Camera not working**
- A: Check if global camera is enabled in Quick Settings
- A: Verify app has camera permission in Permissions tab
- A: Look for LED indicator - if off, camera is blocked

**Q: App cannot access files**
- A: Check if path is in protected list (always blocked)
- A: Approve file access when dialog appears
- A: Verify app has file access permission granted

**Q: High network usage**
- A: Check Activity tab > Network filter
- A: Review which apps are accessing network
- A: Check DNS queries for suspicious domains
- A: Consider enabling VPN

**Q: Privacy score dropped**
- A: Review "Recommended Actions" in Overview
- A: Check Applications tab for suspicious apps
- A: Review Activity history for unexpected access
- A: Implement recommended privacy improvements

### Performance Issues

**Dashboard Running Slowly:**
1. Clear old audit logs: Privacy Dashboard > Settings > Clear History
2. Disable real-time refresh: Settings > Refresh Rate > Manual
3. Close other applications
4. Restart Privacy Dashboard

**High CPU Usage:**
1. Check if app is performing file scanning
2. Review network activity - may be downloading
3. Look for app stuck in permission loop
4. Restart the problematic application

### Permission Dialog Stuck

If a permission dialog won't close:
1. Click outside the dialog to cancel
2. Or press Escape key
3. Application will receive "Denied" response
4. Restart application if needed

### Resetting Privacy Settings

To reset all privacy settings to defaults:

**WARNING: This will:**
- Clear all permission history
- Reset all app-specific policies
- Remove all whitelists/blacklists
- Restore default protections

**To reset:**
1. Privacy Dashboard > Settings > Advanced
2. Click "Reset All Privacy Settings"
3. Confirm the warning dialog
4. System restarts privacy services

---

## Additional Resources

- **Privacy FAQ** - Frequently asked questions
- **Threats Mitigated** - Security threats addressed by Privacy Framework
- **Keyboard Shortcuts** - Quick access to privacy controls
- **Accessibility** - Privacy features for users with disabilities

## Support

For privacy-related issues:
1. Check this guide and FAQ
2. Review troubleshooting section
3. Contact system administrator
4. Submit privacy incident report

---

**Last Updated:** September 2026
**Privacy Framework Version:** 1.0
**Arbor OS Version:** 2.0+
