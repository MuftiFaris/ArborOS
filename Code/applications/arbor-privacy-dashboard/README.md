# Arbor Privacy Dashboard

Comprehensive privacy management and monitoring application for ArborOS.

## Features

### Overview Tab
- System privacy score (0-100 scale)
- Real-time activity monitoring
- Top privacy violations indicator
- Recommended actions for privacy improvements

### Applications Tab
- List of all installed applications with privacy ratings
- Per-application privacy score calculation
- Permissions granted per application
- Access attempt counts and denial tracking
- Search and filter applications
- Detailed view of selected application

### Activity Tab
- Real-time access history (microphone, camera, file access, network)
- Filterable by access type (microphone, camera, file, network, denied)
- Filterable by time range (today, 7 days, 30 days, all time)
- Timestamp, application name, access details, and result
- 5000+ historical records

### Recommendations Tab
- Smart privacy recommendations based on app behavior
- Severity levels (critical, high, medium, low)
- Actionable recommendations
- Dismiss or apply recommendations

### Permissions Tab
- Master list of all granted permissions
- Category filtering (microphone, camera, file access, network, location)
- Last used timestamp
- Quick revoke action
- Export privacy report to PDF or CSV

## Building

### Requirements
- Qt 6.0 or later
- CMake 3.16+
- C++17 compiler

### Build Steps

```bash
mkdir build
cd build
cmake ..
make
sudo make install
```

## Running

```bash
arbor-privacy-dashboard
```

## Architecture

### Main Components

1. **PrivacyDashboardWidget** - Main UI component with tabbed interface
   - Overview tab for system-wide privacy metrics
   - Applications tab for per-app management
   - Activity tab for access history
   - Recommendations tab for suggested actions
   - Permissions tab for permission management

2. **Data Models**
   - AppPrivacyInfo: Per-application privacy metrics
   - PrivacyRecommendation: Actionable privacy recommendations

3. **Real-time Updates**
   - Auto-refresh every 5 seconds
   - Qt signals/slots for event handling
   - Live activity monitoring

## Integration with PrivacyManager

The dashboard connects to the PrivacyManager D-Bus service to:
- Retrieve real-time access history
- Query application permissions
- Apply permission changes
- Track network usage
- Monitor camera/microphone activity
- Analyze access patterns

## Privacy Score Calculation

Score based on:
- Microphone/camera access (weight: 20%)
- File access patterns (weight: 20%)
- Network activity and DNS queries (weight: 30%)
- Permission count (weight: 15%)
- Access denial rate (weight: 15%)

Score ranges:
- 0-25: Critical privacy concerns
- 26-50: Significant privacy issues
- 51-75: Acceptable with caution
- 76-100: Strong privacy protection

## Recommendations Engine

Generates recommendations based on:
- Uncommon access patterns
- Suspicious network connections
- VPN requirements for sensitive apps
- Permission usage analysis
- Comparative analysis with other applications

## Data Export

Export privacy reports in:
- PDF format (human-readable summary)
- CSV format (detailed records for analysis)

## File Structure

```
arbor-privacy-dashboard/
├── main.cpp
├── privacy-dashboard-widget.h
├── privacy-dashboard-widget.cpp
├── CMakeLists.txt
└── README.md
```

## Future Enhancements

- Real-time graph visualization of privacy metrics
- Machine learning-based threat detection
- Custom privacy policies per application
- Integration with system audit logs
- Privacy profile templates (strict, balanced, permissive)
- Scheduled privacy reports
- Integration with mobile app for remote monitoring
