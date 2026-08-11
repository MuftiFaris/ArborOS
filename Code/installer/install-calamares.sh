#!/bin/bash
# Calamares Installer Integration Script
# Phase 4 - Arbor OS Installer

set -e

ROOTFS="$1"

if [ -z "$ROOTFS" ]; then
    echo "Usage: $0 <rootfs_path>"
    exit 1
fi

echo "Installing Calamares to $ROOTFS"

# Install Calamares
dnf --installroot="$ROOTFS" --releasever=39 -y install \
    calamares \
    calamares-qml \
    qt5-qtbase \
    qt5-qtsvg \
    qt5-qtdeclarative \
    qt5-qtquickcontrols2 \
    kpmcore \
    parted \
    e2fsprogs \
    dosfstools \
    btrfs-progs \
    xfsprogs \
    grub2 \
    grub2-efi-x64 \
    efibootmgr \
    os-prober \
    dmidecode

echo "Copying Calamares configuration..."

# Create config directory
mkdir -p "$ROOTFS/etc/calamares"
mkdir -p "$ROOTFS/etc/calamares/modules"
mkdir -p "$ROOTFS/etc/calamares/branding/arbor"

# Copy configs
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cp "$SCRIPT_DIR/calamares/settings.conf" "$ROOTFS/etc/calamares/"
cp "$SCRIPT_DIR/calamares/modules/"*.conf "$ROOTFS/etc/calamares/modules/"
cp "$SCRIPT_DIR/calamares/branding/arbor/"* "$ROOTFS/etc/calamares/branding/arbor/"

echo "Calamares installation complete"
