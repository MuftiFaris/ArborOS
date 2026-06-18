# [Codename: ARBOR OS] — Architecture Blueprint
### A Privacy-First, Performance-Focused Linux Distribution
**Document version:** 1.0 · **Status:** Pre-development architecture spec

---

## 0. Executive Summary

Arbor OS is a from-a-strong-base (not from-scratch) Linux distribution built on Debian Testing/Fedora hybrid tooling, targeting the gap between "technically capable but ugly/fragmented" (mainstream Linux) and "polished but closed" (macOS/Windows). The thesis: **you don't out-engineer the kernel, you out-engineer the integration.** Linux's technical foundation (cgroups, namespaces, Wayland, PipeWire, systemd, Btrfs) is already at parity or ahead of proprietary OSes. What's missing is a single opinionated team making 10,000 small consistency decisions instead of leaving them to fragmented upstream defaults.

---

## 1. OS PHILOSOPHY

### Target Users (in priority order)
1. **Privacy-conscious professionals** — journalists, lawyers, researchers, security-aware knowledge workers who currently tolerate macOS/Windows for polish despite distrust of telemetry.
2. **Developers** — who want a Unix environment without fighting driver hell or theming inconsistency.
3. **Prosumers/creatives** — video/photo/audio work, currently locked to macOS for color-managed, stable creative pipelines.
4. **Gamers** — via Proton/Wine maturity, wanting a SteamOS-like experience but general-purpose.
5. **Switchers** — people leaving Windows 11 over telemetry/ads/forced accounts, who need something that "just works" without a terminal.

### Main Advantages Over Incumbents
| Axis | macOS | Windows 11 | Arbor OS |
|---|---|---|---|
| Telemetry | Opt-out, opaque | Mandatory core telemetry | Zero by default, auditable |
| Hardware lock-in | Apple Silicon only | Broad but bloated | Broad, curated HCL |
| Package trust model | Notarization (Apple-controlled) | Store + arbitrary EXE | Reproducible builds + signed repos |
| Customization | Very limited | Limited | Full, without breaking updates (declarative overlay) |
| Cost | Hardware tax | License + ads | Free, optional paid support tier |
| Update reliability | Good | Historically rocky | Atomic, always-rollback-able |

### Unique Identity
Arbor OS's identity is **"a tree, not a cathedral"** — a strong trunk (immutable, verified base system) with branches (user-space flexibility) that can be pruned/grown without threatening the trunk's integrity. Visually and philosophically this maps to: calm, deterministic, undo-able. The tagline concept: *"Change anything. Break nothing."*

### Design Principles
1. **Immutable core, mutable edges** — the base OS is read-only and image-based; everything user-facing is layered/sandboxed.
2. **Privacy is a default, not a setting** — no dark patterns, no opt-out telemetry, no cloud account requirement.
3. **One good default beats ten mediocre options** — curate; don't ship five text editors "for choice."
4. **Every destructive action is reversible** — updates, package installs, config changes are all snapshot-backed.
5. **Consistency over choice paralysis** — a single toolkit (design system) spanning system and first-party apps.
6. **Open, but opinionated** — fully open source, upstream-first contributions, but Arbor makes the calls macOS makes internally.

---

## 2. BASE SYSTEM

### Distro Base Decision

| Option | Verdict | Reasoning |
|---|---|---|
| **Arch** | ❌ Rejected as base | Rolling release is too unstable for "premium/stable" positioning; no image-based update story out of the box |
| **Ubuntu LTS** | ❌ Rejected as base | Canonical's Snap/telemetry decisions conflict with privacy-first identity; brand confusion ("just another Ubuntu remix") |
| **Fedora (Silverblue/uBlue lineage)** | ✅ **Selected as base** | Best-in-class immutable/atomic tooling (`rpm-ostree`/`bootc`), SELinux maturity, fast upstream kernel cadence, strong Red Hat engineering behind Wayland/PipeWire/systemd — the exact stack we want |
| **Debian** | ✅ **Selected for package layer philosophy** (not base) | Reproducibility culture and stability testing inform our repo QA process, but Debian's release cadence is too slow for driver/hardware currency |

