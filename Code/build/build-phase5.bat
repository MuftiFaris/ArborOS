@echo off
REM ArborOS Phase 5 Build - LXQt Desktop
REM Double-click to build ISO

echo ========================================
echo ArborOS Phase 5 - Desktop Build
echo ========================================
echo.
echo Docker Desktop must be running
echo Build time: 30-45 minutes
echo.
pause

cd /d "%~dp0"
cd ..\..

echo.
echo [BUILD] Starting Docker container...
echo.

docker run --rm -it --privileged -v "%CD%:/workspace" fedora:39 /bin/bash -c "set -e && cd /tmp && echo '[1/10] Installing build tools...' && dnf install -y genisoimage syslinux squashfs-tools dracut-live pciutils usbutils lshw dmidecode 2>&1 | grep -v 'warning' && mkdir -p build/work/rootfs build/iso/boot build/iso/isolinux build/iso/LiveOS && echo '[2/10] Bootstrapping Fedora base (5-10 min)...' && dnf --installroot=/tmp/build/work/rootfs --releasever=39 --setopt=install_weak_deps=False --nodocs -y groupinstall 'Minimal Install' 2>&1 | grep -E 'Installing|Complete' && echo '[3/10] Installing core packages (5-10 min)...' && dnf --installroot=/tmp/build/work/rootfs --releasever=39 -y install kernel systemd NetworkManager openssh-server sudo nano vim-minimal bash-completion curl wget git htop dracut-live dracut-network 2>&1 | grep -E 'Installing|Complete' && echo '[4/10] Installing hardware support (3-5 min)...' && dnf --installroot=/tmp/build/work/rootfs --releasever=39 -y install pciutils usbutils lshw dmidecode linux-firmware mesa-dri-drivers mesa-vulkan-drivers pipewire pipewire-alsa pipewire-pulseaudio wireplumber alsa-utils bluez bluez-tools libinput power-profiles-daemon upower fwupd iw wireless-tools 2>&1 | grep -E 'Installing|Complete' && echo '[5/10] Installing LXQt desktop (10-15 min)...' && dnf --installroot=/tmp/build/work/rootfs --releasever=39 --setopt=install_weak_deps=False -y install lxqt-about lxqt-config lxqt-globalkeys lxqt-notificationd lxqt-panel lxqt-policykit lxqt-powermanagement lxqt-qtplugin lxqt-runner lxqt-session lxqt-sudo openbox obconf-qt pcmanfm-qt qterminal featherpad lightdm lightdm-gtk network-manager-applet pavucontrol-qt lxqt-archiver qps screengrab adwaita-qt5 breeze-icon-theme 2>&1 | grep -E 'Installing|Complete' && echo '[6/10] Configuring system...' && echo arbor-os > /tmp/build/work/rootfs/etc/hostname && chroot /tmp/build/work/rootfs useradd -m -G wheel -s /bin/bash arbor 2>/dev/null && chroot /tmp/build/work/rootfs bash -c 'echo arbor:arbor | chpasswd' && echo '%wheel ALL=(ALL) ALL' > /tmp/build/work/rootfs/etc/sudoers.d/wheel && chmod 0440 /tmp/build/work/rootfs/etc/sudoers.d/wheel && chroot /tmp/build/work/rootfs systemctl enable NetworkManager sshd bluetooth fwupd lightdm 2>/dev/null && chroot /tmp/build/work/rootfs systemctl set-default graphical.target 2>/dev/null && test -f /workspace/Code/system/hardware-detection.sh && cp /workspace/Code/system/hardware-detection.sh /tmp/build/work/rootfs/usr/local/bin/arbor-hwinfo && chmod +x /tmp/build/work/rootfs/usr/local/bin/arbor-hwinfo; mkdir -p /tmp/build/work/rootfs/etc/skel/.config/lxqt && printf '[Desktop]\nSession=lxqt\n' > /tmp/build/work/rootfs/etc/skel/.dmrc && mkdir -p /tmp/build/work/rootfs/etc/lightdm && printf '[Seat:*]\nautologin-user=arbor\nautologin-user-timeout=0\ngreeter-session=lightdm-gtk-greeter\n' > /tmp/build/work/rootfs/etc/lightdm/lightdm.conf && printf 'NAME=\"Arbor OS\"\nVERSION=\"0.5 (Phase 5)\"\nID=arbor-os\nID_LIKE=fedora\nVERSION_ID=0.5\nPRETTY_NAME=\"Arbor OS 0.5\"\nANSI_COLOR=\"0;32\"\n' > /tmp/build/work/rootfs/etc/os-release && echo '[7/10] Building initramfs (3-5 min)...' && KERNEL_VER=$(ls /tmp/build/work/rootfs/lib/modules | head -n1) && chroot /tmp/build/work/rootfs dracut --force --no-hostonly --add 'dmsquash-live livenet' --omit plymouth /boot/initramfs-live.img $KERNEL_VER 2>&1 | grep -v 'dracut:' && echo '[8/10] Copying kernel...' && cp /tmp/build/work/rootfs/boot/vmlinuz-* /tmp/build/iso/boot/vmlinuz && cp /tmp/build/work/rootfs/boot/initramfs-live.img /tmp/build/iso/boot/initramfs.img && echo '[9/10] Creating squashfs (10-15 min)...' && mksquashfs /tmp/build/work/rootfs /tmp/build/iso/LiveOS/squashfs.img -comp xz -b 1M -no-progress && echo '[10/10] Building ISO (2-5 min)...' && printf 'DEFAULT arbor\nTIMEOUT 50\nLABEL arbor\n    KERNEL /boot/vmlinuz\n    APPEND initrd=/boot/initramfs.img root=live:CDLABEL=ArborOS rd.live.image selinux=0 quiet\n' > /tmp/build/iso/isolinux/isolinux.cfg && cp /usr/share/syslinux/isolinux.bin /tmp/build/iso/isolinux/ && cp /usr/share/syslinux/ldlinux.c32 /tmp/build/iso/isolinux/ && mkisofs -quiet -o /tmp/ArborOS-0.5.iso -b isolinux/isolinux.bin -c isolinux/boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table -J -R -V ArborOS /tmp/build/iso && isohybrid /tmp/ArborOS-0.5.iso 2>/dev/null && mkdir -p /workspace/Code/build/output && cp /tmp/ArborOS-0.5.iso /workspace/Code/build/output/ArborOS-0.5.iso && SIZE=$(du -h /workspace/Code/build/output/ArborOS-0.5.iso | cut -f1) && echo && echo '================================' && echo 'BUILD COMPLETE!' && echo \"ISO: Code/build/output/ArborOS-0.5.iso ($SIZE)\" && echo '================================'"

if exist Code\build\output\ArborOS-0.5.iso (
    echo.
    echo ========================================
    echo SUCCESS! ISO Ready
    echo ========================================
    echo Location: Code\build\output\ArborOS-0.5.iso
    for %%A in (Code\build\output\ArborOS-0.5.iso) do echo Size: %%~zA bytes
    echo.
    echo Test in VirtualBox:
    echo 1. Create new VM
    echo 2. Load ArborOS-0.5.iso
    echo 3. Boot - LXQt desktop should appear
    echo ========================================
) else (
    echo.
    echo ========================================
    echo BUILD FAILED
    echo Check errors above
    echo ========================================
)

echo.
pause
