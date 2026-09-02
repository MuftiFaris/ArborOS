# ArborOS Phase 5 Build - Windows PowerShell
# Right-click > Run with PowerShell

Write-Host "========================================" -ForegroundColor Green
Write-Host "ArborOS Phase 5 - Desktop Build" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Requirements:" -ForegroundColor Yellow
Write-Host "- Docker Desktop running"
Write-Host "- 10GB free disk space"
Write-Host "- Build time: 30-45 minutes"
Write-Host ""

$response = Read-Host "Press ENTER to start build (or Ctrl+C to cancel)"

Set-Location $PSScriptRoot
Set-Location ..\..

Write-Host ""
Write-Host "[BUILD] Starting Docker build..." -ForegroundColor Cyan
Write-Host "[BUILD] Preparing build script..." -ForegroundColor Yellow
Write-Host ""

# Create bash build script with proper line endings
$bashScript = @'
#!/bin/bash
set -e
echo '[1/9] Installing build tools...'
dnf install -y -q genisoimage syslinux squashfs-tools dracut-live e2fsprogs ca-certificates
cd /tmp
mkdir -p build/work/rootfs build/iso/boot build/iso/isolinux build/iso/LiveOS
echo '[2/9] Bootstrapping Fedora (300+ packages)...'
dnf --installroot=/tmp/build/work/rootfs --releasever=39 --setopt=install_weak_deps=False --setopt=cachedir=/tmp/dnf-cache --nodocs -y -q groupinstall 'Minimal Install'
echo '[3/9] Installing desktop packages (400+ packages)...'
dnf --installroot=/tmp/build/work/rootfs --releasever=39 --setopt=cachedir=/tmp/dnf-cache -y -q install kernel systemd NetworkManager openssh-server sudo nano bash-completion dracut-live dracut-network linux-firmware mesa-dri-drivers pipewire wireplumber alsa-utils lxqt-panel lxqt-session lxqt-config openbox pcmanfm-qt qterminal featherpad lightdm lightdm-gtk lightdm-gtk-greeter-settings network-manager-applet breeze-icon-theme xorg-x11-server-Xorg xorg-x11-xinit htop wget curl git
echo '[4/9] Configuring system...'
mount -t proc /proc /tmp/build/work/rootfs/proc
mount -t sysfs /sys /tmp/build/work/rootfs/sys
mount --bind /dev /tmp/build/work/rootfs/dev
mount --bind /dev/pts /tmp/build/work/rootfs/dev/pts
echo arbor-os > /tmp/build/work/rootfs/etc/hostname
chroot /tmp/build/work/rootfs useradd -m -G wheel -s /bin/bash arbor 2>/dev/null || true
chroot /tmp/build/work/rootfs bash -c 'echo arbor:arbor | chpasswd'
echo '%wheel ALL=(ALL) ALL' > /tmp/build/work/rootfs/etc/sudoers.d/wheel
chmod 0440 /tmp/build/work/rootfs/etc/sudoers.d/wheel
chroot /tmp/build/work/rootfs systemctl enable NetworkManager lightdm 2>/dev/null || true
chroot /tmp/build/work/rootfs systemctl set-default graphical.target 2>/dev/null || true
mkdir -p /tmp/build/work/rootfs/etc/lightdm
printf '[Seat:*]\nautologin-user=arbor\nautologin-user-timeout=0\n' > /tmp/build/work/rootfs/etc/lightdm/lightdm.conf
if [ -f /workspace/Code/system/hardware-detection.sh ]; then
    cp /workspace/Code/system/hardware-detection.sh /tmp/build/work/rootfs/usr/local/bin/arbor-hwinfo
    chmod +x /tmp/build/work/rootfs/usr/local/bin/arbor-hwinfo
    echo 'Hardware detection script installed'
else
    echo 'Warning: hardware-detection.sh not found'
fi
echo '[5/9] Building initramfs...'
KVER=$(ls /tmp/build/work/rootfs/lib/modules | head -n1)
chroot /tmp/build/work/rootfs dracut --force --no-hostonly --add 'dmsquash-live livenet' --omit plymouth /boot/initramfs-live.img $KVER 2>&1 | grep -v 'dracut: Executing' || true
umount -l /tmp/build/work/rootfs/proc || true
umount -l /tmp/build/work/rootfs/sys || true
umount -l /tmp/build/work/rootfs/dev/pts || true
umount -l /tmp/build/work/rootfs/dev || true
echo '[6/9] Copying kernel...'
mkdir -p /tmp/build/iso/boot
KERNEL_FILE=$(find /tmp/build/work/rootfs/boot -name 'vmlinuz-*' -type f | head -n1)
if [ -z "$KERNEL_FILE" ]; then
    echo 'ERROR: Kernel not found!'
    ls -la /tmp/build/work/rootfs/boot/
    exit 1