**Decision: Fedora Atomic (rpm-ostree/bootc image-based) as the technical base, with a Debian-style multi-tier repo QA process layered on top, and Flatpak as primary app format.** This gives us: RHEL-grade security engineering (SELinux, sandboxing), OSTree's proven atomic/rollback model (already field-tested via Fedora Silverblue, Universal Blue, Bazzite, Aurora), and current kernel/driver support without waiting on a slow-moving stable base.

### Kernel Strategy
- **Base:** Fedora's kernel package (near-upstream, fast CVE turnaround) with **linux-hardened** patch set optional as a toggleable kernel variant (not default — default favors compatibility/gaming; hardened variant for the "Secure" edition).
- **Two kernel channels:**
  - `arbor-kernel` (default): mainline + hardware-enablement backports, tuned for desktop latency (low-latency scheduler config, `CONFIG_PREEMPT` desktop-friendly).
  - `arbor-kernel-hardened`: grsecurity-inspired hardening flags, KASLR+, stack protector strong, restricted `/proc`, opt-in for Secure Workstation edition.
- **LTS fallback kernel** always kept installed as a boot entry for regression safety (GRUB/systemd-boot menu auto-populated by rpm-ostree deployments — this is essentially free with the atomic model).
- Kernel updates ship as part of atomic image updates, never live-patched blindly — always test-boots into a new deployment before becoming default.

