@echo off
REM ArborOS Windows Build Script
REM Double-click to build ISO using Docker

echo ========================================
echo ArborOS Docker Build
echo ========================================
echo.
echo Requirements:
echo - Docker Desktop running
echo - WSL2 enabled
echo.
pause

cd /d "%~dp0"
cd ..\..

echo Starting build in Docker...
echo This takes 30-45 minutes
echo.

docker run --rm -it --privileged -v "%CD%:/workspace" fedora:39 bash -c "
set -e
cd /tmp
echo '[BUILD] Installing tools...'
dnf install -y genisoimage syslinux squashfs-tools dracut-live pciutils usbutils lshw dmidecode

echo '[BUILD] Creating directories...'
mkdir -p build/work/rootfs build/iso/boot build/iso/isolinux build/iso/LiveOS

echo '[BUILD] Bootstrapping Fedora (5-10 min)...'
dnf --installroot=/tmp/build/work/rootfs --releasever=39 --setopt=install_weak_deps=False --nodocs -y groupinstall 'Minimal Install'

echo '[BUILD] Installing system packages (5-15 min)...'
dnf --installroot=/tmp/build/work/rootfs --releasever=39 -y install kernel systemd NetworkManager openssh-server sudo nano vim-minimal bash-completion curl wget git htop dracut-live dracut-network pciutils usbutils lshw dmidecode linux-firmware iwl*-firmware mesa-dri-drivers mesa-vulkan-drivers pipewire pipewire-alsa pipewire-pulseaudio wireplumber alsa-utils bluez bluez-tools libinput power-profiles-daemon upower fwupd iw wireless-tools

echo '[BUILD] Installing LXQt desktop (10-20 min)...'
dnf --installroot=/tmp/build/work/rootfs --releasever=39 --setopt=install_weak_deps=False -y install lxqt-about lxqt-config lxqt-globalkeys lxqt-notificationd lxqt-openssh-askpass lxqt-panel lxqt-policykit lxqt-powermanagement lxqt-qtplugin lxqt-runner lxqt-session lxqt-sudo openbox obconf-qt pcmanfm-qt qterminal featherpad lightdm lightdm-gtk network-manager-applet pavucontrol-qt lxqt-archiver qps screengrab xscreensaver adwaita-qt5 breeze-icon-theme

echo '[BUILD] Configuring system...'
echo arbor-os > /tmp/build/work/rootfs/etc/hostname
chroot /tmp/build/work/rootfs useradd -m -G wheel -s /bin/bash arbor
chroot /tmp/build/work/rootfs bash -c 'echo arbor:arbor | chpasswd'
echo '%wheel ALL=(ALL) ALL' > /tmp/build/work/rootfs/etc/sudoers.d/wheel
chmod 0440 /tmp/build/work/rootfs/etc/sudoers.d/wheel
chroot /tmp/build/work/rootfs systemctl enable NetworkManager sshd bluetooth fwupd lightdm
chroot /tmp/build/work/rootfs systemctl set-default graphical.target

if [ -f /workspace/Code/system/hardware-detection.sh ]; then
  cp /workspace/Code/system/hardware-detection.sh /tmp/build/work/rootfs/usr/local/bin/arbor-hwinfo
  chmod +x /tmp/build/work/rootfs/usr/local/bin/arbor-hwinfo
fi

mkdir -p /tmp/build/work/rootfs/etc/skel/.config/lxqt
echo '[Desktop]' > /tmp/build/work/rootfs/etc/skel/.dmrc
echo 'Session=lxqt' >> /tmp/build/work/rootfs/etc/skel/.dmrc

mkdir -p /tmp/build/work/rootfs/etc/lightdm
cat > /tmp/build/work/rootfs/etc/lightdm/lightdm.conf << 'EOFLDM'
[Seat:*]
autologin-user=arbor
autologin-user-timeout=0
greeter-session=lightdm-gtk-greeter
EOFLDM

cat > /tmp/build/work/rootfs/etc/os-release << 'EOFOS'
NAME=\"Arbor OS\"
VERSION=\"0.5 (Phase 5 - Desktop)\"
ID=arbor-os
ID_LIKE=fedora
VERSION_ID=0.5
PRETTY_NAME=\"Arbor OS 0.5 (Phase 5)\"
ANSI_COLOR=\"0;32\"
HOME_URL=\"https://github.com/MuftiFaris/ArborOS\"
BUG_REPORT_URL=\"https://github.com/MuftiFaris/ArborOS/issues\"
EOFOS

echo '[BUILD] Building initramfs (3-5 min)...'
KERNEL_VER=\$(ls /tmp/build/work/rootfs/lib/modules | head -n1)
chroot /tmp/build/work/rootfs dracut --force --no-hostonly --add 'dmsquash-live livenet' --omit plymouth /boot/initramfs-live.img \$KERNEL_VER

echo '[BUILD] Copying kernel...'
cp /tmp/build/work/rootfs/boot/vmlinuz-* /tmp/build/iso/boot/vmlinuz
cp /tmp/build/work/rootfs/boot/initramfs-live.img /tmp/build/iso/boot/initramfs.img

echo '[BUILD] Creating squashfs (5-15 min)...'
mksquashfs /tmp/build/work/rootfs /tmp/build/iso/LiveOS/squashfs.img -comp xz -b 1M

echo '[BUILD] Configuring bootloader...'
cat > /tmp/build/iso/isolinux/isolinux.cfg << 'EOFISO'
DEFAULT arbor
TIMEOUT 50
LABEL arbor
    KERNEL /boot/vmlinuz
    APPEND initrd=/boot/initramfs.img root=live:CDLABEL=ArborOS rd.live.image selinux=0 quiet
EOFISO

cp /usr/share/syslinux/isolinux.bin /tmp/build/iso/isolinux/
cp /usr/share/syslinux/ldlinux.c32 /tmp/build/iso/isolinux/

echo '[BUILD] Building ISO (2-5 min)...'
mkisofs -o /tmp/ArborOS-0.5.iso -b isolinux/isolinux.bin -c isolinux/boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table -J -R -V ArborOS /tmp/build/iso
isohybrid /tmp/ArborOS-0.5.iso

echo '[BUILD] Copying to Windows...'
mkdir -p /workspace/Code/build/output
cp /tmp/ArborOS-0.5.iso /workspace/Code/build/output/ArborOS-0.5.iso
ls -lh /workspace/Code/build/output/ArborOS-0.5.iso

echo
echo '================================'
echo 'BUILD COMPLETE!'
echo 'ISO: Code/build/output/ArborOS-0.5.iso'
echo '================================'
"

echo.
echo ========================================
if exist Code\build\output\ArborOS-0.5.iso (
    echo BUILD SUCCESS!
    echo ISO: Code\build\output\ArborOS-0.5.iso
    for %%A in (Code\build\output\ArborOS-0.5.iso) do echo Size: %%~zA bytes
) else (
    echo BUILD FAILED - Check errors above
)
echo ========================================
echo.
pause
