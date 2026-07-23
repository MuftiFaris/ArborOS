#!/bin/bash
# Arbor OS Boot Test
# Tests ArborOS-0.1.iso in QEMU

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
ISO_FILE="${PROJECT_ROOT}/build/output/ArborOS-0.1.iso"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

log() {
    echo -e "${GREEN}[TEST]${NC} $*"
}

error() {
    echo -e "${RED}[ERROR]${NC} $*" >&2
    exit 1
}

# Check if ISO exists
if [[ ! -f "$ISO_FILE" ]]; then
    error "ISO not found: $ISO_FILE"
    echo "Run ./build/create_iso.sh first"
    exit 1
fi

# Check for QEMU
if ! command -v qemu-system-x86_64 &> /dev/null; then
    error "qemu-system-x86_64 not found. Install QEMU."
fi

log "Testing Arbor OS in QEMU"
log "ISO: $ISO_FILE"
log ""
log "Press Ctrl+Alt+G to release mouse"
log "Press Ctrl+Alt+2 for QEMU monitor"
log "Type 'quit' in monitor to exit"
log ""

# QEMU settings
MEMORY="2G"
CPUS="2"
DISK_SIZE="20G"
DISK_FILE="${PROJECT_ROOT}/tests/test-disk.qcow2"

# Create test disk if not exists
if [[ ! -f "$DISK_FILE" ]]; then
    log "Creating test disk..."
    qemu-img create -f qcow2 "$DISK_FILE" "$DISK_SIZE"
fi

# Boot QEMU
log "Booting QEMU..."
qemu-system-x86_64 \
    -enable-kvm \
    -m "$MEMORY" \
    -smp "$CPUS" \
    -cdrom "$ISO_FILE" \
    -boot d \
    -drive file="$DISK_FILE",format=qcow2 \
    -net nic,model=virtio \
    -net user \
    -vga std \
    -display sdl

log "QEMU exited"