### Package Manager
Two-tier model, not one:
1. **System layer:** `rpm-ostree`/`bootc` — image-based, atomic, for base OS and drivers only. Users don't "apt install" system packages; the base image is built centrally and delivered as a versioned OCI image (bootc allows treating the OS like a container image — huge win for reproducibility and CI/CD of the OS itself).
2. **Application layer:** **Flatpak-first** (via Flathub + Arbor's own curated/verified remote), with:
   - `Homebrew` supported out-of-box for CLI/dev tooling (already solves cross-distro package sprawl well).
   - `rpm-ostree install` / "layering" as an escape hatch for things that truly need system-level integration (kernel modules, drivers) — clearly labeled as an advanced/rollback-relevant action in UI.
   - **No apt/dnf exposed as the "normal" workflow** — it exists under the hood, but the user-facing story is Flatpak (GUI apps) + Homebrew/Distrobox (dev/CLI) + rpm-ostree (system extensions).
3. **Distrobox/Toolbx built in** for containerized dev environments — lets developers install literally anything without touching the immutable host.

### Init System
**systemd.** Not a real debate at this point — cgroups v2 integration, socket activation, `systemd-homed` for portable/encrypted home directories, `systemd-oomd` for OOM handling, `systemd-boot`/`sd-boot` compatibility, and unit-file consistency make it the only serious choice for a 2026-era distro. We adopt aggressively:
- `systemd-homed` for user account encryption/portability
- `systemd-oomd` tuned as default OOM strategy (big UX win over stock OOM-killer stalls)
- `systemd-boot` as default bootloader (simpler, faster, better UEFI/Secure Boot story than GRUB) with GRUB fallback for BIOS/legacy hardware.

### Filesystem
- **Root:** Btrfs, subvolume-per-deployment, required for OSTree/bootc's atomic model and integrates with Snapper-style snapshotting.
- **Home:** Btrfs by default too (unified snapshot story across root+home), with `systemd-homed` managing per-user LUKS containers so home directories are individually encrypted and portable (pluggable between machines).
- **Optional ZFS** offered for power users/servers wanting the Arbor server edition (send/receive replication story is superior for backup-heavy workflows) — not default on desktop due to licensing/DKMS friction with atomic kernel updates.
- Filesystem-level compression (zstd) on by default — real-world SSD wear + space savings.

### Boot System
- UEFI + Secure Boot (Arbor-signed shim, chainloading `systemd-boot`) as the only fully-supported path; legacy BIOS supported via GRUB but not security-hardened.
- Boot Menu shows: current deployment, N previous deployments (rollback), and "Recovery/Rescue" environment (see §4).
- Target cold boot: **<8 seconds** to login screen on NVMe hardware (see §9).

---

## 3. SYSTEM ARCHITECTURE

```
┌─────────────────────────────────────────────────────────────┐
│ APPLICATION LAYER                                              │
│  Flatpak Sandboxed Apps │ Distrobox Dev Containers │ Native    │
├─────────────────────────────────────────────────────────────┤
│ DESKTOP SHELL (Arbor Shell — GNOME Shell fork, see below)      │
│  Compositor (Wayland) │ Portal/XDG Desktop Portal │ Settings   │
├─────────────────────────────────────────────────────────────┤
│ SECURITY LAYER                                                 │
│  SELinux │ Firejail/Bubblewrap (Flatpak sandbox) │ Polkit      │
│  fapolicyd (execution allowlisting) │ AppArmor (fallback layer)│
├─────────────────────────────────────────────────────────────┤
│ SERVICE LAYER                                                  │
│  systemd │ PipeWire (audio/video) │ NetworkManager │ D-Bus     │
│  systemd-homed │ systemd-oomd │ udisks2 │ power-profiles-daemon│
├─────────────────────────────────────────────────────────────┤
│ GRAPHICS LAYER                                                 │
│  Wayland compositor │ Mesa (AMD/Intel) │ NVIDIA proprietary/   │
│  open kernel modules │ Vulkan-first, VA-API/hardware decode    │
├─────────────────────────────────────────────────────────────┤
│ DRIVER / HAL LAYER                                              │
│  Kernel modules (in-tree + DKMS-free where possible) │ firmware│
│  linux-firmware bundle │ fwupd for firmware updates             │
├─────────────────────────────────────────────────────────────┤
│ KERNEL LAYER — Linux (arbor-kernel), cgroups v2, namespaces     │
├─────────────────────────────────────────────────────────────┤
│ HARDWARE LAYER — x86_64 primary, aarch64 (roadmap) secondary    │
└─────────────────────────────────────────────────────────────┘
```

### Kernel Layer
As above — mainline-tracked, cgroups v2 only (no hybrid legacy), namespaces fully enabled for both container tooling (Distrobox/Podman) and app sandboxing (Bubblewrap).

### Hardware Layer
x86_64 first-class; ARM (aarch64) as a tracked secondary target from day one architecturally (build system is multi-arch from the start) even if not GA until Phase 4+ — this avoids the "bolt ARM on later" pain every distro has hit.

### Driver System
- Prefer in-tree/mainline drivers exclusively where viable; DKMS is a **last resort**, flagged clearly in hardware compatibility listings because DKMS modules are the #1 cause of atomic-update breakage industry-wide.
- `fwupd`/LVFS integrated for firmware updates (BIOS, SSD firmware, peripheral firmware) through the same update UI as OS updates — unifies the "everything updates atomically and safely" story.
- Hardware Compatibility List (HCL) published and crowd-verified, similar in spirit to ChromeOS/SteamOS's curated hardware approach, but not locked — unlisted hardware still generally works via mainline kernel, just without the "verified" badge.

### Graphics System
- **Wayland-only** (no X11 session shipped by default; XWayland present for legacy app compat). This is a 2026-appropriate default — X11 is legacy-support-only now.
- Compositor: fork/heavily-themed layer on **GNOME Shell** (see UX rationale below) using **Mutter**, with a bespoke Arbor theme, not KDE — chosen for animation/consistency control and because GNOME's extension model + portal maturity is currently ahead for a "just works" sandboxed-app story. (KDE Plasma offered as an official "Arbor KDE Edition" for power users who want deep customization — same base, different shell, similar to Fedora's spins model.)
- NVIDIA: both proprietary driver (auto-installed via curated repo, since "make NVIDIA painless" is a top-3 differentiator vs vanilla Fedora/Arch) and support for NVIDIA's open kernel modules where GPU generation supports it. Vulkan and VA-API hardware video decode verified per-GPU in the HCL.

