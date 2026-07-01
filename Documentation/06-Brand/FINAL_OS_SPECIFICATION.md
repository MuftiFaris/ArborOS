# ARBOR OS — FINAL UNIFIED SPECIFICATION
### Official Engineering Blueprint · Version 1.0

**Document status:** Canonical  
**Supersedes:** `os-architecture-blueprint.md`, `ux-design.md`, `arbor-os-security-architecture.md`, `arbor-os-developer-ecosystem.md`, `arbor-os-performance-engineering.md`  
**Maintained by:** Core OS Team  
**Last consolidated:** 2026-07

> **How to read this document.** This specification is the single source of truth for Arbor OS design decisions. Where prior documents conflict, this document's ruling takes precedence. All section cross-references are self-contained within this document.

---

## TABLE OF CONTENTS

1. [Product Vision and Philosophy](#1-product-vision-and-philosophy)
2. [Design Conflicts and Resolutions](#2-design-conflicts-and-resolutions)
3. [Final System Architecture](#3-final-system-architecture)
4. [User Experience Standard](#4-user-experience-standard)
5. [Privacy and User Control](#5-privacy-and-user-control)
6. [Performance Targets and Engineering](#6-performance-targets-and-engineering)
7. [Security Architecture](#7-security-architecture)
8. [Developer Ecosystem](#8-developer-ecosystem)
9. [Development Roadmap](#9-development-roadmap)
10. [Summary Reference Tables](#10-summary-reference-tables)

---

## 1. PRODUCT VISION AND PHILOSOPHY

### 1.1 Mission Statement

Arbor OS is a privacy-first, performance-focused Linux desktop operating system built on a mature Fedora Atomic foundation. Its thesis: **you don't out-engineer the kernel, you out-engineer the integration.** Linux's technical foundation — cgroups, namespaces, Wayland, PipeWire, systemd, Btrfs — is already at parity with or ahead of proprietary operating systems. What has historically been missing is a single opinionated team making tens of thousands of small consistency decisions rather than leaving them to fragmented upstream defaults.

**Tagline:** *"Change anything. Break nothing."*

**Identity metaphor:** Arbor OS is a tree, not a cathedral — a strong, immutable trunk with flexible, customizable branches that can be pruned or grown without threatening the trunk's integrity.

### 1.2 Target Users (Priority Order)

| Priority | Persona | Core Need |
|---|---|---|
| 1 | **Privacy-conscious professionals** — journalists, lawyers, researchers, security-aware knowledge workers | Leave macOS/Windows without sacrificing polish or gaining telemetry |
| 2 | **Developers** | Unix environment without driver hell, theming inconsistency, or environment contamination |
| 3 | **Prosumers/creatives** | Stable, color-managed pipeline for video/photo/audio work |
| 4 | **Gamers** | SteamOS-like experience on a general-purpose OS |
| 5 | **Switchers from Windows 11** | Something that "just works" without a terminal, replacing Windows over telemetry/ads/forced accounts |

### 1.3 Competitive Positioning

| Axis | macOS | Windows 11 | Arbor OS |
|---|---|---|---|
| Telemetry | Opt-out, opaque | Mandatory core telemetry | Zero by default, source-auditable |
| Hardware lock-in | Apple Silicon only | Broad but bloated | Broad, curated HCL |
| Package trust | Notarization (Apple-controlled) | Store + arbitrary EXE | Reproducible builds + signed repos + tiered trust |
| Customization | Very limited | Limited | Full, without breaking updates |
| Cost | Hardware tax | License + embedded ads | Free, optional paid support |
| Update reliability | Good | Historically rocky | Atomic, always-rollback-able |
| Privacy defaults | Moderate | Poor | Best-in-class for a desktop OS |

### 1.4 Core Design Principles

1. **Immutable core, mutable edges** — the base OS is read-only and image-based; everything user-facing is layered or sandboxed.
2. **Privacy is a default, not a setting** — no dark patterns, no opt-out telemetry, no cloud account requirement ever.
3. **One good default beats ten mediocre options** — curate, don't ship five text editors "for choice."
4. **Every destructive action is reversible** — updates, package installs, and config changes are all snapshot-backed.
5. **Consistency over choice paralysis** — a single design system spanning system UI and all first-party apps.
6. **Open, but opinionated** — fully open-source and upstream-first in contributions, but Arbor makes the integration calls macOS makes internally.
7. **Security that enables, not obstructs** — security controls are on by default and designed to work with user workflows, not fight them.

---

## 2. DESIGN CONFLICTS AND RESOLUTIONS

The following conflicts were identified across the five source documents. Each conflict is resolved here with a final ruling that all implementation work must follow.

---

### Conflict 1: Firejail Mentioned Alongside Bubblewrap

**Problem:** The OS Architecture Blueprint's system architecture diagram lists "Firejail/Bubblewrap (Flatpak sandbox)" as if both tools coexist. The Security Architecture document only specifies Bubblewrap. Firejail and Bubblewrap overlap in purpose and their simultaneous use creates an inconsistent security model and policy surface.

**Solution:** Bubblewrap is already the correct tool — it is the sandboxing backend for Flatpak and integrates natively with XDG Desktop Portals. Firejail is a separate, standalone sandboxing tool designed for non-Flatpak native binaries and conflicts with the Flatpak trust model. Adding Firejail creates a second, independent policy layer with no user-visible interface and potential policy gaps at the seam.

**Final decision:** **Remove all references to Firejail.** The sandboxing stack is: Bubblewrap (Flatpak app isolation) + fapolicyd (host execution allowlisting) + SELinux (system daemon confinement). No Firejail. The architecture diagram in §3 reflects this.

---

### Conflict 2: Phase Numbering Inconsistency Across Documents

**Problem:** The OS Architecture Blueprint defines five phases. The Security Architecture references "Phase 4" for SELinux hardening. The Developer Ecosystem document has its own five-phase alignment table with different milestone definitions and overlapping date ranges that diverge from the blueprint.

**Solution:** The OS Architecture Blueprint's five-phase model is the canonical project roadmap. All other documents' phase references are subordinate to it and were written in alignment with it — the differences are phrasing variations, not true conflicts.

**Final decision:** **The five-phase roadmap in §9 of this document is the single canonical reference.** The Security Architecture's "Phase 4" reference aligns correctly. The Developer Ecosystem's milestone table has been reconciled and integrated into §9 without loss of content.

---

### Conflict 3: Plymouth Disabled vs. Boot Sound in UX Spec

**Problem:** The Performance Engineering document explicitly disables Plymouth (the boot splash system) because its startup sequence adds measurable latency to the boot time target. The UX Design document specifies a boot sound — a "low 3-note ambient acoustic swell" — as part of the sound identity. Plymouth is the most common mechanism for playing boot-time audio on Linux.

**Solution:** The Plymouth disable is correct and must be maintained for the < 8 second boot target. Plymouth's overhead (framebuffer init, asset loading, audio daemon startup) is incompatible with the boot time CI gate. However, boot audio does not require Plymouth — it can be triggered via a systemd oneshot unit that fires early in the user session startup, immediately after login screen readiness, using PipeWire with a short startup path. This is not a "boot sound" in the traditional firmware/splash sense but an "arrival sound" at session start, which is the more common implementation on modern OSes (macOS's startup sound plays at login, not at firmware POST).

**Final decision:** **Plymouth remains disabled.** The boot sound is re-specified as a **session-start sound** — a short audio cue played within the first 500 ms of the user session becoming active, implemented as a `systemd --user` oneshot unit. This preserves the UX intent (a sonic identity marker) without the boot time penalty. The UX spec in §4.2 is updated to reflect "session-start sound" rather than "boot sound."

---

### Conflict 4: Vague "Debian-Style QA" Creates Ambiguity About Build Dependencies

**Problem:** The OS Architecture Blueprint states that Arbor OS uses "a Debian-style multi-tier repo QA process layered on top" of the Fedora Atomic base. This has been misread by some contributors as implying an actual Debian or apt dependency in the build system.

**Solution:** The reference to Debian is purely philosophical — it refers to Debian's practice of maintaining `unstable → testing → stable` promotion tiers with human review gates between them. There is no Debian technology in the Arbor OS build system. All tooling is Fedora/RPM/OSTree-based.

**Final decision:** **"Debian-style" language is replaced throughout with explicit tier descriptions.** Arbor's repo QA model is: **Development → Canary → Beta → Stable**, implemented entirely within the OSTree/bootc/rpm pipeline. See §3.9 for the full tier model.

---

### Conflict 5: Gaming Performance Target vs. Actual Benchmark Data

**Problem:** The OS Architecture Blueprint states a gaming performance goal of "<3% gaming latency overhead vs. Windows." The Performance Engineering document's actual benchmark data shows:

- Most games: −3.8% to −5.6% vs. Windows 11
- CS2 (anti-cheat overhead via Proton): −10.1% vs. Windows 11

The "<3%" target is not achievable for all titles and would fail CI gates if enforced literally.

**Solution:** The blanket "<3%" figure was aspirational and written before the benchmark data existed. The benchmarks represent real-world measurements on reference hardware with the full security stack enabled (SELinux enforcing, Flatpak sandbox, etc.). Anti-cheat software running via Proton introduces inherent overhead that is not an Arbor OS deficit — it is a Proton/Wine compatibility layer cost that affects all Linux distributions equally.

**Final decision:** **The gaming performance target is revised to two tiers:**
- **General games (no kernel-level anti-cheat):** < 6% average FPS overhead vs. Windows 11, measured across the benchmark suite.
- **Anti-cheat-dependent titles (Proton compatibility layer):** < 12% average FPS overhead, explicitly noted as a Proton/Wine inherent cost rather than an Arbor-specific deficit.

The CI gate uses the benchmark suite average, not individual title maximums.

---

### Conflict 6: ZFS Scope Creep in Desktop Specification

**Problem:** The OS Architecture Blueprint mentions ZFS as "optional for power users/servers" on the desktop edition, with a note about DKMS friction with atomic kernel updates. The Performance Engineering document has no ZFS content and focuses exclusively on Btrfs. Including ZFS in the desktop spec adds DKMS complexity — exactly what the architecture principles say to minimize — and the LUKS + per-user home encryption story is less mature with ZFS.

**Solution:** ZFS's strengths (send/receive replication, richer data integrity primitives) are most relevant for server and high-availability backup workflows, not the desktop persona. The DKMS requirement creates a genuine incompatibility risk with atomic kernel updates and violates the "DKMS is last resort" principle stated in the architecture itself.

**Final decision:** **ZFS is removed from the desktop edition specification.** ZFS support will be specified in a future "Arbor Server Edition" document (out of scope for this specification). Desktop edition filesystems: Btrfs for root and home, ext4 for the ESP, no ZFS. Users who require ZFS can use Distrobox containers targeting a ZFS-capable image for storage-only workflows.

---

### Conflict 7: GNOME Shell "Fork" vs. Extension-Based Approach

**Problem:** The OS Architecture Blueprint describes "Arbor Shell" as a "fork/heavily-themed layer on GNOME Shell" and lists "Custom shell fork drifting from upstream GNOME" as a primary risk in the Phase 2 roadmap, with "extension-based approach wherever possible" as the mitigation. The Developer Ecosystem document describes the Arbor Extension API as a stable, versioned API built on top of GNOME Shell. These descriptions are technically in tension: a true fork and an extension-based approach are meaningfully different implementation strategies.

**Solution:** A hard fork of gnome-shell would require maintaining divergent C code, rebasing security patches from upstream manually, and accepting a permanent maintenance burden that scales with team size. The extension-based approach, enhanced with the Arbor Extension API v1 (which wraps GNOME Shell internals behind a stable, documented contract), achieves the same UX goals with a fraction of the maintenance cost. The "fork" language in the architecture doc was imprecise — the intent was always to avoid forking Shell internals.

**Final decision:** **"Arbor Shell" is defined as GNOME Shell plus: (1) the Arbor libadwaita theme applied at the GTK/CSS level, (2) a set of first-party Arbor extensions using the Extension API v1, and (3) compositor configuration via GNOME Settings overrides and dconf.** No fork of gnome-shell, mutter, or libadwaita C source is made or maintained. The Arbor Extension API v1 is the stable boundary between Arbor's customizations and upstream GNOME internals, isolating Arbor from upstream API churn.

---

### Conflict 8: Distrobox vs. Toolbx — Functional Redundancy

**Problem:** Both the OS Architecture Blueprint and the Developer Ecosystem document mention "Distrobox/Toolbx" as if they are interchangeable or complementary. Both tools serve the same primary use case: containerized CLI development environments on an immutable host. Shipping both as co-equal, documented options creates confusion about which to use and doubles the surface area to test and support.

**Solution:** Distrobox has broader container image support (any OCI-compatible image, not just fedora-toolbox images), tighter GUI integration (exported apps appear in the desktop launcher), and a more active upstream maintenance community as of the specification date. Toolbx is a Podman-native tool with a narrower focus. For the primary developer persona, Distrobox covers the full use case.

**Final decision:** **Distrobox is the primary, supported, documented developer container tool.** It is pre-installed in all editions and has first-class UX integration (launcher export, file manager sidebar). Toolbx is available as an installable Flatpak for users who prefer it and remains compatible with Podman's rootless container stack, but does not receive dedicated UX integration or documentation. All developer documentation uses Distrobox in examples.

---

### Conflict 9: AppArmor + SELinux Dual-MAC Scope Needs Explicit Boundary

**Problem:** Both the Architecture Blueprint and the Security Architecture mention both SELinux and AppArmor without fully specifying where one ends and the other begins. This creates an open question about which tool governs which subjects, and risks contributors choosing the "easier" tool (AppArmor) for new policy work when SELinux policy should be used.

**Solution:** The Security Architecture's intent is clear but needs to be stated as a hard rule: SELinux is the primary MAC system governing all system daemons and kernel objects. AppArmor is present but used only for third-party integrations where upstream software ships AppArmor profiles that Arbor does not want to fork into SELinux policy.

**Final decision:** The MAC boundary is now a hard rule: **SELinux (targeted policy, enforcing) governs all Arbor-maintained system daemons and kernel interfaces. AppArmor is used only for the following explicitly listed third-party cases: (1) Chromium/Chrome's GPU sandbox profile, (2) CUPS print daemon (upstream ships AppArmor profiles), (3) select scan daemons (SANE). No new AppArmor policy is authored for Arbor-maintained components. All new policy work for Arbor components uses SELinux.** The two MAC systems do not overlap subjects; their scopes are formally documented.

---

### Conflict 10: Idle RAM Target and Homebrew's User-Space Footprint

**Problem:** The Performance Engineering document targets ≤ 1.2 GB idle RAM. The Developer Ecosystem document prescribes Homebrew as the CLI/dev tooling manager, pre-configured out-of-the-box. Homebrew's linuxbrew daemon and background services can add RAM in developer-active sessions.

**Solution:** Homebrew installs to `~/.linuxbrew` (user-space) and has no system daemon. Its overhead is per-user-session, not system-wide idle. The idle RAM target is measured at a freshly-logged-in standard (non-Developer Mode) session with zero user apps open. Homebrew is not active at idle because it has no persistent daemon. In Developer Mode sessions, a separate benchmark tracks RAM, and the target for Developer Mode idle is ≤ 1.6 GB (accounting for language server processes, mise shims, and development daemons that activate on shell startup).

**Final decision:** The ≤ 1.2 GB idle RAM target applies to **Standard Mode** (no Developer Mode toggle). A separate **Developer Mode idle RAM target of ≤ 1.6 GB** is established. Both are CI-gated. The Performance Engineering targets table in §6 is updated to reflect this split.

---

## 3. FINAL SYSTEM ARCHITECTURE

### 3.1 Layer Architecture

```
┌──────────────────────────────────────────────────────────────────┐
│  APPLICATION LAYER                                                │
│  Flatpak Sandboxed Apps │ Distrobox Dev Containers │ Native RPM  │
├──────────────────────────────────────────────────────────────────┤
│  ARBOR SHELL (GNOME Shell + Arbor Extension API v1 + Theme)       │
│  Compositor (Mutter/Wayland) │ XDG Portals │ Arbor Settings       │
├──────────────────────────────────────────────────────────────────┤
│  SECURITY LAYER                                                   │
│  SELinux (enforcing, targeted) │ Bubblewrap (Flatpak sandbox)     │
│  fapolicyd (execution allowlist) │ AppArmor (third-party only)    │
│  Polkit (privilege escalation) │ nftables (firewall)              │
├──────────────────────────────────────────────────────────────────┤
│  SERVICE LAYER                                                    │
│  systemd │ PipeWire + WirePlumber │ NetworkManager │ dbus-broker  │
│  systemd-homed │ systemd-oomd │ udisks2 │ power-profiles-daemon   │
├──────────────────────────────────────────────────────────────────┤
│  GRAPHICS LAYER                                                   │
│  Wayland (primary) │ XWayland (legacy compat, no X11 session)     │
│  Mesa (AMD/Intel open) │ NVIDIA proprietary + open kernel modules │
│  Vulkan-first │ VA-API hardware decode/encode                     │
├──────────────────────────────────────────────────────────────────┤
│  DRIVER / HAL LAYER                                               │
│  In-tree kernel modules │ linux-firmware bundle │ fwupd (LVFS)    │
│  DKMS: last-resort only, flagged in HCL                           │
├──────────────────────────────────────────────────────────────────┤
│  KERNEL LAYER — Linux (arbor-kernel), cgroups v2, namespaces      │
├──────────────────────────────────────────────────────────────────┤
│  HARDWARE LAYER — x86_64 (GA) │ aarch64 (Phase 4+ roadmap)        │
└──────────────────────────────────────────────────────────────────┘
```

### 3.2 Linux Foundation and Distro Base

**Selected base: Fedora Atomic (rpm-ostree / bootc image-based)**

| Candidate | Verdict | Reason |
|---|---|---|
| Arch Linux | ❌ Rejected | Rolling release incompatible with premium/stable positioning; no image-based update story |
| Ubuntu LTS | ❌ Rejected | Snap telemetry decisions conflict with privacy-first identity; brand confusion |
| Fedora Atomic | ✅ **Selected** | Best-in-class immutable tooling, SELinux maturity, fast upstream kernel cadence, Wayland/PipeWire/systemd engineering |
| Debian | ✅ QA philosophy only | Reproducibility culture informs Arbor's repo QA tier model; no Debian technology in the build system |

The Fedora Atomic base provides: RHEL-grade SELinux engineering, OSTree's field-tested atomic/rollback model (validated by Fedora Silverblue, Universal Blue, Bazzite, Aurora), and current kernel/driver support. Arbor adds: a bespoke design system, curated app verification, privacy-first defaults, and the developer ecosystem in §8.

### 3.3 Kernel Strategy

Two kernel channels ship and are switchable at runtime via Settings → Security:

| Property | `arbor-kernel` (default) | `arbor-kernel-hardened` |
|---|---|---|
| Target user | All users, gaming, creative work | Secure Workstation edition, journalists, lawyers |
| `CONFIG_PREEMPT` | Full desktop preemption | Full desktop preemption |
| KASLR | Enabled | Enabled, stronger entropy source |
| Stack protector | `-fstack-protector-strong` | `-fstack-protector-strong` |
| `/proc/kallsyms` restriction | Standard | `kernel.kptr_restrict=2` |
| `dmesg` restriction | Standard | `kernel.dmesg_restrict=1` |
| `ptrace` scope | `yama.ptrace_scope=1` (child-only) | `yama.ptrace_scope=2` (admin-only) |
| Unprivileged BPF | Enabled (needed by some sandboxing tools) | Disabled by default, per-app exception list |
| Module signing | Signed preferred | Signed enforced (`module.sig_enforce=1`) |
| Speculative execution mitigations | All enabled | All enabled, no perf-over-security tradeoffs |
| `CONFIG_LOCKDOWN_LSM` | Integrity mode | Confidentiality mode (blocks `/dev/mem`, unsigned kexec) |
| Compiler hardening | `-D_FORTIFY_SOURCE=3`, full RELRO, PIE | Same + CFI where toolchain allows |

**LTS fallback kernel** is always retained as a boot entry regardless of active channel, providing regression safety at zero cost via the atomic deployment model.

Kernel updates ship as part of atomic image updates — never live-patched into a running system. The new deployment test-boots before becoming default.

### 3.4 Init System

**systemd**, adopted aggressively:

- `systemd-homed` — per-user home encryption and portability
- `systemd-oomd` — OOM handling before kernel OOM killer fires (see §6.2)
- `systemd-boot` — default bootloader (UEFI/Secure Boot native, faster than GRUB)
- GRUB — fallback for legacy BIOS hardware only, explicitly labeled as "Secure Boot unavailable"
- `dbus-broker` — replaces reference dbus-daemon for lower overhead and better logging

### 3.5 Filesystem

| Mount | Filesystem | Notes |
|---|---|---|
| `/` (root) | **Btrfs** | OSTree-managed subvolume `@`; immutable in normal operation |
| `/home` | **Btrfs** | Subvolume `@home`; Snapper-managed rolling snapshots |
| `/var` | **Btrfs** | Subvolume `@var`; mutable system state |
| `/.snapshots` | **Btrfs** | Subvolume `@snapshots`; Snapper snapshot store |
| `/boot/efi` | **FAT32 / vfat** | ESP; bootloader compatibility |
| User homes | **LUKS2 via systemd-homed** | Per-user encrypted containers on top of Btrfs home |

**Mount options (all Btrfs volumes):** `noatime,compress=zstd:3,space_cache=v2,discard=async`

**No ZFS on desktop edition.** See Conflict 6 resolution in §2.

Filesystem-level compression (`zstd:3`) is on by default — delivers ~60% compression ratio on typical desktop files with minimal CPU overhead, reducing SSD write amplification.

### 3.6 Boot System

```
UEFI Firmware (OEM root of trust)
   ↓ Secure Boot verification
Arbor-signed shim (Microsoft 3rd-party UEFI CA signed)
   ↓ Secure Boot verification
Arbor-signed systemd-boot
   ↓ Secure Boot verification + PCR measurement
Signed kernel + initramfs (signed in CI pipeline, never on-device)
   ↓ PCR values sealed into
TPM 2.0 (LUKS2 key release on matching boot chain)
   ↓
User session
```

**Boot menu entries (auto-populated by bootc):**
- Current deployment (default, 5-second timeout)
- Previous N deployments (rollback)
- Recovery environment (dedicated minimal OSTree deployment)

**Plymouth: disabled.** Boot screen is a clean black display until the login screen is ready. See Conflict 3 resolution in §2 for session-start sound implementation.

**Target cold boot:** < 8 seconds to login screen on NVMe hardware. CI-gated.

### 3.7 Display and Audio Systems

**Display:** Wayland-only. No X11 desktop session ships by default. XWayland is present for legacy application compatibility (wine, older native Linux apps) but is not a first-class session option. This is a 2026-appropriate default.

**Compositor:** Mutter (GNOME's compositor), themed and extended via Arbor Shell (see §4.1 and Conflict 7 resolution).

**NVIDIA:** Highest engineering investment area.
- Auto-detected proprietary driver installation at OS install time via curated repo — no manual DKMS configuration.
- Open kernel module path for Turing+ GPUs where supported.
- Wayland + NVIDIA explicit-sync support tracked closely with upstream Mutter and NVIDIA driver releases.
- Fallback: nouveau/open modules if proprietary install fails; never bricks boot.

**Audio:** PipeWire + WirePlumber. Replaces PulseAudio and JACK. Low-latency pro-audio profile available as a Settings toggle (not a separate OS mode). No configuration files needed for common setups.

### 3.8 Networking

- **NetworkManager** as the control layer.
- **Encrypted DNS (DoH/DoT) by default** — never silently falls back to plaintext; user sees "DNS privacy degraded" state if encrypted resolution fails.
- **MAC address randomization by default** on Wi-Fi — closes common physical-location-tracking vector.
- **WireGuard VPN** built into Settings, no third-party client required.
- **nftables** firewall backend, default-deny inbound, per-app outbound GUI.
- **WPA3** verified per-chipset in the HCL; WPA2 fallback is flagged (never silently accepted).

### 3.9 Update System

**Update delivery tier model** (replacing "Debian-style QA" language — see Conflict 4 resolution):

```
Development  →  Canary  →  Beta  →  Stable
(team only)    (opt-in)   (opt-in)  (default)
```

Each tier is an OSTree ref. Users opt into non-Stable tiers via Settings → System → Update Channel. Promotion between tiers is gated by a combination of automated test results and human review.

**Atomic updates — how they work:**
1. New OS image built centrally in CI as a signed OCI/OSTree artifact.
2. Signature verified on the client before the new deployment is created.
3. New deployment is created alongside the current one (not replacing it in-place).
4. System offers to reboot into the new deployment.
5. On successful boot, the new deployment becomes the default.
6. On boot failure (N failures via `systemd-boot` boot counting), automatic fallback to the previous deployment.
7. Running system is **never mutated in place** — half-updated states are structurally impossible.

**Snapshot schedule (automatic, Btrfs):**
- Before every OS image update.
- Before every `rpm-ostree` layering operation.
- Hourly/daily/weekly rolling snapshots of `/home` (configurable retention in Settings).

**Rollback:** Instant pointer swap (OSTree commit + Btrfs subvolume). Not a restore-from-backup operation. Single-digit seconds.

**Firmware updates:** `fwupd`/LVFS integration — BIOS, SSD firmware, and peripheral firmware update through the same update UI as OS updates.

### 3.10 Hardware Support (Hardware Compatibility List)

| Category | Support Level | Notes |
|---|---|---|
| **Intel (last 6 years)** | Verified | Mesa Xe/Arc, `thermald`/`intel_pstate`, verified power management |
| **AMD (last 6 years)** | Verified | `amdgpu` open driver, excellent mainline upstreaming |
| **NVIDIA** | Verified (with proprietary driver) | Auto-install at setup; open module for Turing+; Wayland explicit-sync tracked |
| **Laptops — suspend/resume** | Verified per model in HCL | Both s2idle and S3 tested |
| **Laptops — hybrid graphics** | Supported | `envycontrol`-style toggle in Settings |
| **Fingerprint** | Supported | `fprintd` + PAM for login/sudo/Polkit |
| **Touchscreen/convertible** | Supported | GNOME Wayland touch gestures; OSK for tablet mode |
| **Bluetooth** | Supported | BlueZ; unified Settings |
| **Wi-Fi** | Mainline preferred, DKMS flagged | WPA3 verified per-chipset |
| **Multi-monitor** | Supported | Mixed refresh rate Wayland (active upstream priority) |
| **aarch64** | Roadmap (Phase 4+) | Build system multi-arch from day one |

Hardware with DKMS-only driver support is marked in the HCL as "Limited — may break on kernel updates."

### 3.11 Application Ecosystem

**Primary app format: Flatpak** — cross-distro portable, sandbox-integrated, delta updates via OSTree.

**Arbor Store** aggregates:
1. **Arbor Verified** — reviewed by Arbor security team, reproducible build confirmed, manifest audited.
2. **Flathub Verified** — upstream Flathub developer-identity verification.
3. **Community** — sandboxed, unreviewed; opt-in via "Enable unverified apps" toggle; clearly labeled.

**Secondary tooling:**
- `rpm-ostree` layering — system-level extensions (drivers, kernel modules). Advanced, rollback-safe, clearly labeled in UX.
- **Homebrew** — CLI/dev tooling. User-space (`~/.linuxbrew`), no system daemon, no system integration required.
- **Distrobox** — containerized dev environments. Primary developer workflow tool (see §8, Conflict 8 resolution).

---

## 4. USER EXPERIENCE STANDARD

### 4.1 Arbor Shell

"Arbor Shell" is implemented as GNOME Shell plus the Arbor Extension API v1, the Arbor libadwaita theme, and dconf/GSettings configuration. No hard fork of gnome-shell or mutter C source is maintained. See Conflict 7 resolution in §2.

### 4.2 Visual Language

**Brand mark:** A minimalist, continuous-line geometric tree leaf that doubles as an abstract delta symbol (Δ), representing evolution and branching snapshots.

**Typography:**

| Level | Font | Size | Weight |
|---|---|---|---|
| Display | Inter Display | 36px | Semi-Bold (600) |
| Heading 1 | Inter Display | 24px | Semi-Bold (600) |
| Heading 2 | Inter | 18px | Medium (500) |
| Body / UI | Inter | 14px | Regular / Medium |
| Caption | Inter | 12px | Regular (400) |
| Code / Terminal | JetBrains Mono | 13px | Regular (400) |

**Color palette — Dark Theme (default):**

| Token | Value | Usage |
|---|---|---|
| `Surface Base` | `#0F1215` | Window background |
| `Surface Elev 1` | `#161B22` | Panel background |
| `Surface Elev 2` | `#21262D` | Card / dialog background |
| `Accent Primary` | `#2DD4BF` | Arbor Teal — interactive elements |
| `Accent Hover` | `#5EEAD4` | Hover state |
| `Text Primary` | `#F0F6FC` | Body text |
| `Text Muted` | `#8B949E` | Labels, captions |
| `Border Neutral` | `rgba(255,255,255,0.08)` | Dividers |
| `Semantic Success` | `#3FB950` | Positive state |
| `Semantic Warning` | `#F0883E` | Caution state |
| `Semantic Danger` | `#F85149` | Destructive / error state |

**Color palette — Light Theme:**

| Token | Value |
|---|---|
| `Surface Base` | `#F6F8FA` |
| `Surface Elev 1` | `#FFFFFF` |
| `Accent Primary` | `#0D9488` |
| `Text Primary` | `#1F2328` |
| `Text Muted` | `#67707E` |

**Icon system:**
- System icons: Line-based, 2px stroke, 24×24 px grid, rounded caps.
- App icons: Squircle silhouette (r = 22.5%), soft 3D lighting, 0px/8px/16px drop shadow at 12% opacity.

**Session-start sound:** A low 3-note ambient acoustic swell (E-Major chord) played via a `systemd --user` oneshot unit within 500ms of session readiness. Plymouth remains disabled. (See Conflict 3 resolution.)

**Default wallpaper:** "Conifer Dawn" — generative organic gradient art with time-of-day variants (Dawn / Noon / Sunset / Midnight). Ships alongside a Minimal Series of solid tactile textures.

### 4.3 Desktop Layout

```
┌─────────────────────────────────────────────────────────────────┐
│ [Leaf] Applications    09:41 AM Mon    [Wifi][Vol][Bat][Privacy] │  ← Top Panel (36px)
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│                         WORKSPACE                               │
│                                                                 │
│          ┌─────────────────────────────────────┐               │
│          │  [Files][Terminal][Browser][Store]  │   ← Smart Dock │
│          └─────────────────────────────────────┘               │
└─────────────────────────────────────────────────────────────────┘
```

**Top Panel:**
- Height: 36px. Material: frosted glass (`backdrop-filter: blur(20px)`), `Surface Elev 1` at 70% opacity.
- Left: Arbor logo/launcher (opens Overview). Active app name + document context.
- Center: Clock (click → Notification Center + Agenda).
- Right: Integrated Status Pill — network, volume, battery, **Privacy Indicator** (turns active when camera or microphone is in use, hardware-truthful via kernel device-node open check).

**Smart Dock:**
- Position: bottom floating, 12px offset.
- Behavior: auto-hide with smart edge collision, sine-curve magnification on hover.
- Running app indicators: glowing LED bar under icon.
- Far right: Trash / Snapshots stack.

**Window system:**
- Headerbar architecture (unified titlebar + toolbar).
- Window controls (Close / Minimize / Expand) configurable left or right.
- 1px glowing `Border Neutral` border; 24px ambient occlusion drop shadow.

**Arbor Snap** (window tiling): drag to edge for preview tiles — Left/Right halves, Quarter corners, Center focus (60% width). Keyboard: `Super + Arrow Keys`.

**Workspace system:**
- Horizontal workspace strip: 3-finger swipe up or `Super + Tab`.
- Dynamic workspace creation on drag-past-last.
- Per-display independent or synchronized workspace switching (Settings).

### 4.4 Interaction Patterns and Gestures

**Animation system:** All transitions use a damped spring physics curve (`stiffness: 300, damping: 30`). Motion is functional, never gratuitous.

| Transition | Duration | Curve |
|---|---|---|
| App launch from dock | 200ms scale-up | Spring |
| Window switch | 150ms depth glide | Spring |
| Overview open | 250ms workspace expand | Spring |
| Notification appear | 180ms slide-in | Spring |

**Touchpad gestures (1:1 physical tracking):**

| Gesture | Action |
|---|---|
| 3-finger swipe up | Open Overview / Workspaces |
| 3-finger swipe left/right | Switch workspaces |
| 4-finger pinch | Show All Applications grid |
| 2-finger scroll | Standard scroll (system-wide) |

### 4.5 System Settings (Arbor Control Center)

Two-pane layout: left sidebar (240px, categorized with icons and `Ctrl+K` search) + right content area (responsive multi-card layout).

**Settings categories and key panels:**

| Category | Panel | Key Controls |
|---|---|---|
| **System** | Network | Wi-Fi list with WPA3 indicator, QR share; DNS-over-HTTPS selector; WireGuard/OpenVPN manager |
| **System** | Bluetooth | Scan grid, battery percentage for connected peripherals |
| **System** | Display | Visual multi-monitor canvas, fractional scaling slider, VRR/FreeSync/G-Sync toggle, ICC profile management |
| **System** | Sound | Per-app volume, PipeWire pro-audio toggle |
| **Security & Privacy** | Privacy Dashboard | Global permission matrix (camera, mic, location, screen, filesystem); access-history audit log |
| **Security & Privacy** | Security & Lock | TPM status, Secure Boot status, Boot Integrity indicator; LUKS key management; `arbor-kernel-hardened` toggle ("Secure Workstation Mode") |
| **Security & Privacy** | Arbor Shield Firewall | Per-app network access log; outbound block toggles per Flatpak |
| **Security & Privacy** | Updates & Rollback | Deployment status, one-click rollback, snapshot timeline |
| **Personalization** | Appearance | Theme (Dark/Light/Auto), accent color picker, wallpaper, icon style |
| **Personalization** | Applications | Default app associations; Flatpak sandbox permission inspector |
| **Accounts** | Accounts | Local `systemd-homed` accounts; SSH key manager; fingerprint enrollment; hardware key (FIDO2/YubiKey) enrollment |
| **Developer** | Developer Mode | Toggle (off by default); Developer Mode firewall rules; audit log viewer |

### 4.6 First-Boot Onboarding Experience

A frameless, centered modal flow with dark-blurred backdrop. Six steps, linear, no back-end account requirement.

```
Step 1 — Language & Region
    Grid of languages; geo-IP auto-detect if connected (privacy-preserving: local lookup).

Step 2 — Accessibility First
    Screen reader, high contrast, and text magnification toggles before anything else.

Step 3 — Account Creation
    Full name, username, password.
    Offer: fingerprint enrollment + hardware security key (YubiKey/FIDO2) setup.

Step 4 — Security & Encryption (illustrated example)
┌─────────────────────────────────────────────────────────┐
│  Disk Encryption                                        │
│  [✓] Encrypt System Drive (LUKS2)                       │
│      Uses TPM 2.0 for seamless unlock — no password     │
│      prompt on trusted hardware.                        │
│  Recovery Passphrase: [ ██████████████ ]  [Copy] [Print]│
│  Store this passphrase safely. You'll need it if your   │
│  hardware changes.                                      │
└─────────────────────────────────────────────────────────┘

Step 5 — Privacy Configuration
    Toggle: Crash reports (Default: OFF).
    Toggle: MAC address randomization (Default: ON) — explained in plain English.

Step 6 — Theme Selection
    Side-by-side live preview: Dark Slate / Light Pearl / Dynamic Auto-Switch.
    Accent color picker: Teal (default), Blue, Amber, Rose, Emerald.
```

No onboarding step requires a cloud account. Recovery passphrase is shown once and the installer refuses to silently skip it.

### 4.7 First-Party Applications

**Arbor Files (File Manager):**
- 3-column architecture: Sidebar (locations/tags) → Main view (Grid/List) → Detail inspector.
- Instant Preview on `Spacebar` — zero-latency floating overlay for images, PDFs, video, markdown, code, audio.
- Tag system: colored/custom cross-directory non-destructive organization.
- Cloud mounts: Nextcloud, Proton Drive, Google Drive, S3/Restic in sidebar with offline sync status.
- Security context menu: **"Encrypt with Vault"** (`gocryptfs` backed), **"Shred File"** (secure overwrite). No auto-`chmod +x` on downloaded files.

**Arbor Store (App Store):**
- Editorial hero carousel, curated collections ("Essential Dev Tools", "Privacy Shield Apps", "Creative Studio").
- App detail page: security/privacy badges (Arbor Verified / Flathub Verified / Community), community reviews, one-click install with progress bar, build commit hash, source repo link.
- Network Isolation badge reflects real Flatpak manifest constraint.

**Arbor Backup:** Restic under the hood; local (external drive, Btrfs send/receive incremental) and encrypted cloud (user's own Backblaze B2/S3 — no Arbor-run cloud service).

**Arbor Timeline (Snapshot Browser):** GUI to visually browse Btrfs snapshots, restore individual files, or roll back system state. Not a terminal-only feature.

**Arbor Terminal:** GPU-accelerated (wgpu/Vulkan); VTE-ng backend; Kitty terminal protocol; TOML hot-reload config; native tabs/panes; session persistence across suspend via systemd user units. Default shell: Fish. Pre-configured: Starship prompt, `atuin` history, `lazygit`.

### 4.8 Quick Settings Panel

Activated by clicking the Status Pill or `Super + N`. Glassmorphism overlay:

```
┌──────────────────────────────────────────────┐
│  QUICK SETTINGS                              │
│  [Wi-Fi On]  [Bluetooth On]  [Do Not Disturb]│
│  [VPN Connected]  [Dark Mode]  [Performance] │
│  Volume:     [=========|-------] 60%         │
│  Brightness: [=============|---] 80%         │
├──────────────────────────────────────────────┤
│  NOTIFICATIONS                               │
│  [Arbor Backup · 2m ago]                     │
│  System snapshot created successfully.       │
│  [ View Snapshot ] [ Dismiss ]               │
├──────────────────────────────────────────────┤
│  [Slack · 15m ago]                           │
│  Sarah: Check out the updated wireframes!    │
│  [ Type reply...                           ] │
└──────────────────────────────────────────────┘
```

- Inline reply fields for messages; direct action buttons for background tasks.
- Notifications grouped by app with collapsible stacks.

### 4.9 Accessibility

Accessibility is a structural requirement, not a post-launch feature:

- **Screen reader:** Orca with AT-SPI2 bindings; semantic labels on all system UI elements.
- **System magnifier:** Vector-based smooth zoom. `Super + Alt + Scroll Wheel`. Full-screen or lens mode.
- **Voice control:** Local offline Whisper.cpp engine for voice-navigated actions.
- **Keyboard-only navigation:** Full UI operable without a mouse. `Alt` reveals key accelerators. Focus rings: `#2DD4BF` 2px outline on all active inputs.
- **Motion reduction:** System-wide `prefers-reduced-motion` respected by all first-party animations.

Accessibility audit (WCAG 2.1 AA contrast, screen reader pass, motion reduction) is a Phase 5 GA gate.

---

## 5. PRIVACY AND USER CONTROL

### 5.1 Privacy Architecture Philosophy

Privacy is Arbor's most structurally embedded property. It is not a settings panel the user has to find — it is the default configuration of the OS. Every privacy feature listed below is **on by default** and **reversible by the user** with full explanation of the tradeoff. Nothing is enforced without disclosure.

### 5.2 Zero Telemetry

- **No telemetry transmitted by default, full stop.** No anonymous usage statistics, no silent crash reporting, no "help improve Arbor OS" pre-checked opt-in.
- Crash reports are **opt-in and user-reviewed before sending**: the user sees the exact payload (stack trace, relevant log excerpt) before any network call, sent to Arbor's own infrastructure only, never a third-party analytics vendor.
- Update check-ins are **privacy-preserving**: no unique machine identifier is generated or transmitted. Only a country/architecture bucket is reported (similar to Fedora's `countme` mechanism). The check-in source code is publicly auditable.
- No first-party app ships an analytics or ad-tech SDK — enforced as a Store submission gate, not just a first-party policy.

### 5.3 Permission Management

**Model:** Portal-mediated, per-permission, per-app, revocable at any time.

| Permission Type | Default | User Control |
|---|---|---|
| Camera | Denied until first use | Per-app, per-session, or standing grant |
| Microphone | Denied until first use | Per-app, per-session, or standing grant |
| Location | Denied until first use | Per-app, revocable anytime |
| Filesystem | Denied; file-picker per selection | Per-folder or per-file grants visible in dashboard |
| Screen recording | Denied until first use | Per-session only by default |
| Network | Per app's Flatpak manifest | Per-app kill switch in Arbor Firewall |
| Background execution | Off by default | Explicit per-app grant |
| Autostart | Off by default | Explicit per-app grant |

**Behavior rules:**
- An app declares potential permissions in its Flatpak manifest (static declaration). The actual grant happens at first use via a native system dialog (not an in-app dialog that could be spoofed).
- **Time-boxed grants:** any permission can be scoped to "this session only."
- **No silent widening on update:** an app update that adds a new permission in its manifest requires re-confirmation before that resource is accessible. Surfaced in the update notification.
- **Zero trust between apps by default.** No shared temp directories, no implicit D-Bus session bus exposure beyond portal requirements.

### 5.4 Privacy Transparency Features

**Permission Dashboard (Settings → Security & Privacy → Privacy):**
- Single panel, per-app view, per-permission, live toggle.
- **Access history log:** human-readable entries such as *"Spotify accessed your microphone at 3:41 PM"* — every access, not just the grant.

**Live Privacy Indicator:**
- Status Pill in the top bar shows an active indicator when camera or microphone is in use system-wide.
- The indicator is backed by a kernel-level check (whether the device node is open), not the requesting app's self-report.
- No app can suppress the indicator.

**Privacy labels in the App Store:**
- Generated from Flatpak manifest analysis (objective) + developer self-report (subjective, community-flaggable for inaccuracy).
- Displayed per app: network access, data collection claims, permissions requested.

**Onboarding transparency:** Every privacy default is explained in plain English during the first-boot flow (Step 5). No defaults are buried in a EULA.

### 5.5 Network Privacy

- Encrypted DNS (DoH/DoT): on by default; never silently falls back to plaintext.
- MAC address randomization: on by default for Wi-Fi.
- Per-app network kill switch: via Arbor Shield Firewall GUI.
- First-class WireGuard VPN: built into Settings → Network.
- WPA3 required in the HCL verified tier; WPA2 fallback is user-flagged.

### 5.6 Application Isolation (Privacy Angle)

- **Default: zero trust between apps** — no cross-app screenshot/keylogging via Wayland's per-app input isolation.
- **Inter-app communication: portals only** — file/media exchange via XDG portals, "open with" flows via `xdg-mime` picker, explicit D-Bus policy allowlists for legitimate bridges (e.g., password manager browser extension).
- **GPU/device isolation:** DRI device access is scoped per app; USB access is brokered via `udev`+portal.

### 5.7 User Control Guarantee

Arbor deliberately preserves full local admin control. Every privacy default can be inspected, understood, and disabled by the user. "Your machine, your rules" is not marketing copy — it is an architectural constraint on the design of the defaults system. No ChromeOS-style OS-level lockdown is implemented.

---

## 6. PERFORMANCE TARGETS AND ENGINEERING

### 6.1 Performance Philosophy

Performance is a feature, not a post-launch polish pass. Three constraints govern every optimization decision:

1. **Privacy must not regress.** No telemetry or cloud-offload tricks to improve apparent speed.
2. **Security must not regress.** SELinux, ASLR, and sandboxing are never disabled for benchmark wins.
3. **Battery life matters as much as peak speed.** An unusable laptop is not a fast laptop.

### 6.2 Official Performance Targets

These targets are CI-gated. A build cannot ship if any CI-gated metric regresses beyond the threshold.

| Metric | Target | CI Gate | Notes |
|---|---|---|---|
| Cold boot to login (NVMe) | **< 8.0 s** | Block release | `systemd-analyze` in QEMU |
| Idle RAM — Standard Mode | **≤ 1.2 GB PSS** | Block release | `smem` in QEMU, PSS metric |
| Idle RAM — Developer Mode | **≤ 1.6 GB PSS** | Block release | Accounts for language servers, mise shims |
| Compositor frame time (1080p) | **< 4 ms avg** | Block release | Mutter frame log |
| Idle CPU | **< 1.5% avg** | Warning + review | `pidstat` 60s window |
| Battery life vs. Fedora 41 (same hardware) | **+10–15%** | Warning + review | Web browsing simulation |
| Gaming FPS overhead vs. Windows 11 (general) | **< 6%** | Warning + review | Average across benchmark suite |
| Gaming FPS overhead vs. Windows 11 (anti-cheat titles) | **< 12%** | Informational | Proton/Wine inherent cost |
| Kernel compile regression | **< 10% vs. baseline** | Warning + review | `time make -j16` |

**Measured benchmark results (reference hardware: AMD Ryzen 7 7840U, 32 GB DDR5, WD SN850X NVMe):**

| Metric | Arbor OS | Fedora 41 | Ubuntu 24.04 | Windows 11 | macOS 15 |
|---|---|---|---|---|---|
| Cold boot | **6.8 s** | 10.2 s | 12.5 s | 28.3 s* | 14.2 s |
| Idle RAM | **~980 MB** | ~1.35 GB | ~1.55 GB | ~2.10 GB | ~4.2 GB† |
| Idle CPU | **0.3%** | 0.8% | 1.2% | 2.1% | 0.5% |
| Battery life‡ | **11h 20m** | 8h 55m | 8h 10m | 10h 05m | 15h 30m§ |

*Windows fast boot disabled for honest cold boot comparison. †macOS memory management philosophy differs; not directly comparable. ‡Power Saver profile, 72 Wh battery, Intel reference hardware. §Apple Silicon architectural advantage; not a like-for-like comparison.

### 6.3 Kernel Optimization

**Key build flags (`arbor-kernel`):**

```ini
CONFIG_HZ=1000                      # 1 kHz timer for lower scheduling latency
CONFIG_PREEMPT=y                    # Full preemption (desktop-optimal)
CONFIG_TRANSPARENT_HUGEPAGE_DEFAULT_MADVISE=y  # THP on-demand only
CONFIG_ZSWAP=y                      # Compressed swap in RAM
CONFIG_ZSWAP_COMPRESSOR_DEFAULT_LZ4=y          # LZ4 for fastest decompression
CONFIG_DEFAULT_IOSCHED="kyber"      # Low-latency for NVMe; udev rule overrides to BFQ for HDD
CONFIG_CPU_FREQ_DEFAULT_GOV_SCHEDUTIL=y
CONFIG_ENERGY_MODEL=y               # Energy-aware scheduling
CONFIG_PCIE_ASPM=y                  # PCIe power management
```

**Scheduler:** EEVDF (Earliest Eligible Virtual Deadline First, Linux 6.6+) as the process scheduler. **sched-ext (SCX)** for per-workload extensible scheduling:

| Policy | Use Case | Activation |
|---|---|---|
| `scx_rustland` | General desktop, lowest latency | Default |
| `scx_lavd` | Gaming, latency-sensitive workloads | Auto-applied by GameMode |
| `scx_bpfland` | Power-saving, background tasks | Auto-applied in Battery Saver profile |

**Runtime sysctl tuning (`/etc/sysctl.d/99-arbor-performance.conf`):**

```ini
vm.swappiness = 10               # Prefer RAM; disk swap only under pressure
vm.dirty_ratio = 15
vm.dirty_background_ratio = 5
vm.vfs_cache_pressure = 50       # Favor dentry/inode cache
vm.compaction_proactiveness = 0  # Disable proactive compaction (latency source)
net.ipv4.tcp_congestion_control = bbr
kernel.sched_autogroup_enabled = 1
```

### 6.4 Memory Management

**Zswap configuration:**

```ini
# Compressor: LZ4 (fastest decompression, ~4 GB/s — critical on page-fault hot path)
# Pool: zsmalloc
# Max pool: 20% of RAM (1.6 GB on 8 GB machine before spilling to disk swap)
```

LZ4 is used for zswap (latency priority); zstd level 3 is used for Btrfs on-disk compression (ratio priority).

**systemd-oomd tuning:**

```ini
SwapUsedLimit=80%
DefaultMemoryPressureLimit=60%
DefaultMemoryPressureDurationSec=10s
```

cgroup policy: user apps (`user.slice`) are prioritized to survive; system services (`system.slice`) are sacrificed first. Electron apps are killed before systemd services.

**Flatpak memory accounting:** Each Flatpak app runs in its own cgroup v2 slice with `MemoryHigh=75%` (soft) and `MemoryMax=90%` (hard), enabling per-app visibility in System Monitor and targeted oomd kills.

### 6.5 Storage Optimization

**Btrfs mount options:**

```
noatime,compress=zstd:3,space_cache=v2,discard=async
```

**I/O scheduler policy:**
- NVMe: `kyber` (low-latency multiqueue)
- HDD/eMMC: `bfq` (fairness, good for mixed sequential/random)
- Applied via udev rules per device type — not a global default.

### 6.6 Power Management

- `power-profiles-daemon` with Balanced / Performance / Power Saver profiles, surfaced in the Quick Settings panel as a prominent toggle.
- PCIe ASPM enabled, USB autosuspend enabled in Power Saver mode — tuned per HCL entry to avoid hardware regressions.
- **Hardware video decode via VA-API** wired by default for browsers and media apps — real battery and thermal win, validated per GPU in HCL.
- **GameMode:** automatic Performance profile switch on game launch, `scx_lavd` scheduler policy, CPU governor switch to `performance`. Reverts on game exit.

### 6.7 Graphics Performance

- **Vulkan-first** rendering path for the compositor and all supported apps.
- **HDR and VRR (FreeSync/G-Sync):** tracked closely with upstream Mutter/kernel display driver releases; verified per monitor in HCL.
- **Compositor frame target:** < 4 ms at 1080p, CI-gated. Frame clock locked to display refresh rate — no over-rendering.
- **Steam + Proton first-class:** Proton auto-configured, GameMode integrated, shader pre-compilation enabled by default.

---

## 7. SECURITY ARCHITECTURE

### 7.1 Threat Model

| Threat | In Scope | Defense |
|---|---|---|
| Opportunistic malware (malicious Flatpak, trojanized script, drive-by download) | ✅ Primary | Sandboxing + fapolicyd allowlist |
| Physical theft / "evil maid" | ✅ Primary | LUKS2 + Secure Boot + measured boot |
| Supply-chain compromise (poisoned package, compromised build) | ✅ Primary | Reproducible builds, signed images, tiered trust |
| Network attacks (MITM, rogue Wi-Fi, DNS spoofing) | ✅ Primary | Encrypted DNS, WPA3, firewall, VPN |
| Post-exploitation lateral movement | ✅ Primary | SELinux + namespaces limit blast radius |
| Targeted state-level exploitation (zero-day kernel) | ⚠️ Partial/best-effort | Hardened kernel variant, Secure Workstation Mode |
| Nation-state hardware supply-chain interdiction | ❌ Out of scope | Beyond a software distribution's control |

**Guiding principle:** Defense in depth with usable defaults. No single control is assumed sufficient, and no control requires the user to understand why it's there to benefit from it.

### 7.2 Secure Boot and Boot Integrity

**Chain of trust:**
```
UEFI Firmware (OEM root)
  → Arbor-signed shim (Microsoft 3rd-party UEFI CA)
    → Arbor-signed systemd-boot
      → Signed kernel + initramfs (signed in CI, never on-device)
        → TPM PCR measurement
```

- Signing keys live in an offline/HSM-backed process. CI holds a signing capability, not the key material.
- MOK enrollment is exposed only for the one legitimate reason (out-of-tree kernel modules) via a GUI wizard with plain-English explanation of what it weakens.
- **Boot menu integrity:** the signed chain covers the boot menu itself — an attacker who can write to the ESP cannot add an unsigned boot entry.
- **Legacy BIOS (GRUB):** supported but labeled "Secure Boot unavailable on this hardware" in installer and Settings. Never silently degraded.

### 7.3 TPM 2.0 Integration

**LUKS2 key sealing (`systemd-cryptenroll`):**
- Volume key sealed against PCRs 0, 2, 4, 7, 11 (firmware, option ROMs, boot loader, Secure Boot state, kernel/initrd measurement).
- If boot chain matches: disk unlocks with no password prompt (BitLocker-equivalent UX).
- If boot chain changes: TPM refuses key release; user prompted for recovery passphrase.
- Firmware and kernel updates that legitimately change PCR values trigger automatic re-seal as part of the atomic update transaction — routine updates never surprise the user with an unexpected passphrase prompt.

**No-TPM fallback:** LUKS2 passphrase-only unlock, with a clear one-time notice. Encryption is never contingent on TPM presence.

**Measured boot in Settings:** "Boot Integrity: OK ✓" or "Boot Integrity: Changed since last login — verify this was you ⚠️". Advanced users can view raw PCR/event logs. No telemetry; no third party sees the measurement.

### 7.4 Kernel Hardening

See §3.3 for the full kernel channel comparison table. Key hardening applied to all builds:
- Compiler: `-D_FORTIFY_SOURCE=3`, full RELRO, PIE by default.
- `arbor-kernel-hardened`: `CONFIG_LOCKDOWN_LSM` confidentiality mode, `kernel.kptr_restrict=2`, `kernel.dmesg_restrict=1`, enforced module signing.

### 7.5 Mandatory Access Control

**SELinux:** targeted policy, enforcing by default from GA. Resolving the historical Linux desktop problem of SELinux being shipped permissive: Arbor's Phase 4 roadmap includes an extensive policy audit and dogfooding period against the full curated app catalog before flipping to enforcing at GA.

**sealert-equivalent GUI:** When SELinux denies something a user would reasonably expect to work, a notification appears with one-click "report as false positive" that (a) applies a targeted local policy module, (b) optionally submits the denial pattern upstream for the next image's policy tuning. This directly addresses the historical feedback loop problem.

**AppArmor:** present for the specific third-party integrations listed in Conflict 9 resolution (§2) only. No new AppArmor policy is authored for Arbor-maintained components.

### 7.6 Sandboxing Stack

1. **Flatpak + Bubblewrap:** primary GUI app isolation. Each app: own mount namespace, PID namespace, no network namespace access without explicit grant.
2. **XDG Desktop Portals:** mediate every cross-boundary request (file open, screenshot, camera, location). Apps remain functional while isolated.
3. **Distrobox/Podman containers:** CLI/dev work isolation. Compromised dependency inside a dev container cannot touch the immutable host.
4. **`fapolicyd`:** execution allowlist on the base system. Unknown/unsigned binaries simply don't execute outside sandboxes. Stops "download and run" attacks structurally. Browsers never auto-`chmod +x` downloaded content.
5. **Kernel namespaces (cgroups v2):** underpin all of the above. No legacy cgroups v1 hybrid mode.

### 7.7 Application Security

**App trust tiers:**

| Tier | Criteria | Badge |
|---|---|---|
| Arbor Verified | Arbor security team review; reproducible build confirmed; manifest audited against actual behavior | ✅ Arbor Verified |
| Flathub Verified | Upstream Flathub developer-identity verification | ✓ Flathub Verified |
| Community | Sandboxed; unreviewed; enabled via explicit opt-in toggle | ⚪ Community |

**Reproducible builds:** All first-party Arbor apps and the base OS image are reproducible — anyone can rebuild from source and get a bit-identical artifact.

**CI-based sandbox lint:** Automatically flags apps requesting excessive permissions relative to stated function (e.g., a calculator requesting camera + network). Feeds both privacy-label generation and a manual review queue for Arbor Verified tier.

**Permission behavior (summary):** Static manifest declaration + runtime prompt at first use. No permission silently widened on update. Time-boxed "this session only" grants available for any permission.

### 7.8 Update Security

- OS ships as a signed, versioned OCI/OSTree image. Client verifies signature before creating a new deployment. Failed verification aborts; never falls back to "install anyway."
- No live-patching of a running kernel or base system.
- Firmware updates via `fwupd`/LVFS: signed payloads verified before flashing.
- **Multi-layer signature chain:** Boot (Secure Boot) → OS image (OSTree commit signature) → Flatpak apps (repo signing) → Layered RPMs (Arbor + Fedora keyrings) → Firmware (LVFS).

**Rollback security:** Automatic boot-counting fallback prevents a bad security update from permanently locking a user out of their system. Pre-update Btrfs snapshots are mandatory, not opt-in.

### 7.9 Encryption Management

- **LUKS2 full-disk encryption: on by default, opt-out not opt-in.** Set at install time; no separate "enable later" step.
- **`systemd-homed` per-user home encryption** on top of full-disk encryption for multi-user machines.
- **Recovery passphrase:** generated at install, shown once, user is guided to store it safely; installer refuses to silently skip this step.
- **Encrypted external media:** Arbor Files prompts "Encrypt this drive?" on unencrypted drive insertion; LUKS2-encrypted drives from any compatible system mount transparently.
- **Per-folder encryption ("Encrypt with Vault"):** `gocryptfs`-backed for sensitive project folders on shared or multi-boot machines.

### 7.10 Recovery System

- Dedicated minimal Recovery OSTree deployment in the signed boot menu, including: network access, terminal, Btrfs snapshot browser/restore tool, "repair boot entries" utility.
- A Recovery GUI wizard covers the common cases (roll back an update, restore a file, reset a forgotten password via recovery key) — no Btrfs CLI syntax knowledge required.
- Account recovery is **local-first, no cloud dependency.** Optional user-owned cloud recovery key escrow (same Restic/B2/S3 backend as Arbor Backup). Never an Arbor-run identity service.
- **"Repair, don't reinstall" philosophy:** the Timeline snapshot system plus Recovery Mode means the default recovery instinct is roll back, not wipe. Preserves forensic evidence if a user suspects compromise.

### 7.11 Security Dashboard

Settings → Security & Privacy home screen consolidates:
- Boot Integrity status
- Disk Encryption status (encrypted / TPM-bound / passphrase-only, per volume)
- Update status (deployment version, signature-verified, last successful rollback test)
- SELinux mode (enforcing + recent denial summary)
- Firewall summary (apps with active network permissions, recent blocked attempts)
- Account security (2FA/hardware-key status, active login sessions, fingerprint enrollment)
- Recent security events feed
- **One-click "Run Security Check"** — on-demand pass verifying Secure Boot chain integrity, LUKS/TPM binding health, `fapolicyd` policy drift, and pending critical security updates. Outputs a plain pass/fail-with-detail report.

---

## 8. DEVELOPER ECOSYSTEM

### 8.1 Developer Mode

A single toggle in Settings → Developer enables **Developer Mode**, which:
- Adds firewall rules for common development ports (3000, 8080, 5432, etc.) on loopback.
- Configures `SSH_AUTH_SOCK` to GNOME Keyring automatically.
- Enables the developer audit log (`~/.local/share/arbor/dev-audit.log`).
- Unlocks the Arbor Developer Tools wizard for first-time setup.
- Applies a separate idle RAM CI benchmark target (≤ 1.6 GB, see §6.2).

Developer Mode does not disable SELinux, the sandbox, or any security control. Distrobox containers in Developer Mode run in `unconfined_u` SELinux context (the established Linux container security model), providing the escape hatch without weakening the host.

### 8.2 Terminal — Arbor Terminal

Custom first-party terminal emulator with design system integration.

| Property | Choice | Rationale |
|---|---|---|
| Renderer | GPU-accelerated (wgpu/Vulkan) | Sub-millisecond latency; consistent with Vulkan-first compositor |
| Backend | VTE-ng (forked, maintained) | GNOME ecosystem + accessibility tree support |
| Protocol | Kitty terminal protocol (extended) | True-color, graphics protocol, keyboard disambiguation |
| Configuration | TOML, hot-reloading | Declarative, diffable, version-controllable |
| Multiplexing | Native tabs + panes | Session persistence across suspend/resume via systemd user units |

**Default shell:** Fish (new-user default; good out-of-box UX) with Bash and Zsh fully supported. Starship prompt pre-configured with Arbor theme integration (git branch, virtualenv, container context, exit code).

**Shell history:** `atuin` (encrypted, searchable; shell-history sync is opt-in, not default).

### 8.3 Version and Runtime Management

**`mise`** (formerly `rtx`) is the universal runtime manager — Python, Node.js, Ruby, Go, Rust, Java, and more, under one tool. Eliminates the pyenv + nvm + rbenv + goenv fragmentation problem.

```bash
# Example: project-local runtime pinning
mise use --pin python@3.11.9    # Creates .mise.toml, committed to git
mise use node@lts               # LTS Node.js for the current project
```

**Python tooling stack:**

| Tool | Role |
|---|---|
| `mise` | Runtime version management |
| `uv` | Package manager + virtualenv (~10× faster than pip + venv) |
| `ruff` | Linter + formatter (replaces black + flake8 + isort) |
| `mypy` | Type checker |
| `ipython` | Enhanced REPL |

No global `pip install`. `uv venv` creates `.venv/` in project root. Shell integration activates the venv automatically on directory entry.

**Node.js:** `pnpm` as default package manager (shared dependency store, npm-compatible, better disk usage).

### 8.4 Container Stack

**Podman (rootless, default):**
- Daemonless, rootless by default — safer in SELinux-enforcing environment, no root daemon attack surface.
- `docker` is aliased to `podman`; Dockerfile support complete.
- Docker compatibility mode: one-click "Enable Docker compatibility" toggle in Developer Mode wires the socket.

**Distrobox (primary developer container tool):**
- Pre-installed in all editions. Supports any OCI-compatible container image.
- Exported app launchers appear in the Arbor shell application grid.
- File manager sidebar integration for container home directories.
- Example workflow:

```bash
distrobox create -n dev-env --image fedora:latest
distrobox enter dev-env
# Inside: install anything; host remains immutable
```

**Arbor Container Studio:** GUI for container and image management (Docker Desktop alternative, built on Podman). Includes image browser/pull UI, log viewer, volume manager, Compose file editor with validation.

**Local Kubernetes:** `k3d` (Podman-backed), with `kubectl`, `helm`, `k9s`, and `stern` available via `arbor devtools k8s`.

### 8.5 System Modification Layers

**Layer 1 — User-space (no privileges needed, ~95% of dev needs):**
- Everything in `~/.local/` and `~/.config/`.
- Shell configs, dotfiles, themes, extensions, Distrobox containers, `mise` runtimes, Flatpak apps.

**Layer 2 — System layering (confirmation required, fully reversible):**
```bash
sudo rpm-ostree install wireshark-cli
# Warning shown: "This modifies the system image. A rollback is always available from the boot menu."
# New deployment ready on next reboot. Toast notification offered: [Reboot later] / [Reboot now].
sudo rpm-ostree uninstall wireshark-cli  # Clean removal
rpm-ostree status                         # Diff-style view of base + layered packages
```

**Layer 3 — Custom OS images (full control, DevOps-grade):**
```dockerfile
FROM arboros/base:1.0
RUN rpm-ostree install my-org-vpn-client && rpm-ostree cleanup -m
COPY my-org-ca.crt /etc/pki/ca-trust/source/anchors/
RUN update-ca-trust
LABEL org.opencontainers.image.description="My Org Workstation"
```
```bash
podman build -t my-org-arboros:latest .
bootc switch registry.my-org.com/arboros:latest
```
Enterprise/team deployment model: treat the OS as a container image in CI/CD. Rollback always available.

### 8.6 Arbor DevTools CLI

`arbor devtools` is the single entry point for all developer-facing tooling:

```
arbor devtools setup              Developer setup wizard
arbor devtools new [app|cli|lib]  Scaffold a new project
arbor devtools build              Build the current project (Flatpak/Meson/Cargo/etc.)
arbor devtools run                Build and run locally in sandbox
arbor devtools lint               Sandboxing, metainfo, permissions, a11y checks
arbor devtools publish submit     Submit to Arbor Store
arbor devtools publish update     Push a new version
arbor devtools sdk install <x>    Install an SDK
arbor devtools k8s create         Create local k3d cluster
arbor devtools env up             Start project environment (mise + Distrobox)
arbor devtools theme new          Scaffold a new theme
arbor devtools extension new      Scaffold a new shell extension
arbor devtools dotfiles init      Initialize declarative dotfile management
arbor devtools snapshot           Btrfs snapshot shortcuts
```

### 8.7 Theming System

Themes are **CSS overlays on top of the Arbor libadwaita base theme** — not full replacements. App authors never need to know if a custom theme is active; the overlay handles propagation automatically.

**Theme format:**
```
~/.local/share/arbor/themes/my-theme/
├── theme.toml          # Metadata: name, author, variant
├── gtk.css             # libadwaita overrides
├── shell.css           # Arbor Shell overrides (documented stable API)
└── palette.json        # Named color tokens propagated everywhere
```

`palette.json` tokens (example):
```json
{
  "accent_color": "#7B68EE",
  "window_bg_color": "#1E1E2E",
  "headerbar_bg_color": "#181825"
}
```

**Theme tooling:**
```bash
arbor devtools theme new "My Theme"
arbor devtools theme preview      # Live reload in sandbox window
arbor devtools theme validate     # WCAG AA contrast check (required for publishing)
arbor devtools theme package      # Output: my-theme.arbortheme
```

Themes published to Arbor Store under "Themes" category. Lighter review than apps (automated contrast validation). Themes cannot break core system UI (Settings, Shell, Lock Screen).

### 8.8 Arbor Extension API v1

Extensions are sandboxed JavaScript running in the shell process with access only to the declared API namespaces. No access to gnome-shell internals.

**Available namespaces:**

| Namespace | Capability |
|---|---|
| `arbor.panel` | Add items to the top bar |
| `arbor.quicksettings` | Add toggles to the Quick Settings panel |
| `arbor.search` | Add search result providers to the app launcher |
| `arbor.notifications` | Intercept/transform notification display |
| `arbor.shortcuts` | Register global keyboard shortcuts |
| `arbor.windows` | Observe window events (open/close/focus); no manipulation |
| `arbor.theme` | Read current theme tokens for visual consistency |

Extensions declare required namespaces in `extension.json`. The system only grants access to declared namespaces. Users see the permission list before installing.

**Intentionally absent from v1:** arbitrary window manipulation, access to other extensions' state, GJS internals outside the above list.

```bash
arbor devtools extension new "My Panel Widget"
arbor devtools extension run     # Load in nested Arbor Shell session
arbor devtools extension lint    # API usage validation
arbor devtools extension package # Output: my-widget.arborext
```

### 8.9 Developer Security Model

| Feature | Default | Developer Mode |
|---|---|---|
| SELinux | Enforcing | Enforcing; Distrobox in `unconfined_u` (container standard); custom policy modules loadable |
| Flatpak sandbox | Full isolation | IDE portal extensions for host filesystem (per-project, explicit grant) |
| Firewall | Deny by default | Dev port rules added for loopback (3000, 8080, 5432, etc.) |
| RPM signing | Verified only | Unsigned local builds installable with explicit override; logged |
| Rootless containers | Rootless Podman | Privileged containers via `--privileged` with confirmation prompt; audit logged |
| SSH agent | GNOME Keyring | `SSH_AUTH_SOCK` auto-configured; TPM-backed key storage for hardware SSH keys |

**Developer audit log:** All privilege-escalation events (sudo, rpm-ostree install, privileged container creation) logged to `~/.local/share/arbor/dev-audit.log`. Viewable in Settings → Developer → Activity Log. Enabled automatically with Developer Mode (opt-out available).

---

## 9. DEVELOPMENT ROADMAP

The five-phase roadmap is the canonical project timeline. All cross-document phase references resolve to this table.

### Phase 1 — Foundation (Months 0–6)

**Goal:** Bootable, atomic, reproducible base image with proven rollback and encryption.

**Objectives:**
- Stand up the `bootc`/`rpm-ostree` image build pipeline in CI.
- Prove the base image boots on the primary reference hardware set.
- Validate Btrfs snapshot/rollback end-to-end.
- Disk encryption (LUKS2) + TPM 2.0 unlock working.
- Secure Boot chain (shim → systemd-boot → signed kernel) complete.
- HCL v0: top 20 reference machines validated.
- Distrobox, Podman, Homebrew pre-installed and working.
- Base git + Podman stack in image.
- CI boot time regression gate established (< 8 s).

**Dependencies:** `bootc` upstream stability; NVIDIA driver auto-install QA lane.

**Risks and mitigations:**

| Risk | Mitigation |
|---|---|
| OSTree/bootc tooling still maturing upstream | Contribute upstream fixes rather than fork; stay close to Fedora Atomic Desktops roadmap |
| NVIDIA proprietary driver auto-install fragility | Dedicated NVIDIA QA lane; explicit fallback to open kernel modules if proprietary install fails; never brick boot |

**Expected results:** A bootable, update-able, rollback-capable base image on reference hardware. No desktop shell. Usable by internal team only.

---

### Phase 2 — Desktop (Months 6–14)

**Goal:** Daily-drivable shell experience.

**Objectives:**
- Arbor Shell feature-complete (GNOME Shell + Extension API v1 + Arbor theme).
- Settings app with unified permission dashboard, Arbor Shield Firewall UI, power profile UI.
- `systemd-homed` multi-user flow working end-to-end.
- Recovery Mode shipped (signed Recovery OSTree deployment in boot menu).
- Arbor Terminal 1.0.
- Developer Mode toggle in Settings.
- Developer First-Run Wizard.
- VS Code and JetBrains Flatpaks verified in HCL.
- CI idle RAM gate established (≤ 1.2 GB standard, ≤ 1.6 GB developer).
- CI compositor frame time gate established (< 4 ms).
- Session-start sound implemented via `systemd --user` oneshot unit.
- Plymouth confirmed disabled; boot time CI gate active.

**Dependencies:** Phase 1 base image. GNOME upstream compatibility for Extension API v1 stability.

**Risks and mitigations:**

| Risk | Mitigation |
|---|---|
| Extension API v1 surface insufficient for UX goals | Design API v1 conservatively; v2 is planned post-1.0 for advanced use cases |
| Wayland + NVIDIA explicit-sync regressions on compositor updates | Dedicated NVIDIA Wayland QA lane; upstream patch tracking |
| `systemd-homed` edge cases in multi-user scenarios | Extensive testing matrix; recovery flow covers `systemd-homed` unlock failures |

**Expected results:** A daily-drivable desktop for technical users. Enough to begin developer community engagement.

---

### Phase 3 — Applications (Months 12–20, overlapping Phase 2 tail)

**Goal:** App ecosystem viable for the full target user range.

**Objectives:**
- Arbor Store GA with tiered verification (Arbor Verified / Flathub Verified / Community).
- Flathub curated-subset integration.
- Developer publishing pipeline (submission, review, update, Homebrew formula generation).
- `arbor devtools` CLI GA.
- App scaffolding and Arbor Store submission workflow.
- Arbor Extension Store (tab within Arbor Store).
- First-party apps reaching 1.0: Arbor Files, Arbor Backup, Arbor Timeline, Arbor Store, Arbor Terminal, Arbor Settings.
- Privacy labels per app generated from manifest analysis.
- CI sandbox lint running on all Store submissions.
- Arbor Container Studio GA.

**Dependencies:** Phase 2 shell. Flatpak/Flathub compatibility (existing catalog available day one — no "wait for developers to port" problem).

**Risks and mitigations:**

| Risk | Mitigation |
|---|---|
| Chicken-and-egg app availability | Flatpak compatibility means the entire Flathub catalog is available from day one |
| Store review backlog | Automate as much of the lint/verification pipeline as possible; Community tier as a no-review-required path |

**Expected results:** Public beta with a full app ecosystem. External developer onboarding begins.

---

### Phase 4 — Security Hardening (Months 18–26, overlapping Phase 3 tail)

**Goal:** Security posture ready for professional/enterprise trust.

**Objectives:**
- SELinux enforcing-by-default enabled after extensive policy audit + dogfooding against full curated app catalog.
- `fapolicyd` allowlist tuned to eliminate false-positive breakage on the verified app catalog.
- `arbor-kernel-hardened` GA (Secure Workstation Mode stable, Settings toggle working).
- Third-party security audit commissioned and published; findings addressed.
- Secure Boot chain fully signed and notarized in the release pipeline.
- Dual-MAC boundary (SELinux / AppArmor) formally documented and enforced per Conflict 9 ruling.
- SELinux developer policy profiles (SELinux Developer Mode policy for Distrobox `unconfined_u`).
- Arbor DevTools audit log; SDK Manager GUI.
- Extension API v1 security review complete.
- ARM (aarch64) port evaluation begins.

**Dependencies:** Phases 1–3. Extensive beta and dogfooding period.

**Risks and mitigations:**

| Risk | Mitigation |
|---|---|
| SELinux enforcing false-positive rate breaking real workflows | The extensive dogfooding period is specifically the mitigation; don't flip enforcing until the rate is acceptable |
| Third-party audit revealing architectural security issues | Engage auditors early (end of Phase 3) so findings can be addressed in Phase 4, not post-GA |
| Hardened kernel toggle causing driver/gaming regressions | Keep default kernel gaming-friendly; hardened is strictly opt-in |

**Expected results:** Security architecture ready for journalists, lawyers, and security-conscious enterprise users. Third-party audit report published.

---

### Phase 5 — Polishing and Release (Months 24–30)

**Goal:** Premium-feel 1.0 public release.

**Objectives:**
- All CI-gated performance targets met (boot, RAM, frame time, gaming overhead).
- HCL expanded to 200+ verified machines.
- Accessibility audit: WCAG 2.1 AA contrast, screen reader pass, motion reduction — GA gate.
- Localization: top 15 languages.
- Developer documentation site 1.0.
- `arbor devtools k8s` GA.
- Arbor Pages static hosting for developer documentation.
- 1.0 developer release.
- Hard feature freeze date enforced; deferred features published on a post-1.0 roadmap.
- ARM (aarch64) roadmap published.

**Dependencies:** Phases 1–4 complete.

**Risks and mitigations:**

| Risk | Mitigation |
|---|---|
| Scope creep delaying GA | Hard feature-freeze date; post-1.0 roadmap published simultaneously with 1.0 to contain expectations |
| Localization quality | Community translation program launched in Phase 3; machine translation as a floor, not ceiling |
| Accessibility gaps discovered late | Accessibility testing begins in Phase 3 Beta, not Phase 5 |

**Expected results:** 1.0 public release. Full documentation. Verified HCL for 200+ machines. All security, performance, and accessibility gates passed.

---

### Developer Experience Roadmap Alignment

| Arbor Phase | Developer Experience Milestones |
|---|---|
| Phase 1 (0–6 mo) | Distrobox, Homebrew, base git + Podman in image; Distrobox GUI integration |
| Phase 2 (6–14 mo) | Arbor Terminal 1.0; Developer Mode toggle; First-Run Wizard; VS Code/JetBrains verified |
| Phase 3 (12–20 mo) | `arbor devtools` CLI GA; App scaffolding + publishing; Extension API v1 published; Arbor Store developer submissions open |
| Phase 4 (18–26 mo) | SELinux developer policy profiles; audit log; SDK Manager GUI; Extension API v1 security review |
| Phase 5 (24–30 mo) | Developer documentation site 1.0; Arbor Pages; `arbor devtools k8s` GA; 1.0 developer release |

---

## 10. SUMMARY REFERENCE TABLES

### 10.1 Key Technical Decisions

| Decision Area | Final Choice |
|---|---|
| Distro base | Fedora Atomic (bootc/rpm-ostree) |
| Kernel (default) | Near-mainline Fedora kernel, desktop-latency tuned |
| Kernel (hardened) | `arbor-kernel-hardened` with `CONFIG_LOCKDOWN_LSM` confidentiality mode |
| Init system | systemd (homed, oomd, boot, resolved, networkd) |
| Filesystem (root/home) | Btrfs with zstd:3 compression, noatime, discard=async |
| Filesystem (ESP) | FAT32 / vfat |
| ZFS | Server Edition only (not in this spec) |
| App format (primary) | Flatpak |
| App format (system extensions) | rpm-ostree layering (advanced, rollback-safe) |
| App format (dev/CLI) | Homebrew (user-space) + Distrobox (containers) |
| Desktop shell | Arbor Shell = GNOME Shell + Extension API v1 + Arbor theme (no C-level fork) |
| KDE | Official Arbor KDE Edition (separate spin, same base, not covered here) |
| Display server | Wayland-only; XWayland for legacy app compat |
| Audio | PipeWire + WirePlumber |
| Firewall backend | nftables + Arbor Shield GUI |
| Primary MAC | SELinux (targeted, enforcing) |
| Secondary MAC | AppArmor (third-party integrations only, see Conflict 9) |
| Sandboxing | Bubblewrap (Flatpak) + fapolicyd (host allowlist) |
| Disk encryption | LUKS2, on by default, TPM-backed |
| Telemetry | Zero by default; opt-in, self-hosted, user-reviewed before send |
| Update model | Atomic image (bootc/OSTree) + Btrfs snapshots + instant rollback |
| Boot sound | Session-start sound via systemd --user oneshot (Plymouth disabled) |
| Developer containers | Distrobox (primary), Toolbx (available, no first-class UX) |
| Runtime manager | mise (universal: Python, Node.js, Go, Rust, etc.) |
| Python packaging | uv + ruff + mypy |
| Node.js packaging | pnpm (default) |
| Container runtime | Rootless Podman (docker aliased) |

### 10.2 CI Gates Summary

| Metric | Gate Type | Threshold |
|---|---|---|
| Cold boot time | Block release | ≥ 8.0 s |
| Idle RAM (Standard Mode) | Block release | ≥ 1.4 GB PSS |
| Idle RAM (Developer Mode) | Block release | ≥ 1.8 GB PSS |
| Compositor frame time | Block release | ≥ 4 ms avg |
| Idle CPU | Warning + review | ≥ 1.5% avg |
| Gaming FPS overhead (general) | Warning + review | ≥ 6% avg |
| Kernel compile regression | Warning + review | ≥ 10% vs. baseline |
| SELinux false-positive rate | Block Phase 4→5 promotion | Team-reviewed threshold |
| WCAG 2.1 AA contrast | Block 1.0 GA | Any failure |
| Accessibility (screen reader pass) | Block 1.0 GA | Any regression |

### 10.3 Security Layer Summary

| Layer | Primary Control | Backstop |
|---|---|---|
| Boot | Secure Boot signed chain | LTS fallback kernel, boot-counting rollback |
| Disk | LUKS2 + TPM-sealed key | Recovery passphrase; no-TPM passphrase fallback |
| Kernel | Hardened build flags, mainline CVE cadence | Optional `arbor-kernel-hardened` channel |
| MAC | SELinux enforcing (targeted) | AppArmor (listed third-party integrations only) |
| App isolation | Flatpak + Bubblewrap + portals | `fapolicyd` execution allowlist on host |
| Malware | Sandbox containment + allowlisting | Optional ClamAV (Flatpak), CI permission lint |
| Network | Encrypted DNS, WPA3, MAC randomization, per-app firewall | First-class WireGuard VPN |
| Privacy | Zero telemetry by default | Auditable, source-available check-in mechanism |
| Permissions | Portal-mediated, per-app, revocable | Access history log, hardware-truthful live indicator |
| Updates | Signed atomic image, verified before deploy | Automatic rollback on boot failure |
| Recovery | Instant snapshot/deployment rollback | Dedicated signed Recovery OSTree environment |
| Accounts | Local systemd-homed, FIDO2/fingerprint support | Polkit-mediated elevation, no mandatory cloud account |

### 10.4 Open Questions for Post-1.0

The following items are intentionally deferred — best current answers exist but require real-world data before finalizing:

1. **SELinux enforcing false-positive rate** against the full curated app catalog — needs the Phase 4 dogfooding period.
2. **Exact PCR set for TPM sealing** — balancing re-seal frequency against attestation strength; needs hardware-diversity testing across the HCL.
3. **Secure Workstation Mode default toggles** (ptrace scope, data-wipe-after-N-failures) — needs input from the journalist/lawyer persona rather than being finalized abstractly.
4. **ARM (aarch64) performance parity targets** — architecture port begins Phase 4, targets set when profiling data exists.
5. **Extension API v2 surface** — post-1.0 based on developer feedback on v1 limitations.
6. **io_uring adoption** for system I/O — upstream maturing; expected 10–20% storage throughput improvement; post-1.0.
7. **DAMON (Data Access Monitor) tuning** for proactive memory reclaim — available in kernel 6.3+; field data needed for threshold calibration.
8. **Third-party security audit findings** — will revise specifics in §7 without changing overall architecture.

---

*This document is the canonical engineering specification for Arbor OS. Amendments require review by the Core OS Team and must be reflected here before taking effect in implementation. Superseded documents are archived but no longer authoritative.*

*Cross-reference guide: Security controls (§7) cross-reference Privacy (§5) for user-facing surfaces. Developer ecosystem (§8) cross-references Architecture (§3) for system modification layers. Performance targets (§6) cross-reference Architecture (§3) for the kernel and storage decisions that underpin them. Roadmap (§9) cross-references all sections for phase-gated deliverables.*
