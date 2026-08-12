#!/bin/bash
# Arbor OS Hardware Detection & Information Tool
# Phase 3 - Hardware Support

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_header() {
    echo -e "${BLUE}=== $1 ===${NC}"
}

print_ok() {
    echo -e "${GREEN}✓${NC} $1"
}

print_warn() {
    echo -e "${YELLOW}⚠${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

# Check if command exists
has_cmd() {
    command -v "$1" &> /dev/null
}

# Main
echo ""
print_header "ARBOR OS HARDWARE REPORT"
echo "Generated: $(date)"
echo ""

# CPU Information
print_header "CPU"
if has_cmd lscpu; then
    lscpu | grep -E 'Model name|Architecture|CPU\(s\)|Thread|Core|MHz'
    
    # Detect CPU vendor
    cpu_vendor=$(lscpu | grep 'Vendor ID' | awk '{print $3}')
    if [[ "$cpu_vendor" == "GenuineIntel" ]]; then
        print_ok "Intel CPU detected"
        if [ -f /sys/devices/system/cpu/intel_pstate/status ]; then
            echo "  P-State: $(cat /sys/devices/system/cpu/intel_pstate/status)"
        fi
    elif [[ "$cpu_vendor" == "AuthenticAMD" ]]; then
        print_ok "AMD CPU detected"
        if [ -f /sys/devices/system/cpu/amd_pstate/status ]; then
            echo "  P-State: $(cat /sys/devices/system/cpu/amd_pstate/status)"
        fi
    fi
else
    print_error "lscpu not found"
fi
echo ""

# Memory
print_header "MEMORY"
if has_cmd free; then
    free -h
else
    print_error "free command not found"
fi
echo ""

# GPU
print_header "GRAPHICS"
if has_cmd lspci; then
    gpu_info=$(lspci | grep -i 'vga\|3d\|display')
    if [ -n "$gpu_info" ]; then
        echo "$gpu_info"
        
        # Detect GPU vendor
        if echo "$gpu_info" | grep -qi intel; then
            print_ok "Intel GPU detected (i915/xe driver)"
            if lsmod | grep -q i915; then
                echo "  Driver: i915 (loaded)"
            elif lsmod | grep -q xe; then
                echo "  Driver: xe (loaded)"
            fi
        fi
        
        if echo "$gpu_info" | grep -qi amd; then
            print_ok "AMD GPU detected (amdgpu driver)"
            if lsmod | grep -q amdgpu; then
                echo "  Driver: amdgpu (loaded)"
            fi
        fi
        
        if echo "$gpu_info" | grep -qi nvidia; then
            print_warn "NVIDIA GPU detected"
            if lsmod | grep -q nvidia; then
                echo "  Driver: nvidia proprietary (loaded)"
            elif lsmod | grep -q nouveau; then
                echo "  Driver: nouveau open source (loaded)"
                echo "  Note: Limited performance, install proprietary for gaming"
            else
                echo "  Driver: none loaded"
            fi
        fi
    else
        print_warn "No GPU detected"
    fi
else
    print_error "lspci not found"
fi
echo ""

# Network
print_header "NETWORK"
if has_cmd ip; then
    ip link show
    
    # Check for WiFi
    if has_cmd iw; then
        wifi_devs=$(iw dev | grep Interface | awk '{print $2}')
        if [ -n "$wifi_devs" ]; then
            print_ok "WiFi adapter(s): $wifi_devs"
        else
            print_warn "No WiFi adapter detected"
        fi
    fi
    
    # Check specific WiFi chipsets
    if has_cmd lspci; then
        if lspci | grep -qi 'intel.*wireless'; then
            print_ok "Intel WiFi detected (iwlwifi)"
        fi
        if lspci | grep -qi 'atheros'; then
            print_ok "Atheros WiFi detected"
        fi
        if lspci | grep -qi 'realtek.*wireless'; then
            print_warn "Realtek WiFi detected (may need firmware)"
        fi
    fi
else
    print_error "ip command not found"
fi
echo ""

# Storage
print_header "STORAGE"
if has_cmd lsblk; then
    lsblk -o NAME,SIZE,TYPE,FSTYPE,MOUNTPOINT
    
    # Check for NVMe
    if ls /dev/nvme* &> /dev/null; then
        print_ok "NVMe SSD detected"
    fi
else
    print_error "lsblk not found"
fi
echo ""

# USB
print_header "USB DEVICES"
if has_cmd lsusb; then
    lsusb
else
    print_error "lsusb not found"
fi
echo ""

# Audio
print_header "AUDIO"
if has_cmd aplay; then
    aplay -l
else
    print_error "aplay not found"
fi

# Check PipeWire
if systemctl --user is-active pipewire &> /dev/null; then
    print_ok "PipeWire is running"
elif systemctl --user is-active pulseaudio &> /dev/null; then
    print_warn "PulseAudio is running (PipeWire recommended)"
else
    print_error "No audio server detected"
fi
echo ""

# Bluetooth
print_header "BLUETOOTH"
if has_cmd bluetoothctl; then
    if systemctl is-active bluetooth &> /dev/null; then
        print_ok "Bluetooth service running"
        bluetoothctl show | head -5
    else
        print_warn "Bluetooth service not running"
    fi
else
    print_error "bluetoothctl not found"
fi
echo ""

# Battery (laptops)
print_header "BATTERY"
if [ -d /sys/class/power_supply/BAT* ] || [ -d /sys/class/power_supply/battery ]; then
    if has_cmd upower; then
        battery=$(upower -e | grep battery | head -1)
        if [ -n "$battery" ]; then
            print_ok "Battery detected"
            upower -i "$battery" | grep -E 'state|percentage|time'
        fi
    else
        print_warn "Battery present but upower not installed"
    fi
else
    echo "No battery detected (desktop system)"
fi
echo ""

# Power Management
print_header "POWER MANAGEMENT"
if [ -d /sys/devices/system/cpu/cpu0/cpufreq ]; then
    governor=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor)
    driver=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_driver)
    print_ok "CPU frequency scaling: $driver ($governor)"
else
    print_warn "CPU frequency scaling not available"
fi

# Check suspend support
if [ -f /sys/power/mem_sleep ]; then
    mem_sleep=$(cat /sys/power/mem_sleep)
    echo "Suspend modes: $mem_sleep"
fi
echo ""

# Kernel
print_header "KERNEL"
uname -r
echo "Loaded modules: $(lsmod | wc -l)"
echo ""

# Firmware
print_header "FIRMWARE"
if [ -d /lib/firmware ]; then
    fw_count=$(find /lib/firmware -type f 2>/dev/null | wc -l)
    print_ok "Firmware files: $fw_count"
else
    print_error "/lib/firmware not found"
fi

# Check for missing firmware
missing_fw=$(dmesg | grep -i 'firmware.*failed\|firmware.*not found' | tail -5)
if [ -n "$missing_fw" ]; then
    print_warn "Missing firmware detected:"
    echo "$missing_fw"
fi
echo ""

# Summary
print_header "SUMMARY"
echo "System: $(cat /etc/hostname 2>/dev/null || echo 'unknown')"
echo "Kernel: $(uname -r)"
echo "Architecture: $(uname -m)"
if has_cmd uptime; then
    uptime
fi
echo ""

print_ok "Hardware detection complete"
echo ""
echo "To save this report: $0 > hardware-report.txt"
echo "To submit hardware: hardware-report.txt to hardware@arboros.org"
echo ""