### Audio System
**PipeWire** for audio + video + screen-capture routing (already the modern standard, replacing PulseAudio/JACK/mixing headaches). WirePlumber as session manager. Low-latency profile available for pro-audio users (toggle in Settings, not a separate OS mode).

### Networking System
NetworkManager as the control layer, with:
- **Built-in system-wide DNS-over-HTTPS/DoT** (user-selectable resolver, defaulting to a privacy-respecting provider, never silently downgrading to plaintext).
- **MAC address randomization by default** on Wi-Fi (like modern mobile OSes — a real privacy default competitors don't ship).
- Optional integrated WireGuard VPN client as a first-party Settings panel (not a third-party app requirement).
- `nftables` as the firewall backend (see §5).

### Security Layer
SELinux enforcing by default (targeted policy, Fedora-grade maturity) + Flatpak/Bubblewrap sandboxing for apps + `fapolicyd` execution allowlisting for the base system + Polkit for privilege escalation UX. Detailed in §5.

### Application Layer
Flatpak-first sandboxed GUI apps, Distrobox/Toolbx for CLI/dev environments, native RPM layering as an advanced escape hatch. Detailed in §7.

---

## 4. UPDATE SYSTEM

This is where the image-based (bootc/rpm-ostree) foundation pays for itself directly.

### Atomic Updates
- The entire OS is built centrally as a versioned OCI-style image (bootc). Update = pull new image layer + create new deployment; **the running system is never mutated in place.**
- Updates are all-or-nothing: either the new deployment boots successfully or the bootloader falls back to the last-known-good deployment automatically after N failed boot attempts (systemd-boot's boot counting feature).
- No "half-updated" states are possible by construction — a core reliability advantage over apt/dnf's transaction-based (but in-place) model.

### Snapshots
- Btrfs snapshots taken automatically: (a) before every OS image update, (b) before every layered package operation, (c) on a rolling schedule for `/home` (hourly/daily/weekly retention, Snapper-style, user-configurable retention in Settings).
- Snapshots are exposed in a GUI **Timeline** app — visually browse "what my system looked like" and restore individual files or roll back the whole system state, not just a CLI feature.

### Rollback
- One-click rollback from the boot menu (select previous deployment) or from within a booted session (`Settings → System → Rollback`, triggers reboot into prior deployment).
- Rollback is **instant** (pointer swap to prior OSTree commit + Btrfs subvolume), not a restore-from-backup operation — this is the single biggest reliability differentiator vs Windows/traditional Linux.

### Recovery Mode
- Dedicated minimal recovery environment baked into the boot menu (separate small OSTree deployment), including: network access, a terminal, Btrfs snapshot browser/restore tool, and a "repair boot entries" utility — modeled on macOS Recovery / Windows WinRE, but transparent and scriptable.

### Backup System
- First-party **Arbor Backup** app: local (external drive, Btrfs send/receive incremental) and optional encrypted cloud (user's own provider — Backblaze B2/S3/etc. via Restic under the hood, zero Arbor-run cloud service required, preserving the privacy stance).
- Full-disk image export supported for migration to new hardware.

---

## 5. SECURITY ARCHITECTURE

| Component | Design |
|---|---|
| **Secure Boot** | Arbor-signed shim + MOK enrollment flow simplified in a GUI wizard; kernel/initramfs signed as part of the image build pipeline (mkosi/bootc build chain signs artifacts in CI). |
| **TPM support** | TPM 2.0 used for: (1) LUKS key sealing (no password prompt on trusted-boot-state machines, prompts only if boot chain measurements changed — Windows Hello/BitLocker-style UX via `systemd-cryptenroll`), (2) measured boot attestation exposed to Settings as a plain-English "Boot Integrity: OK/Changed" indicator. |
| **Disk encryption** | LUKS2 full-disk encryption **on by default** for every install (opt-out, not opt-in — matches macOS FileVault default posture), TPM-backed unlock, per-user home encryption via `systemd-homed` on top for multi-user machines. |
| **Sandboxing** | All Flatpak apps run under Bubblewrap + portals; no unrestricted filesystem/network access without explicit, revocable per-app permission grants (see Privacy §6). SELinux confines system daemons regardless of Flatpak status. |
| **Permission system** | XDG Desktop Portal-mediated access to camera/mic/location/files/screen-capture, with a centralized permission dashboard (§6) — same mental model as iOS/Android permission prompts, novel for desktop Linux. |
| **Firewall** | `nftables` backend, default-deny inbound, GUI front-end ("Arbor Firewall") showing per-app outbound connection attempts with allow/block/always-ask, not just a raw port-based interface. |
| **Malware prevention** | `fapolicyd` allowlist-based execution control on the base system (only signed/known binaries execute by default outside sandboxes); Flatpak sandbox containment as primary anti-malware boundary for third-party apps; no AV suite bundled (reduces attack surface/telemetry vector) but ClamAV offered as an optional Flatpak for users who need to scan files headed to Windows machines. |

---

## 6. PRIVACY ARCHITECTURE

### Zero Telemetry
- **No telemetry transmitted by default, full stop** — no "anonymous usage statistics," no crash reporter phoning home without explicit per-incident consent (crash reports are opt-in, shown to the user for review before anything is sent, and sent to Arbor's own infrastructure, not a third party).
- Update checks use a **privacy-preserving check-in** (no unique machine ID; anonymized country/arch bucket only, similar to how Debian's popcon or Fedora's countme are designed to be non-identifying) — and this mechanism itself is documented publicly and auditable, source available.

### Permission Dashboard
- A single Settings panel: every app, every permission it currently holds (camera, mic, location, contacts-equivalent, filesystem scope, network), toggle-able live, with a history log ("Spotify accessed your microphone at 3:41 PM" style, mirroring modern mobile transparency).

### Network Privacy
- Encrypted DNS by default, MAC randomization by default, per-app network kill switch via the firewall UI, VPN-first-class integration, no ad-tech telemetry SDKs in any first-party app.

### Application Privacy
- Flatpak sandbox scoping is the enforcement mechanism — apps declare needed permissions in their manifest, users see/approve/revoke at install and any time after, not just once.
- App Store (§7) surfaces a **privacy label per app** (à la Apple's App Store nutrition labels) — network access, data collection claims, permissions requested — generated partly from manifest analysis, partly from developer self-report with community flagging for inaccuracy.

### User Control
- Full local admin/root control retained (unlike ChromeOS-style lockdown) — privacy defaults protect the non-technical user, but power users can disable any of it, because "your machine, your rules" remains core to the Linux identity Arbor keeps.

---

## 7. APPLICATION ECOSYSTEM

### Package Format
**Flatpak** as the primary distributable format for GUI apps (sandboxing, cross-distro portability, delta updates already solved upstream via OSTree-based Flatpak repos — synergizes with our OSTree base). RPM used only for system-level layered packages (drivers, low-level system extensions).

### App Store
First-party **Arbor Store**:
- Aggregates Flathub (verified subset) + Arbor's own curated/reviewed apps + optional user-enabled third-party remotes (openly, not hidden — "Enable Flathub (unverified apps)" toggle, similar to enabling unknown sources on Android, defaulting to curated-only for new users).
- Store front-end designed with real editorial curation (staff picks, categories) rather than pure algorithmic ranking — part of the "premium feel" differentiator.

### Application Verification
- Tiered trust badges: **Arbor Verified** (reviewed by Arbor security team, reproducible build verified) → **Flathub Verified** (upstream Flathub's own verification) → **Community** (unverified, sandboxed but unreviewed) — clearly labeled, never hidden, so users make informed trust decisions rather than a false binary.
- Reproducible builds pursued for all first-party Arbor apps and encouraged (tooling provided) for third-party submissions.

### Developer Publishing System
- Standard Flatpak manifest submission pipeline (same skills as Flathub — zero new learning curve for existing Linux devs).
- CI-based automatic sandboxing lint (flags apps requesting excessive permissions, feeds the privacy-label generation in §6).
- Revenue path for developers: optional "Support this developer" one-time/subscription tipping built into the Store (no Arbor revenue cut model needed initially — platform-not-marketplace posture keeps trust high).

---

## 8. HARDWARE SUPPORT

| Category | Strategy |
|---|---|
| **Intel** | Full mainline support, Intel graphics (Xe/Arc) via Mesa, power management via `thermald`/`intel_pstate`, verified HCL tier for recent (last 6 years) chipsets. |
| **AMD** | Full mainline support (AMD's upstreaming is excellent), amdgpu open driver, ROCm support tracked for compute/ML workflows as a Phase 3+ goal. |
| **NVIDIA** | Highest engineering investment here since it's Linux's historically weakest spot — auto-detected proprietary driver install at OS install time (no manual DKMS dance), open kernel module path for Turing+ GPUs, Wayland+NVIDIA explicit-sync support tracked closely with upstream Mutter/NVIDIA driver releases. |
| **Laptops** | Suspend/resume (s2idle and S3 both tested per-model in HCL), backlight control, hybrid graphics (Optimus) via `envycontrol`-style tooling integrated into Settings, fingerprint via `fprintd`. |
| **Desktops** | Multi-monitor/mixed-refresh-rate Wayland support prioritized (historically weak area, actively improving upstream), full RGB/peripheral ecosystem via OpenRGB integration. |
| **Touchscreen** | Wayland touch + gesture support (GNOME's touch gestures maturing well), on-screen keyboard for convertible/tablet form factors. |
| **Fingerprint** | `fprintd` + PAM integration for login/sudo/polkit prompts, TPM-backed where hardware supports it. |
| **Bluetooth** | BlueZ stack, unified with the same Settings/permission model as other radios. |
| **WiFi** | Mainline driver preference, MAC randomization default (§6), WPA3 support verified per-chipset in HCL. |

---

## 9. PERFORMANCE STRATEGY

### Boot Optimization
- Target: **<8s cold boot to login** on NVMe/modern hardware; `systemd-analyze` regression-tested in CI per image build (any commit that regresses boot time by >5% blocks release).
- Parallelized service startup (systemd default), minimal enabled-by-default service set, no legacy init scripts.

### RAM Usage Goals
- Idle desktop session target: **≤1.2GB** RAM at login with default app set (comparable to a lean GNOME/KDE session, meaningfully better than default Windows 11 idle footprint).
- `systemd-oomd` tuned aggressively enough to prevent full-system stalls under memory pressure (a real, common Linux desktop pain point) without being trigger-happy on legitimate heavy workloads (cgroup-scoped pressure thresholds, not global).

### Battery Optimization
- `power-profiles-daemon` (Balanced/Performance/Power-Saver) surfaced prominently in the shell (menu-bar quick toggle, like macOS), TLP-equivalent tuning baked into the power-saver profile, PCIe ASPM and USB autosuspend tuned per-HCL-entry (not blind global defaults that break random hardware).
- Target: match or beat vanilla Fedora Workstation battery life on identical hardware (baseline), with a longer-term goal of closing further gap to macOS on supported laptop lines.

### GPU Acceleration
- Vulkan-first rendering path for the compositor and supported apps; VA-API hardware video decode/encode wired through by default for browsers/media apps (real day-one battery and thermal win).
- Gaming: **Steam + Proton first-class**, HDR/VRR support tracked per current kernel/Mutter capability, GameMode integration for automatic performance-profile switching during play.

---

## 10. DEVELOPMENT ROADMAP

### Phase 1 — Foundation (Months 0–6)
**Goal:** Bootable, atomic, reproducible base image.
- Milestones: bootc/rpm-ostree image pipeline stood up in CI; base image boots on reference hardware set; Btrfs snapshot/rollback proven end-to-end; disk encryption + TPM unlock working; HCL v0 (top 20 reference machines) validated.
- Risks: OSTree/bootc tooling still maturing upstream → **Mitigation:** contribute upstream fixes rather than fork; stay close to Fedora's own Atomic Desktops roadmap.
- Risks: NVIDIA driver auto-install fragility → **Mitigation:** dedicated NVIDIA QA lane, explicit fallback to nouveau/open modules if proprietary install fails, never brick boot.

### Phase 2 — Desktop (Months 6–14)
**Goal:** Daily-drivable shell experience.
- Milestones: Arbor Shell (GNOME Shell fork/theme) feature-complete; Settings app with unified permission dashboard, firewall UI, power profile UI; systemd-homed multi-user flow; Recovery Mode shipped.
- Risks: Custom shell fork drifting from upstream GNOME, causing maintenance burden → **Mitigation:** extension-based approach wherever possible instead of hard forking Mutter/Shell internals; only fork where extensions genuinely can't achieve the design.

### Phase 3 — Applications (Months 12–20, overlapping Phase 2 tail)
**Goal:** App ecosystem viable for daily use.
- Milestones: Arbor Store GA with tiered verification; Flathub curated-subset integration; Distrobox/Toolbx dev workflow polished; first-party apps (Files, Backup, Timeline/snapshots, Settings, Store) reach 1.0.
- Risks: Chicken-and-egg app availability problem → **Mitigation:** Flatpak/Flathub compatibility means Arbor launches with the *entire* existing Flathub catalog available day one — this isn't a "wait for developers to port" problem like a brand-new OS would face.

### Phase 4 — Security Hardening (Months 18–26, overlapping)
**Goal:** Security posture ready for professional/enterprise trust.
- Milestones: SELinux policy audit completed; `fapolicyd` allowlist tuned to avoid false-positive breakage; hardened kernel variant GA; third-party security audit commissioned and published; Secure Boot chain fully signed/notarized release pipeline.
- Risks: SELinux/fapolicyd false positives breaking real user workflows (historically why many distros ship SELinux permissive) → **Mitigation:** extensive beta/dogfooding period specifically stress-testing policy against the full curated app catalog before flipping to enforcing-by-default at GA.

### Phase 5 — Polishing (Months 24–30)
**Goal:** Premium-feel 1.0 release.
- Milestones: performance targets (§9) met and CI-gated; full HCL expanded to 200+ verified machines; accessibility audit (screen reader, contrast, motion-reduction) passed; localization for top 15 languages; public 1.0 launch.
- Risks: scope creep delaying GA → **Mitigation:** hard feature-freeze date with a published "post-1.0 roadmap" for anything cut, so deferred features don't become silent scope creep.

---

## Summary Table: Key Technical Decisions

| Decision | Choice |
|---|---|
| Base | Fedora Atomic (bootc/rpm-ostree) |
| Kernel | Near-mainline + optional hardened variant |
| Init | systemd (homed, oomd, boot) |
| Filesystem | Btrfs (root+home), ZFS optional server tier |
| App format | Flatpak-first, RPM layering for system extensions |
| Shell | GNOME Shell fork ("Arbor Shell"), KDE spin offered |
| Display | Wayland-only, XWayland for legacy |
| Audio | PipeWire |
| Firewall | nftables + GUI |
| Encryption | LUKS2 on by default, TPM-backed |
| Update model | Atomic image + Btrfs snapshots + instant rollback |
| Telemetry | Zero by default, opt-in only, self-hosted if enabled |

---

*This document is a starting architecture, not a locked spec — every decision above should be treated as "best current answer, open to revision" once real hardware testing and community feedback start Phase 1.*
