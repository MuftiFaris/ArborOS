#!/bin/bash
# ArborOS Docker Build Script
# Build ISO directly from Windows using Docker
# No VirtualBox needed

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE="/workspace"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

log() { echo -e "${GREEN}[BUILD]${NC} $*"; }
error() { echo -e "${RED}[ERROR]${NC} $*" >&2; exit 1; }
warn() { echo -e "${YELLOW}[WARN]${NC} $*"; }

# Check Docker running
if ! docker info &>/dev/null; then
    error "Docker not running. Start Docker Desktop first."
fi

log "ArborOS Docker Build"
log "===================="
log ""
log "Building in Docker container..."
log "Output: ArborOS-0.5.iso"
log ""

# Run build in Docker container
docker run --rm -it --privileged \
    -v "$(dirname "$(dirname "$SCRIPT_DIR")"):/workspace" \
    fedora:39 bash /workspace/Code/build/create_iso.sh

set -e

log() { echo "[BUILD] $*"; }
error() { echo "[ERROR] $*" >&2; exit 1; }

cd /tmp

log "Installing build tools..."
dnf install -y genisoimage syslinux squashfs-tools dracut-live \
    pciutils usbutils lshw dmidecode >/dev/null 2>&1

log "Creating build directories..."
mkdir -p build/work/rootfs build/iso/{boot,isolinux,LiveOS}

log "Bootstrapping Fedora base (2-5 min)..."
dnf --installroot=/tmp/build/work/rootfs \
    --releasever=39 \
    --setopt=install_weak_deps=False \
    --setopt=cachedir=/tmp/dnf-cache \
    --nodocs \
    -y groupinstall "Minimal Install" >/dev/null 2>&1

log "Installing packages (3-10 min)..."
dnf --installroot=/tmp/build/work/rootfs \
    --releasever=39 \
    --setopt=cachedir=/tmp/dnf-cache \
    -y install \
    kernel systemd NetworkManager openssh-server sudo \
    nano vim-minimal bash-completion curl wget git htop \
    dracut-live dracut-network \
    pciutils usbutils lshw dmidecode linux-firmware \
    iwl*-firmware mesa-dri-drivers mesa-vulkan-drivers \
    pipewire pipewire-alsa pipewire-pulseaudio wireplumber \
    alsa-utils bluez bluez-tools libinput \
    power-profiles-daemon upower fwupd \
    iw wireless-tools >/dev/null 2>&1

log "Configuring system..."
echo arbor-os > /tmp/build/work/rootfs/etc/hostname

chroot /tmp/build/work/rootfs useradd -m -G wheel -s /bin/bash arbor 2>/dev/null || true
chroot /tmp/build/work/rootfs bash -c "echo arbor:arbor | chpasswd"
chroot /tmp/build/work/rootfs bash -c "echo root:arbor | chpasswd"

echo "%wheel ALL=(ALL) ALL" > /tmp/build/work/rootfs/etc/sudoers.d/wheel
chmod 0440 /tmp/build/work/rootfs/etc/sudoers.d/wheel

chroot /tmp/build/work/rootfs systemctl enable NetworkManager
chroot /tmp/build/work/rootfs systemctl enable sshd
chroot /tmp/build/work/rootfs systemctl enable bluetooth
chroot /tmp/build/work/rootfs systemctl enable fwupd

# Copy arbor-hwinfo if exists
if [ -f "/workspace/Code/system/hardware-detection.sh" ]; then
    mkdir -p /tmp/build/work/rootfs/usr/local/bin
    cp /workspace/Code/system/hardware-detection.sh \
        /tmp/build/work/rootfs/usr/local/bin/arbor-hwinfo
    chmod +x /tmp/build/work/rootfs/usr/local/bin/arbor-hwinfo
    log "Hardware detection tool installed"
fi

log "Building initramfs..."
KERNEL_VER=$(ls /tmp/build/work/rootfs/lib/modules | head -n1)
chroot /tmp/build/work/rootfs dracut --force --no-hostonly \
    --add "dmsquash-live livenet" --omit plymouth \
    /boot/initramfs-live.img $KERNEL_VER 2>/dev/null

log "Copying kernel and initramfs..."
cp /tmp/build/work/rootfs/boot/vmlinuz-* /tmp/build/iso/boot/vmlinuz
cp /tmp/build/work/rootfs/boot/initramfs-live.img /tmp/build/iso/boot/initramfs.img

log "Creating squashfs (5-10 min)..."
mksquashfs /tmp/build/work/rootfs /tmp/build/iso/LiveOS/squashfs.img \
    -comp xz -b 1M 2>/dev/null

log "Configuring bootloader..."
cat > /tmp/build/iso/isolinux/isolinux.cfg <<EOF
DEFAULT arbor
TIMEOUT 50
LABEL arbor
    KERNEL /boot/vmlinuz
    APPEND initrd=/boot/initramfs.img root=live:CDLABEL=ArborOS rd.live.image selinux=0 quiet
EOF

cp /usr/share/syslinux/isolinux.bin /tmp/build/iso/isolinux/
cp /usr/share/syslinux/ldlinux.c32 /tmp/build/iso/isolinux/

log "Building ISO..."
mkisofs -o /tmp/ArborOS-phase3.iso \
    -b isolinux/isolinux.bin \
    -c isolinux/boot.cat \
    -no-emul-boot \
    -boot-load-size 4 \
    -boot-info-table \
    -J -R -V ArborOS \
    /tmp/build/iso 2>/dev/null

isohybrid /tmp/ArborOS-phase3.iso

# Copy to Windows shared folder
log "Copying ISO to Windows..."
cp /tmp/ArborOS-phase3.iso /workspace/ArborOS-phase3.iso

SIZE=$(du -h /workspace/ArborOS-phase3.iso | cut -f1)
log ""
log "================================"
log "BUILD COMPLETE!"
log "ISO: ArborOS-phase3.iso ($SIZE)"
log "Location: ArborOS root folder"
log "================================"
'

# Check if ISO created
if [ -f "$(dirname "$(dirname "$SCRIPT_DIR")")/Code/build/output/ArborOS-0.5.iso" ]; then
    log ""
    log "✅ ISO ready: Code/build/output/ArborOS-0.5.iso"
    log ""
    log "Test: VirtualBox > New VM > Use ISO"
else
    error "Build failed - ISO not found"
fi
