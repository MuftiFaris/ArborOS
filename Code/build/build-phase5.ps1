# ArborOS Phase 5 Build - LXQt Desktop
# Right-click > Run with PowerShell

Write-Host "========================================" -ForegroundColor Green
Write-Host "ArborOS Phase 5 - Desktop Build" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Requirements:" -ForegroundColor Yellow
Write-Host "- Docker Desktop running"
Write-Host "- Build time: 30-45 minutes"
Write-Host ""
Pause

Set-Location $PSScriptRoot
Set-Location ..\..

Write-Host ""
Write-Host "[BUILD] Starting Docker build..." -ForegroundColor Cyan
Write-Host ""

docker run --rm -it --privileged -v "${PWD}:/workspace" fedora:39 /bin/bash -c 'set -e && dnf install -y genisoimage syslinux squashfs-tools dracut-live 2>&1 | grep -v warning && mkdir -p /tmp/build/work/rootfs /tmp/build/iso/boot /tmp/build/iso/isolinux /tmp/build/iso/LiveOS && echo "[2/10] Bootstrapping Fedora..." && dnf --installroot=/tmp/build/work/rootfs --releasever=39 --setopt=install_weak_deps=False --nodocs -y groupinstall "Minimal Install" 2>&1 | grep -E "Installing|Complete" && echo "[3/10] Installing packages..." && dnf --installroot=/tmp/build/work/rootfs --releasever=39 -y install kernel systemd NetworkManager openssh-server sudo nano bash-completion dracut-live dracut-network linux-firmware mesa-dri-drivers pipewire wireplumber alsa-utils 2>&1 | grep -E "Installing|Complete" && echo "[4/10] Installing LXQt..." && dnf --installroot=/tmp/build/work/rootfs --releasever=39 --setopt=install_weak_deps=False -y install lxqt-panel lxqt-session lxqt-config openbox pcmanfm-qt qterminal featherpad lightdm network-manager-applet breeze-icon-theme 2>&1 | grep -E "Installing|Complete" && echo "[5/10] Configuring..." && echo arbor-os > /tmp/build/work/rootfs/etc/hostname && chroot /tmp/build/work/rootfs useradd -m -G wheel -s /bin/bash arbor && chroot /tmp/build/work/rootfs bash -c "echo arbor:arbor | chpasswd" && echo "%wheel ALL=(ALL) ALL" > /tmp/build/work/rootfs/etc/sudoers.d/wheel && chmod 0440 /tmp/build/work/rootfs/etc/sudoers.d/wheel && chroot /tmp/build/work/rootfs systemctl enable NetworkManager lightdm && chroot /tmp/build/work/rootfs systemctl set-default graphical.target && test -f /workspace/Code/system/hardware-detection.sh && cp /workspace/Code/system/hardware-detection.sh /tmp/build/work/rootfs/usr/local/bin/arbor-hwinfo && chmod +x /tmp/build/work/rootfs/usr/local/bin/arbor-hwinfo || true && mkdir -p /tmp/build/work/rootfs/etc/lightdm && printf "[Seat:*]\nautologin-user=arbor\nautologin-user-timeout=0\n" > /tmp/build/work/rootfs/etc/lightdm/lightdm.conf && echo "[6/10] Building initramfs..." && KERNEL_VER=$(ls /tmp/build/work/rootfs/lib/modules | head -n1) && chroot /tmp/build/work/rootfs dracut --force --no-hostonly --add "dmsquash-live livenet" --omit plymouth /boot/initramfs-live.img $KERNEL_VER && echo "[7/10] Copying kernel..." && cp /tmp/build/work/rootfs/boot/vmlinuz-* /tmp/build/iso/boot/vmlinuz && cp /tmp/build/work/rootfs/boot/initramfs-live.img /tmp/build/iso/boot/initramfs.img && echo "[8/10] Creating squashfs..." && mksquashfs /tmp/build/work/rootfs /tmp/build/iso/LiveOS/squashfs.img -comp xz -b 1M -no-progress && echo "[9/10] Building ISO..." && printf "DEFAULT arbor\nTIMEOUT 50\nLABEL arbor\n    KERNEL /boot/vmlinuz\n    APPEND initrd=/boot/initramfs.img root=live:CDLABEL=ArborOS rd.live.image selinux=0 quiet\n" > /tmp/build/iso/isolinux/isolinux.cfg && cp /usr/share/syslinux/isolinux.bin /tmp/build/iso/isolinux/ && cp /usr/share/syslinux/ldlinux.c32 /tmp/build/iso/isolinux/ && mkisofs -quiet -o /tmp/ArborOS-0.5.iso -b isolinux/isolinux.bin -c isolinux/boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table -J -R -V ArborOS /tmp/build/iso && isohybrid /tmp/ArborOS-0.5.iso && mkdir -p /workspace/Code/build/output && cp /tmp/ArborOS-0.5.iso /workspace/Code/build/output/ArborOS-0.5.iso && echo && echo "BUILD COMPLETE!" && ls -lh /workspace/Code/build/output/ArborOS-0.5.iso'

if (Test-Path "Code\build\output\ArborOS-0.5.iso") {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "SUCCESS! ISO Ready" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "Location: Code\build\output\ArborOS-0.5.iso"
    $size = (Get-Item "Code\build\output\ArborOS-0.5.iso").Length
    Write-Host "Size: $size bytes"
    Write-Host ""
    Write-Host "Test in VirtualBox:" -ForegroundColor Yellow
    Write-Host "1. Create new VM"
    Write-Host "2. Load ArborOS-0.5.iso"
    Write-Host "3. Boot - LXQt desktop should appear"
    Write-Host "========================================" -ForegroundColor Green
} else {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Red
    Write-Host "BUILD FAILED" -ForegroundColor Red
    Write-Host "Check errors above" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Red
}

Write-Host ""
Pause
