#!/bin/bash
# Cleanup Script for Phase 2 Temporary Files
# Run this in Ubuntu VM after successful build

echo "=== ArborOS Phase 2 Cleanup ==="
echo ""

# Clean Ubuntu VM temporary files
echo "[1/3] Cleaning Ubuntu VM temporary directories..."
sudo rm -rf ~/ArborOS-Code/livecd-temp
sudo rm -rf ~/ArborOS-Code/build/livecd-work  
sudo rm -rf ~/liveos-proper
sudo rm -rf /tmp/liveos-*
sudo rm -rf /tmp/rootfs-*

echo "[2/3] Cleaning old build artifacts..."
sudo rm -rf ~/ArborOS-Code/build/output/ArborOS-REAL-FINAL.iso
sudo rm -rf ~/ArborOS-Code/build/output/ArborOS-CORRECT.iso

echo "[3/3] Checking disk space..."
df -h ~ | grep /dev/sda2

echo ""
echo "=== Cleanup Complete ==="
echo ""
echo "Windows files to manually delete:"
echo "  - ArborOS-CORRECT.iso (old)"
echo "  - ArborOS-FINAL.iso (old)"
echo "  - ArborOS-WORKING.iso (old)"
echo "  - ArborOS-0.1.iso (old)"
echo "  - rootfs.img (temp)"
echo "  - mksquashfs-bin (temp)"
echo "  - LiveOS-final/ (folder)"
echo "  - livecd-temp/ (folder)"
echo ""
echo "Keep these files:"
echo "  ✅ ArborOS-SELINUX-OFF.iso (working build)"
echo "  ✅ Code/ (source)"
echo "  ✅ Documentation/ (docs)"
echo ""
