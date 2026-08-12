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
echo This takes 15-30 minutes
echo.

docker run --rm -it --privileged -v "%CD%:/workspace" fedora:39 bash -c "cd /tmp && dnf install -y genisoimage syslinux squashfs-tools dracut-live pciutils usbutils lshw dmidecode && mkdir -p build/work/rootfs build/iso/boot build/iso/isolinux build/iso/LiveOS && echo [BUILD] Bootstrapping Fedora base... && dnf --installroot=/tmp/build/work/rootfs --releasever=39 --setopt=install_weak_deps=False --setopt=cachedir=/tmp/dnf-cache --nodocs -y groupinstall 'Minimal Install' && echo [BUILD] Installing packages... && dnf --installroot=/tmp/build/work/rootfs --releasever=39 --setopt=cachedir=/tmp/dnf-cache -y install kernel systemd NetworkManager openssh-server sudo nano vim-minimal bash-completion curl wget git htop dracut-live dracut-network pciutils usbutils lshw dmidecode linux-firmware iwl*-firmware mesa-dri-drivers mesa-vulkan-drivers pipewire pipewire-alsa pipewire-pulseaudio wireplumber alsa-utils bluez bluez-tools libinput power-profiles-daemon upower fwupd iw wireless-tools && echo [BUILD] Configuring system... && echo arbor-os > /tmp/build/work/rootfs/etc/hostname && chroot /tmp/build/work/rootfs useradd -m -G wheel -s /bin/bash arbor 2>nul && chroot /tmp/build/work/rootfs bash -c 'echo arbor:arbor | chpasswd' && chroot /tmp/build/work/rootfs bash -c 'echo root:arbor | chpasswd' && echo '%%wheel ALL=(ALL) ALL' > /tmp/build/work/rootfs/etc/sudoers.d/wheel && chmod 0440 /tmp/build/work/rootfs/etc/sudoers.d/wheel && chroot /tmp/build/work/rootfs systemctl enable NetworkManager && chroot /tmp/build/work/rootfs systemctl enable sshd && chroot /tmp/build/work/rootfs systemctl enable bluetooth && chroot /tmp/build/work/rootfs systemctl enable fwupd && if [ -f /workspace/Code/system/hardware-detection.sh ]; then mkdir -p /tmp/build/work/rootfs/usr/local/bin && cp /workspace/Code/system/hardware-detection.sh /tmp/build/work/rootfs/usr/local/bin/arbor-hwinfo && chmod +x /tmp/build/work/rootfs/usr/local/bin/arbor-hwinfo && echo [BUILD] Hardware tool installed; fi && echo [BUILD] Building initramfs... && KERNEL_VER=$(ls /tmp/build/work/rootfs/lib/modules | head -n1) && chroot /tmp/build/work/rootfs dracut --force --no-hostonly --add 'dmsquash-live livenet' --omit plymouth /boot/initramfs-live.img $KERNEL_VER && echo [BUILD] Copying kernel... && cp /tmp/build/work/rootfs/boot/vmlinuz-* /tmp/build/iso/boot/vmlinuz && cp /tmp/build/work/rootfs/boot/initramfs-live.img /tmp/build/iso/boot/initramfs.img && echo [BUILD] Creating squashfs... && mksquashfs /tmp/build/work/rootfs /tmp/build/iso/LiveOS/squashfs.img -comp xz -b 1M && echo [BUILD] Configuring bootloader... && cat > /tmp/build/iso/isolinux/isolinux.cfg << 'EOF'
DEFAULT arbor
TIMEOUT 50
LABEL arbor
    KERNEL /boot/vmlinuz
    APPEND initrd=/boot/initramfs.img root=live:CDLABEL=ArborOS rd.live.image selinux=0 quiet
EOF
 && cp /usr/share/syslinux/isolinux.bin /tmp/build/iso/isolinux/ && cp /usr/share/syslinux/ldlinux.c32 /tmp/build/iso/isolinux/ && echo [BUILD] Building ISO... && mkisofs -o /tmp/ArborOS-phase3.iso -b isolinux/isolinux.bin -c isolinux/boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table -J -R -V ArborOS /tmp/build/iso && isohybrid /tmp/ArborOS-phase3.iso && echo [BUILD] Copying to Windows... && cp /tmp/ArborOS-phase3.iso /workspace/ArborOS-phase3.iso && ls -lh /workspace/ArborOS-phase3.iso && echo && echo ================================ && echo BUILD COMPLETE! && echo ================================"

echo.
echo ========================================
if exist ArborOS-phase3.iso (
    echo BUILD SUCCESS!
    echo ISO: ArborOS-phase3.iso
    for %%A in (ArborOS-phase3.iso) do echo Size: %%~zA bytes
) else (
    echo BUILD FAILED - Check errors above
)
echo ========================================
echo.
pause
