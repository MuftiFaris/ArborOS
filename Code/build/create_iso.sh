#!/bin/bash
# Arbor OS ISO Builder - Phase 3
# Version: 0.2
# Hardware Support Build
# Builds Fedora-based system with full hardware compatibility

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_ROOT}/build"
WORK_DIR="${BUILD_DIR}/work"
ISO_DIR="${BUILD_DIR}/iso"
OUTPUT_DIR="${BUILD_DIR}/output"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

log() {
    echo -e "${GREEN}[BUILD]${NC} $*"
}

error() {
    echo -e "${RED}[ERROR]${NC} $*" >&2
    exit 1
}

warn() {
    echo -e "${YELLOW}[WARN]${NC} $*"
}

# Check if running on Linux
if [[ "$(uname -s)" != "Linux" ]]; then
    error "This script must run on Linux. Use a Linux VM."
fi

# Check for required tools
check_dependencies() {
    log "Checking dependencies..."
    
    local deps=(
        "dnf"
        "mkisofs"
        "isohybrid"
        "mksquashfs"
        "mkfs.ext4"
    )
    
    local missing=()
    for dep in "${deps[@]}"; do
        if ! command -v "$dep" &> /dev/null; then
            missing+=("$dep")
        fi
    done
    
    if [[ ${#missing[@]} -gt 0 ]]; then
        error "Missing dependencies: ${missing[*]}"
    fi
    
    log "All dependencies present"
}

# Clean previous build
clean_build() {
    log "Cleaning previous build..."
    rm -rf "$WORK_DIR" "$ISO_DIR"
    mkdir -p "$WORK_DIR" "$ISO_DIR" "$OUTPUT_DIR"
}

# Create base rootfs
create_rootfs() {
    log "Creating root filesystem..."
    
    local ROOTFS="${WORK_DIR}/rootfs"
    mkdir -p "$ROOTFS"
    
    log "Bootstrapping Fedora base system..."
    dnf --installroot="$ROOTFS" \
        --releasever=39 \
        --setopt=install_weak_deps=False \
        --nodocs \
        -y groupinstall "Minimal Install"
    
    log "Installing essential packages..."
    dnf --installroot="$ROOTFS" \
        --releasever=39 \
        --setopt=install_weak_deps=False \
        --nodocs \
        -y install \
        kernel \
        systemd \
        NetworkManager \
        openssh-server \
        sudo \
        nano \
        vim-minimal \
        bash-completion \
        curl \
        wget \
        git \
        htop \
        dracut-live \
        dracut-network
    
    log "Installing hardware support packages..."
    dnf --installroot="$ROOTFS" \
        --releasever=39 \
        --setopt=install_weak_deps=False \
        --nodocs \
        -y install \
        pciutils \
        usbutils \
        lshw \
        dmidecode \
        linux-firmware \
        iwl*-firmware \
        mesa-dri-drivers \
        mesa-vulkan-drivers \
        pipewire \
        pipewire-alsa \
        pipewire-pulseaudio \
        wireplumber \
        alsa-utils \
        bluez \
        bluez-tools \
        libinput \
        power-profiles-daemon \
        upower \
        fwupd \
        iw \
        wireless-tools
    
    log "Rootfs created with hardware support"
}

# Configure system
configure_system() {
    log "Configuring system..."
    
    local ROOTFS="${WORK_DIR}/rootfs"
    
    echo "arbor-os" > "$ROOTFS/etc/hostname"
    
    cat > "$ROOTFS/etc/hosts" <<EOF
127.0.0.1   localhost
127.0.1.1   arbor-os
::1         localhost ip6-localhost ip6-loopback
EOF
    
    chroot "$ROOTFS" useradd -m -G wheel -s /bin/bash arbor 2>/dev/null || true
    chroot "$ROOTFS" sh -c 'echo "arbor:arbor" | chpasswd'
    chroot "$ROOTFS" sh -c 'echo "root:arbor" | chpasswd'
    
    echo "%wheel ALL=(ALL) ALL" > "$ROOTFS/etc/sudoers.d/wheel"
    chmod 0440 "$ROOTFS/etc/sudoers.d/wheel"
    
    chroot "$ROOTFS" systemctl enable NetworkManager
    chroot "$ROOTFS" systemctl enable sshd
    chroot "$ROOTFS" systemctl enable bluetooth
    chroot "$ROOTFS" systemctl enable fwupd
    
    # Enable PipeWire for user session
    mkdir -p "$ROOTFS/etc/skel/.config/systemd/user/default.target.wants"
    chroot "$ROOTFS" systemctl --user --global enable pipewire pipewire-pulse wireplumber 2>/dev/null || true
    
    chroot "$ROOTFS" ln -sf /usr/share/zoneinfo/UTC /etc/localtime
    echo "LANG=en_US.UTF-8" > "$ROOTFS/etc/locale.conf"
    
    # Copy hardware detection script
    mkdir -p "$ROOTFS/usr/local/bin"
    if [ -f "$PROJECT_ROOT/system/hardware-detection.sh" ]; then
        cp "$PROJECT_ROOT/system/hardware-detection.sh" "$ROOTFS/usr/local/bin/arbor-hwinfo"
        chmod +x "$ROOTFS/usr/local/bin/arbor-hwinfo"
        log "Hardware detection script installed"
    fi
    
    log "System configured with hardware support"
}

# Create initramfs with live support
create_initramfs() {
    log "Creating initramfs with live modules..."
    
    local ROOTFS="${WORK_DIR}/rootfs"
    local KERNEL_VERSION=$(ls "$ROOTFS/lib/modules" | head -n1)
    
    chroot "$ROOTFS" dracut --force \
        --no-hostonly \
        --add "dmsquash-live livenet" \
        --omit plymouth \
        "/boot/initramfs-live.img" \
        "$KERNEL_VERSION"
    
    log "Initramfs created"
}

# Setup bootloader
setup_bootloader() {
    log "Setting up bootloader..."
    
    local ROOTFS="${WORK_DIR}/rootfs"
    local KERNEL_VERSION=$(ls "$ROOTFS/lib/modules" | head -n1)
    
    mkdir -p "$ISO_DIR/boot"
    cp "$ROOTFS/boot/vmlinuz-${KERNEL_VERSION}" "$ISO_DIR/boot/vmlinuz"
    cp "$ROOTFS/boot/initramfs-live.img" "$ISO_DIR/boot/initramfs.img"
    
    mkdir -p "$ISO_DIR/isolinux"
    cat > "$ISO_DIR/isolinux/isolinux.cfg" <<EOF
DEFAULT arbor
TIMEOUT 50
LABEL arbor
    KERNEL /boot/vmlinuz
    APPEND initrd=/boot/initramfs.img root=live:CDLABEL=ArborOS rd.live.image selinux=0 quiet
EOF
    
    cp /usr/share/syslinux/isolinux.bin "$ISO_DIR/isolinux/"
    cp /usr/share/syslinux/ldlinux.c32 "$ISO_DIR/isolinux/"
    
    log "Bootloader configured"
}

# Create squashfs with proper live structure
create_squashfs() {
    log "Creating squashfs filesystem..."
    
    local ROOTFS="${WORK_DIR}/rootfs"
    
    # Create LiveOS structure with actual filesystem contents
    mkdir -p /tmp/liveos-build/LiveOS
    
    log "Copying rootfs contents to LiveOS structure..."
    cp -a "$ROOTFS"/* /tmp/liveos-build/LiveOS/
    
    mkdir -p "$ISO_DIR/LiveOS"
    
    log "Compressing filesystem to squashfs..."
    mksquashfs /tmp/liveos-build/LiveOS "$ISO_DIR/LiveOS/squashfs.img" \
        -comp xz \
        -b 1M \
        -noappend
    
    rm -rf /tmp/liveos-build
    
    log "Squashfs created"
}

# Build ISO
build_iso() {
    log "Building ISO image..."
    
    local ISO_LABEL="ArborOS"
    local ISO_FILE="${OUTPUT_DIR}/ArborOS-0.2.iso"
    
    mkisofs -o "$ISO_FILE" \
        -b isolinux/isolinux.bin \
        -c isolinux/boot.cat \
        -no-emul-boot \
        -boot-load-size 4 \
        -boot-info-table \
        -J -R -V "$ISO_LABEL" \
        "$ISO_DIR"
    
    isohybrid "$ISO_FILE"
    
    log "ISO created: $ISO_FILE"
    log "Size: $(du -h "$ISO_FILE" | cut -f1)"
    log "Version: 0.2 (Phase 3 - Hardware Support)"
}

# Main
main() {
    log "Arbor OS ISO Builder - Phase 3"
    log "Version: 0.2 - Hardware Support"
    log "================================"
    
    check_dependencies
    clean_build
    create_rootfs
    configure_system
    create_initramfs
    setup_bootloader
    create_squashfs
    build_iso
    
    log ""
    log "Build complete!"
    log "ISO: ${OUTPUT_DIR}/ArborOS-0.2.iso"
    log "Changes from 0.1:"
    log "  - Full hardware support (GPU, WiFi, Audio, Power)"
    log "  - PipeWire audio stack"
    log "  - Bluetooth support"
    log "  - Power management"
    log "  - Hardware detection tool (arbor-hwinfo)"
    log ""
    log "Test with: qemu-system-x86_64 -m 2G -cdrom ${OUTPUT_DIR}/ArborOS-0.2.iso"
}

main "$@"
