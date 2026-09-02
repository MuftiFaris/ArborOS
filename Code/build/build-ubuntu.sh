#!/bin/bash
# ArborOS Phase 5 Builder for Ubuntu VM
# Builds via Docker Fedora container

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"

echo "========================================"
echo "ArborOS Phase 5 - Desktop Build"
echo "========================================"
echo ""
echo "Building via Docker Fedora container..."
echo ""

# Check Docker installed
if ! command -v docker &> /dev/null; then
    echo "ERROR: Docker not installed"
    echo "Install: sudo apt install docker.io -y"
    exit 1
fi

# Run build in Fedora container
sudo docker run --rm -it --privileged \
  -v "$PROJECT_ROOT:/workspace" \
  fedora:39 \
  bash -c '
set -e

echo "[1/9] Installing build tools..."
dnf install -y genisoimage syslinux squashfs-tools dracut-live e2fsprogs pciutils usbutils lshw dmidecode ca-certificates

cd /tmp
mkdir -p build/work/rootfs build/iso/boot build/iso/isolinux build/iso/LiveOS

echo "[2/9] Bootstrapping Fedora..."
dnf --installroot=/tmp/build/work/rootfs \
    --releasever=39 \
    --setopt=install_weak_deps=False \
    --setopt=cachedir=/tmp/dnf-cache \
    --nodocs \
    -y groupinstall "Minimal Install"

echo "[3/9] Installing packages..."
dnf --installroot=/tmp/build/work/rootfs \
    --releasever=39 \
    --setopt=cachedir=/tmp/dnf-cache \
    -y install \
    kernel systemd NetworkManager openssh-server sudo nano \
    bash-completion dracut-live dracut-network linux-firmware \
    mesa-dri-drivers pipewire wireplumber alsa-utils \
    lxqt-panel lxqt-session lxqt-config openbox \
    pcmanfm-qt qterminal featherpad lightdm lightdm-gtk lightdm-gtk-greeter-settings \
    network-manager-applet breeze-icon-theme xorg-x11-server-Xorg xorg-x11-xinit \
    htop wget curl git

echo "[4/9] Configuring system..."
mount -t proc /proc /tmp/build/work/rootfs/proc
mount -t sysfs /sys /tmp/build/work/rootfs/sys
mount --bind /dev /tmp/build/work/rootfs/dev
mount --bind /dev/pts /tmp/build/work/rootfs/dev/pts
echo arbor-os > /tmp/build/work/rootfs/etc/hostname
chroot /tmp/build/work/rootfs useradd -m -G wheel -s /bin/bash arbor
chroot /tmp/build/work/rootfs bash -c "echo arbor:arbor | chpasswd"
echo "%wheel ALL=(ALL) ALL" > /tmp/build/work/rootfs/etc/sudoers.d/wheel
chmod 0440 /tmp/build/work/rootfs/etc/sudoers.d/wheel
chroot /tmp/build/work/rootfs systemctl enable NetworkManager lightdm
chroot /tmp/build/work/rootfs systemctl set-default graphical.target

mkdir -p /tmp/build/work/rootfs/etc/lightdm
cat > /tmp/build/work/rootfs/etc/lightdm/lightdm.conf <<EOF
[Seat:*]
autologin-user=arbor
autologin-user-timeout=0
EOF

if [ -f /workspace/Code/system/hardware-detection.sh ]; then
    cp /workspace/Code/system/hardware-detection.sh /tmp/build/work/rootfs/usr/local/bin/arbor-hwinfo
    chmod +x /tmp/build/work/rootfs/usr/local/bin/arbor-hwinfo
    echo "Hardware detection script installed"
else
    echo "Warning: hardware-detection.sh not found"
fi

echo "[5/9] Building initramfs..."
KVER=$(ls /tmp/build/work/rootfs/lib/modules | head -n1)
chroot /tmp/build/work/rootfs dracut --force --no-hostonly \
    --add "dmsquash-live livenet" \
    --omit plymouth \
    /boot/initramfs-live.img $KVER
umount -l /tmp/build/work/rootfs/proc || true
umount -l /tmp/build/work/rootfs/sys || true
umount -l /tmp/build/work/rootfs/dev/pts || true
umount -l /tmp/build/work/rootfs/dev || true

echo "[6/9] Copying kernel..."
mkdir -p /tmp/build/iso/boot
KERNEL_FILE=$(ls /tmp/build/work/rootfs/boot/vmlinuz-* 2>/dev/null | head -n1)
if [ -z "$KERNEL_FILE" ]; then
    echo "ERROR: Kernel not found"
    exit 1
fi
cp "$KERNEL_FILE" /tmp/build/iso/boot/vmlinuz
cp /tmp/build/work/rootfs/boot/initramfs-live.img /tmp/build/iso/boot/initramfs.img
echo "Kernel copied: $(basename $KERNEL_FILE)"

echo "[7/9] Creating squashfs..."
mksquashfs /tmp/build/work/rootfs /tmp/build/iso/LiveOS/squashfs.img -comp xz -b 1M

echo "[8/9] Configuring bootloader..."
cat > /tmp/build/iso/isolinux/isolinux.cfg <<EOF
DEFAULT arbor
TIMEOUT 50
LABEL arbor
    KERNEL /boot/vmlinuz
    APPEND initrd=/boot/initramfs.img root=live:CDLABEL=ArborOS rd.live.image selinux=0 quiet
EOF

cp /usr/share/syslinux/isolinux.bin /tmp/build/iso/isolinux/
cp /usr/share/syslinux/ldlinux.c32 /tmp/build/iso/isolinux/

echo "[9/9] Building ISO..."
mkisofs -o /tmp/ArborOS-0.5.iso \
    -b isolinux/isolinux.bin \
    -c isolinux/boot.cat \
    -no-emul-boot \
    -boot-load-size 4 \
    -boot-info-table \
    -J -R -V ArborOS \
    /tmp/build/iso

isohybrid /tmp/ArborOS-0.5.iso

mkdir -p /workspace/Code/build/output
cp /tmp/ArborOS-0.5.iso /workspace/Code/build/output/

echo ""
echo "========================================"
echo "BUILD COMPLETE!"
echo "========================================"
ls -lh /workspace/Code/build/output/ArborOS-0.5.iso
'

echo ""
echo "========================================"
echo "Build finished!"
echo "ISO: Code/build/output/ArborOS-0.5.iso"
echo "========================================"