fi
cp "$KERNEL_FILE" /tmp/build/iso/boot/vmlinuz
cp /tmp/build/work/rootfs/boot/initramfs-live.img /tmp/build/iso/boot/initramfs.img
echo "Kernel: $(basename $KERNEL_FILE)"
echo '[7/9] Creating squashfs...'
mksquashfs /tmp/build/work/rootfs /tmp/build/iso/LiveOS/squashfs.img -comp xz -b 1M -no-progress
echo '[8/9] Configuring bootloader...'
mkdir -p /tmp/build/iso/isolinux
printf 'DEFAULT arbor\nTIMEOUT 50\nLABEL arbor\n    KERNEL /boot/vmlinuz\n    APPEND initrd=/boot/initramfs.img root=live:CDLABEL=ArborOS rd.live.image selinux=0 quiet\n' > /tmp/build/iso/isolinux/isolinux.cfg
cp /usr/share/syslinux/isolinux.bin /tmp/build/iso/isolinux/
cp /usr/share/syslinux/ldlinux.c32 /tmp/build/iso/isolinux/
echo '[9/9] Building ISO...'
mkisofs -quiet -o /tmp/ArborOS-0.5.iso -b isolinux/isolinux.bin -c isolinux/boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table -J -R -V ArborOS /tmp/build/iso
isohybrid /tmp/ArborOS-0.5.iso 2>/dev/null || true
mkdir -p /workspace/Code/build/output
cp /tmp/ArborOS-0.5.iso /workspace/Code/build/output/
echo
echo 'BUILD COMPLETE!'
ls -lh /workspace/Code/build/output/ArborOS-0.5.iso
'@

# Write script with Unix LF line endings
$tempScript = "$PSScriptRoot\temp-build.sh"
$utf8NoBom = New-Object System.Text.UTF8Encoding $false
[System.IO.File]::WriteAllLines($tempScript, $bashScript, $utf8NoBom)

Write-Host "[BUILD] Running Docker container..." -ForegroundColor Cyan

docker run --rm -it --privileged -v "${PWD}:/workspace" fedora:39 bash /workspace/Code/build/temp-build.sh

# Cleanup
Remove-Item $tempScript -ErrorAction SilentlyContinue

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "Checking build result..." -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    
    if (Test-Path "Code\build\output\ArborOS-0.5.iso") {
        $iso = Get-Item "Code\build\output\ArborOS-0.5.iso"
        $sizeMB = [math]::Round($iso.Length / 1MB, 2)
        
        Write-Host ""
        Write-Host "SUCCESS! ISO Ready" -ForegroundColor Green
        Write-Host "Location: $($iso.FullName)" -ForegroundColor Cyan
        Write-Host "Size: $sizeMB MB" -ForegroundColor Cyan
        Write-Host "Created: $($iso.LastWriteTime)" -ForegroundColor Cyan
        Write-Host ""
        Write-Host "Test in VirtualBox:" -ForegroundColor Yellow
        Write-Host "1. Create new VM (Linux/Fedora 64-bit)" -ForegroundColor White
        Write-Host "2. RAM: 2048MB minimum" -ForegroundColor White
        Write-Host "3. Load ArborOS-0.5.iso as optical drive" -ForegroundColor White
        Write-Host "4. Boot - LXQt desktop should appear" -ForegroundColor White
        Write-Host "5. Login: arbor / arbor" -ForegroundColor White
        Write-Host ""
    } else {
        Write-Host ""
        Write-Host "WARNING: ISO file not found" -ForegroundColor Red
        Write-Host "Build may have failed silently" -ForegroundColor Red
        Write-Host "Check Docker Desktop logs" -ForegroundColor Yellow
        Write-Host ""
    }
} else {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Red
    Write-Host "BUILD FAILED" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Red
    Write-Host "Exit code: $LASTEXITCODE" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Common issues:" -ForegroundColor Yellow
    Write-Host "- Docker Desktop not running" -ForegroundColor White
    Write-Host "- Not enough disk space (need 10GB)" -ForegroundColor White
    Write-Host "- Internet connection required" -ForegroundColor White
    Write-Host ""
}

Write-Host ""
Read-Host "Press ENTER to exit"
