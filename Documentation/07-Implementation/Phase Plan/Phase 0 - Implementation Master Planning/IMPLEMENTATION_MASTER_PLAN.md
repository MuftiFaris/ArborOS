# ARBOR OS — IMPLEMENTATION MASTER PLAN

**Derived from:** `FINAL_OS_SPECIFICATION_V2.md` (v2.0, Canonical)
**Document purpose:** Translate the product/architecture specification into an actionable engineering execution plan — team structure, build order, tooling, dependencies, and test strategy. This document does not alter any ruling made in the specification; where the spec's 5-phase roadmap (§9) is reorganized below into 7 execution phases, this is a scheduling/work-breakdown convenience only, mapped explicitly in §3.

---

## 1. PROJECT OVERVIEW

### 1.1 What Arbor OS Is

Arbor OS is a privacy-first, performance-focused Linux desktop distribution built on a **Fedora Atomic** foundation (`rpm-ostree`/`bootc`, image-based, immutable root). Rather than building new low-level technology, Arbor's engineering thesis is integration: taking Linux subsystems that already lead the field on a technical basis — cgroups v2, Wayland, PipeWire, systemd, Btrfs, SELinux — and making the thousands of small, consistent product decisions that turn a technically excellent stack into a polished, opinionated consumer/prosumer OS. The identity metaphor is a tree: an immutable trunk (the base image) with customizable, prunable branches (layered apps, containers, user configuration) that never threaten the trunk's integrity.

Tagline: *"Change anything. Break nothing."*

### 1.2 Target Users (in priority order)

1. **Privacy-conscious professionals** (journalists, lawyers, researchers) — need a polished OS with zero telemetry.
2. **Developers** — need a Unix environment without driver hell or environment contamination.
3. **Prosumers/creatives** — need a stable, color-managed audio/video/photo pipeline.
4. **Gamers** — need SteamOS-like performance on a general-purpose desktop.
5. **Windows 11 switchers** — need something that "just works" without a terminal.

### 1.3 Main Engineering Goals

- **Immutable core, mutable edges:** read-only, atomically-updated base image; all user/app state layered, sandboxed, or containerized.
- **Zero-telemetry privacy by default** — no opt-out dark patterns, no mandatory account.
- **Reversibility of every destructive action** — updates, package installs, and config changes are all snapshot-backed with single-digit-second rollback.
- **CI-gated performance parity or advantage** vs. Windows 11/macOS on boot time, idle RAM, frame time, and battery life, without weakening security or privacy to hit the numbers.
- **A curated, single-opinion product** rather than a distro that ships every option "for choice."
- **A real developer ecosystem** (Distrobox, Homebrew, `mise`, Arbor Store publishing pipeline) so Arbor is a legitimate daily driver for engineers, not just end users.

This plan treats the specification as frozen scope. Engineering work references spec section numbers (e.g. `§3.9`) throughout so implementers can trace every decision back to its rationale and the Conflict Resolution (§2 of the spec) that produced it.

---

## 2. TECHNOLOGY STACK

Each choice below is taken directly from the spec's Key Technical Decisions table (§10.1) and the relevant architecture section, with the engineering justification restated for the implementation team.

### 2.1 Linux Base

**Fedora Atomic (rpm-ostree / bootc image-based)** — selected over Arch (no image-based update story, rolling release conflicts with a stable/premium product), Ubuntu LTS (Snap telemetry conflicts with the privacy-first identity), and a from-scratch LFS-style build (unjustifiable maintenance cost). Fedora Atomic supplies RHEL-grade SELinux tooling, a field-proven OSTree atomic/rollback model (validated in production by Silverblue, Universal Blue, Bazzite, Aurora), and a fast upstream kernel/driver cadence. **Debian is referenced only as a QA-philosophy influence** (unstable→testing→stable promotion gates) — there is no Debian technology anywhere in the Arbor build system; all tooling is Fedora/RPM/OSTree-based (spec Conflict 4).

### 2.2 Kernel Strategy

Two switchable kernel channels, both shipped as part of atomic image updates (never live-patched):

- **`arbor-kernel`** (default): near-mainline Fedora kernel, full desktop preemption, `CONFIG_HZ=1000`, EEVDF scheduler as the resting-state fallback with `scx_rustland` loaded as the session-default scheduler via sched-ext (BPF), `scx_lavd` swapped in under GameMode, `scx_bpfland` under Battery Saver.
- **`arbor-kernel-hardened`**: same base, `CONFIG_LOCKDOWN_LSM` confidentiality mode, `kernel.kptr_restrict=2`, `yama.ptrace_scope=2`, unprivileged BPF disabled by default, enforced module signing.
- An **LTS fallback kernel** is always retained as a boot entry regardless of active channel (zero-cost regression safety under the atomic deployment model).
- Build system is **multi-arch from day one** (x86_64 GA; aarch64 evaluated starting Phase 4/roadmap-published at GA) even though only x86_64 ships initially.

### 2.3 Package / Update System

- **rpm-ostree layering** for system-level extensions (drivers, kernel modules) — advanced, rollback-safe, clearly labeled "modifies the system image" in the UX.
- **Flatpak** as the primary end-user application format (sandboxed, portable, delta-updated via OSTree).
- **Update delivery tiers:** `Development → Canary → Beta → Stable`, each an OSTree ref, promotion gated by automated tests plus human review (replaces the ambiguous "Debian-style QA" language from v1.0 — spec Conflict 4).
- Atomic update flow: signed OCI/OSTree artifact built in CI → signature verified client-side → new deployment created alongside the current one → reboot offered → boot-counted rollback on failure. The running system is never mutated in place.

### 2.4 Build Tools

- **CI-driven image build pipeline** producing signed OCI/OSTree artifacts (no on-device signing, ever).
- **`bootc`** as the primary image-build/deploy tool, with `rpm-ostree` for compatibility and layering operations.
- **Reproducible build infrastructure** for both the base image and all first-party Arbor apps — any contributor can rebuild from source and get a bit-identical artifact (supports the Arbor Verified trust tier, §7.7 of the spec).
- **QEMU-based CI harness** for boot-time, idle-RAM, and idle-CPU CI gates (`systemd-analyze`, `smem`, `pidstat`).

### 2.5 Desktop Technologies

- **Arbor Shell = GNOME Shell + Arbor Extension API v1 + Arbor libadwaita theme + dconf/GSettings configuration.** No fork of `gnome-shell`, `mutter`, or `libadwaita` C source is created or maintained (spec Conflict 7) — this is a hard architectural constraint, not a preference, because a true fork would create a permanent, team-size-scaling maintenance burden against upstream security patches.
- **Compositor:** Mutter, Wayland-only session (no default X11 session; XWayland present for legacy app compatibility only).
- **Audio:** PipeWire + WirePlumber (replaces PulseAudio/JACK).
- **First-party apps** (Arbor Files, Arbor Terminal, Arbor Settings/Control Center, Arbor Backup, Arbor Timeline, Arbor Store, Arbor Container Studio) built against the same Extension API v1 / libadwaita design system.

### 2.6 Installer Technologies

- **Boot chain:** UEFI Secure Boot → Arbor-signed shim (Microsoft 3rd-party CA signed) → Arbor-signed `systemd-boot` → signed kernel/initramfs, with PCR values sealed to TPM 2.0 for LUKS2 key release. GRUB ships only as a legacy-BIOS fallback, explicitly labeled "Secure Boot unavailable."
- **Filesystem layout at install time:** Btrfs for `/`, `/home`, `/var`, `/.snapshots` (all OSTree/Snapper-managed subvolumes); **FAT32/vfat for the ESP** — this is a firmware constraint (UEFI can only read FAT12/16/32 from the EFI System Partition) and must never be implemented as ext4 (spec Conflict 6 / changelog item 1).
- **LUKS2 full-disk encryption on by default at install time**, TPM-backed with recovery passphrase generation; the installer refuses to silently skip the recovery-passphrase step.
- **First-boot onboarding wizard** (6 linear steps: language/region → accessibility → account → encryption → privacy → theme) with no cloud account requirement anywhere in the flow, and IBus input-method auto-configuration tied to the selected language (spec changelog item 8).

---

## 3. DEVELOPMENT PHASES

The specification's canonical roadmap (§9) defines five phases (Foundation, Desktop, Applications, Security Hardening, Polishing/Release) over a ~30-month timeline. To match the deliverable structure requested for this plan, that roadmap is broken into **seven execution phases** below. This is a work-breakdown reorganization only — every objective, dependency, and risk is carried over unchanged from the spec; no scope is added or removed. The mapping is:

| This plan's phase | Spec phase(s) it draws from |
|---|---|
| 1. Foundation | Spec Phase 1 (image, boot, encryption, HCL v0) |
| 2. Base System | Spec Phase 1 tail + Phase 2 head (init system, filesystem, update tiers, security baseline) |
| 3. Hardware Support | Spec Phase 1 (HCL v0) → Phase 4/5 (HCL expansion to 200+ machines, ARM evaluation) |
| 4. Desktop Environment | Spec Phase 2 (Arbor Shell, Settings, Terminal, Developer Mode) |
| 5. Applications | Spec Phase 3 (Arbor Store, first-party apps, developer publishing pipeline) |
| 6. Optimization | Spec Phase 4 security hardening + the performance-tuning work embedded throughout §6 |
| 7. Testing | Spec Phase 5 (accessibility/localization/HCL gates) + the CI gates defined throughout §6/§10.2, executed continuously but formally exit-gated here |

### Phase 1 — Foundation (Months 0–6)

**Goal:** A bootable, atomic, reproducible base image with proven rollback and encryption — internal-team-only, no shell.

Objectives:
- Stand up the `bootc`/`rpm-ostree` CI image-build pipeline.
- Boot the base image on the initial reference hardware set.
- Validate Btrfs snapshot/rollback end-to-end (pre-update, pre-layering, and rolling `/home` snapshots).
- Working LUKS2 + TPM 2.0 unlock.
- Complete Secure Boot chain (shim → `systemd-boot` → signed kernel).
- Base git + Podman + Distrobox + Homebrew pre-installed in the image.
- CI boot-time regression gate established (< 8.0 s, block-release).

Key dependency: upstream `bootc` stability; an NVIDIA driver auto-install QA lane must exist before this phase can be called complete, since NVIDIA support is the highest-investment graphics area in the spec (§3.7).

Exit criteria: an internal engineer can install, boot, update, and roll back the image unattended on the top 20 reference machines.

### Phase 2 — Base System (Months 4–8, overlapping Phase 1 tail)

**Goal:** All non-shell system services, the security baseline, and the update-tier model are in place and CI-verifiable.

Objectives:
- `systemd` service stack: `systemd-homed`, `systemd-oomd`, `systemd-boot`, `dbus-broker`, `power-profiles-daemon`, `udisks2`.
- `IBus` installed and wired to the onboarding language selection (base-system infrastructure, not optional — required for the Phase 5/7 CJK/Indic localization commitment).
- Filesystem baseline finalized: Btrfs mount options (`noatime,compress=zstd:3,space_cache=v2,discard=async`) applied system-wide; FAT32 ESP confirmed in every build target.
- Update delivery tiers (`Development → Canary → Beta → Stable`) implemented as OSTree refs with promotion gating.
- Snapshot disk-space safeguard: 15% free-space floor with oldest-first automatic pruning of non-pinned snapshots (spec changelog item 9).
- Baseline security layer: SELinux targeted policy (permissive initially, enforcing deferred to Phase 6 per spec Phase 4 risk mitigation), `fapolicyd` allowlist skeleton, `nftables` default-deny inbound firewall, encrypted DNS by default with visible degradation state.
- Kernel channel split (`arbor-kernel` / `arbor-kernel-hardened`) buildable and switchable, with LTS fallback entry always present.

Exit criteria: a fresh install has working accounts, encryption, networking, firewall, and update channel switching, with SELinux running in a monitored (not yet fully enforcing) mode.

### Phase 3 — Hardware Support (Months 2–26, continuous; gated milestones at Month 6 and Month 26)

**Goal:** A Hardware Compatibility List (HCL) that grows from a 20-machine internal set to 200+ verified machines by GA, without ever silently degrading on unsupported hardware.

Objectives:
- **HCL v0 (Month 6, exits with Phase 1):** top 20 reference machines validated for boot, encryption, and basic display.
- Intel/AMD GPU support verified (Mesa Xe/Arc, `amdgpu`) — both are "last 6 years, Verified" tier.
- NVIDIA support: auto-detected proprietary driver install at OS-install time via a curated repo (no manual DKMS), open kernel module path for Turing+ GPUs, and hard fallback to `nouveau`/open modules if proprietary install fails — the system must never brick boot on an NVIDIA machine. This is the single highest hardware-engineering investment area per the spec and needs its own dedicated QA lane, not a shared graphics lane.
- Wi-Fi (WPA3 verified per-chipset), Bluetooth (BlueZ), fingerprint (`fprintd`+PAM), touchscreen/convertible (GNOME Wayland gestures), hybrid graphics laptops (`envycontrol`-style toggle), and suspend/resume (both s2idle and S3) added to the HCL per model.
- Camera HCL coverage for both UVC (standard webcams) and MIPI-CSI/IPU stacks (e.g. Intel IPU6 via `libcamera`), tiered "Limited" where upstream driver maturity is incomplete — this row exists specifically because the Privacy Indicator in the top panel depends on reliable camera-in-use detection (spec changelog item 7).
- Printers/Scanners: CUPS driverless-first (IPP/eSCL) with per-model fallback drivers, SANE for scanning, surfaced as a real Settings panel (not just an AppArmor exception entry — spec changelog item 13).
- Minimum/Recommended/Reference hardware tiers published and used as the actual QA floor: 8 GB/64 GB NVMe minimum (Standard Mode only, Developer Mode not certified at this tier), 16 GB/256 GB NVMe recommended (where Developer Mode, gaming, and creative workflows are certified), 32 GB reference bench (where all published benchmark numbers come from).
- **aarch64 (ARM):** build system is multi-arch from day one; the actual port begins evaluation in Phase 4/6 of this plan (Months 18+) with performance-parity targets set once profiling data exists — this remains an explicitly open post-1.0 item, not a GA blocker.
- **HCL expansion to 200+ verified machines** as a GA exit gate (Month 26–30, folded into Phase 7 of this plan).

Exit criteria (GA): 200+ verified machines across the categories above, with every DKMS-only driver clearly flagged "Limited — may break on kernel updates" and no unflagged silent-degradation cases.

### Phase 4 — Desktop Environment (Months 6–14)

**Goal:** A daily-drivable Arbor Shell experience for technical users, sufficient to begin developer community engagement.

Objectives:
- Arbor Shell feature-complete: GNOME Shell + Extension API v1 (the stable, versioned wrapper around Shell internals — the isolation layer that absorbs upstream GNOME churn) + Arbor libadwaita theme + dconf/GSettings overrides.
- Arbor Control Center (Settings) built out per the two-pane layout in the spec: Network, Bluetooth, Display, Sound, Printers & Scanners, Input Sources (IBus), Privacy Dashboard, Security & Lock, Arbor Shield Firewall, Updates & Rollback, Appearance, Applications, Accounts, Developer Mode.
- Session-start sound implemented as a `systemd --user` oneshot unit firing within 500 ms of session readiness — **not** a Plymouth boot splash; Plymouth remains disabled for the boot-time budget (spec Conflict 3). This distinction matters operationally: the audio team must not attempt to route this through Plymouth.
- Top panel (Privacy Indicator hardware-truthful via kernel device-node open checks), Smart Dock, window tiling ("Arbor Snap"), workspace system, and the full touchpad gesture set.
- Arbor Terminal 1.0 (GPU-accelerated `wgpu`/Vulkan renderer, VTE-ng backend, Kitty protocol, Fish default shell with Starship prompt, `atuin` encrypted history).
- `systemd-homed` multi-user flow validated end-to-end, including failure-mode recovery.
- Recovery Mode shipped as a dedicated signed Recovery OSTree deployment in the boot menu, plus a GUI wizard covering rollback/file-restore/password-reset without requiring Btrfs CLI knowledge.
- Developer Mode toggle (firewall rules for dev ports on loopback, `SSH_AUTH_SOCK` auto-wired to GNOME Keyring, dev audit log, First-Run Wizard) and VS Code/JetBrains Flatpaks verified in the HCL.
- CI gates activated: idle RAM (≤1.2 GB Standard / ≤1.6 GB Developer, both with the documented +200 MB CI-block tolerance buffer) and compositor frame time (<4 ms @1080p).

Key risk owned in this phase: NVIDIA Wayland explicit-sync depends on coordinated NVIDIA driver + Mutter releases and is re-validated on every release of either — this needs a dedicated QA lane gating promotion, with fallback to open modules/`nouveau` if a regression can't be resolved before a release deadline (spec §3.12).

Exit criteria: an internal/early-access user can use Arbor Shell as a daily driver with working developer tooling.

### Phase 5 — Applications (Months 12–20, overlapping Phase 4 tail)

**Goal:** An app ecosystem viable across the full target-user range, ready for public beta.

Objectives:
- Arbor Store GA with the three-tier trust model (Arbor Verified / Flathub Verified / Community, opt-in toggle for the unreviewed tier).
- Flathub catalog integration — the entire existing Flatpak catalog is available from day one, avoiding the chicken-and-egg app-availability problem that sinks most new Linux distros.
- Developer publishing pipeline: submission, review, update, Homebrew formula generation; `arbor devtools` CLI GA; app scaffolding.
- CI-based sandbox lint on all Store submissions (flags apps requesting permissions inconsistent with stated function), feeding both auto-generated privacy labels and the manual Arbor Verified review queue.
- First-party apps reach 1.0: Arbor Files, Arbor Backup, Arbor Timeline, Arbor Store, Arbor Terminal, Arbor Settings.
- Arbor Container Studio GA (Podman-backed Docker Desktop alternative: image browser/pull, log viewer, volume manager, Compose editor).
- `k3d`-based local Kubernetes (`kubectl`/`helm`/`k9s`/`stern` via `arbor devtools k8s`) reaches CLI-usable state (GA deferred to Phase 7 of this plan).

Exit criteria: public beta launch with a full, real app catalog and an open external-developer onboarding path.

### Phase 6 — Optimization (Months 18–26, overlapping Phase 5 tail)

**Goal:** Performance targets locked in under CI gates, and the security posture hardened to a level suitable for professional/enterprise trust — without regressing privacy or security to chase benchmark numbers (this ordering constraint is explicit in the spec §6.1 and must not be reversed by schedule pressure).

Security-hardening objectives (carried from spec Phase 4):
- Flip SELinux from monitored to **enforcing by default**, only after an extensive policy audit and dogfooding pass against the full curated app catalog — the false-positive rate against real workflows is the gating metric, not a calendar date.
- Tune the `fapolicyd` allowlist to eliminate false-positive breakage on the verified catalog.
- `arbor-kernel-hardened` reaches GA (Secure Workstation Mode stable and toggleable in Settings); default kernel stays gaming-friendly — hardened mode is strictly opt-in so it never causes driver/gaming regressions for users who didn't ask for it.
- Commission and publish a third-party security audit — engaged at the *end* of Phase 5 so findings land early enough in this phase to actually be fixed pre-GA, not discovered after.
- Formalize and enforce the dual-MAC boundary: SELinux governs all Arbor-maintained daemons and kernel interfaces; AppArmor is scoped only to the three explicitly-listed third-party cases (Chromium GPU sandbox, CUPS, SANE) with no new AppArmor policy authored for Arbor components (spec Conflict 9).
- SELinux Developer Mode policy profile for Distrobox `unconfined_u` containers, with the explicit list of controls that remain active around those containers documented (spec changelog item 5) — this is the bounded exception, not an open-ended one.
- Extension API v1 security review completed.

Performance-optimization objectives (drawn from spec §6 throughout, formally locked in this phase):
- Kernel build flags and sysctl tuning finalized (`CONFIG_HZ=1000`, zswap/LZ4, `vm.swappiness=10`, BBR congestion control, etc.).
- `scx_rustland`/`scx_lavd`/`scx_bpfland` scheduler policies validated per-workload (general desktop / gaming / battery saver), with automatic, silent fallback to EEVDF if SCX loading fails on a given kernel build — re-validated on every `arbor-kernel` point release (spec §3.12).
- `systemd-oomd` and per-Flatpak cgroup memory accounting (`MemoryHigh=75%`/`MemoryMax=90%`) tuned so user apps survive over system services under memory pressure.
- I/O scheduler policy applied per device type via udev (`kyber` for NVMe, `bfq` for HDD/eMMC).
- GameMode integration (automatic performance-profile switch, `scx_lavd`, CPU governor switch) validated against the two-tier gaming target: <6% average FPS overhead for general titles, <12% for anti-cheat-dependent Proton titles (explicitly documented as a Proton/Wine-inherent cost, not an Arbor deficit — spec Conflict 5).
- Battery-life target (+10–15% vs. equivalent Fedora on the same hardware) validated via the web-browsing-simulation benchmark.
- ARM (aarch64) port evaluation begins in this phase, per the spec's Phase 4 alignment — targets remain deferred until profiling data exists.

Exit criteria: SELinux enforcing in production with an acceptable false-positive rate, hardened kernel GA, published third-party audit with findings addressed, and all CI-gated performance metrics from §6.2 passing on the reference benchmark hardware.

### Phase 7 — Testing (Months 24–30)

**Goal:** Premium-feel, fully-gated 1.0 public release.

Objectives:
- All CI-gated performance targets re-verified as release-blocking gates (boot time, idle RAM both modes, compositor frame time, gaming overhead) — see §6 of this plan for the full test matrix.
- HCL expanded to 200+ verified machines (final exit of the Phase 3 hardware-support track).
- Accessibility audit as a hard GA gate: WCAG 2.1 AA contrast (any failure blocks GA), full screen-reader pass, motion-reduction verification. Accessibility testing begins in the Phase 5/Beta window, not deferred to this phase, so gaps surface early.
- Localization to the top 15 languages, with machine translation treated as a floor (not a ceiling) and a community translation program that started back in Phase 5.
- Developer documentation site 1.0, Arbor Pages static hosting, `arbor devtools k8s` GA.
- Hard feature-freeze date enforced; any descoped feature goes onto a published post-1.0 roadmap released simultaneously with 1.0, to contain scope-creep expectations rather than let it silently slip the date.
- ARM (aarch64) roadmap published (not shipped) at GA.

Exit criteria: 1.0 public release with full documentation, verified 200+-machine HCL, and every CI performance/accessibility/security gate passed — detailed in §6 (Testing Strategy) below.

---

## 4. DEPENDENCIES

### 4.1 Required Software / Base Components

- **Fedora Atomic** base image tooling: `rpm-ostree`, `bootc`, OSTree.
- **systemd** family: `systemd-homed`, `systemd-oomd`, `systemd-boot`, `systemd-analyze` (CI), `dbus-broker`.
- **Btrfs** + `Snapper` (snapshot management), `zstd` (compression).
- **SELinux** (targeted policy, `refpolicy`-based) + `fapolicyd`.
- **AppArmor** (scoped strictly to Chromium GPU sandbox, CUPS, SANE profiles — no new Arbor policy).
- **Wayland**, **Mutter**, **GNOME Shell**, **XWayland** (compat only).
- **PipeWire** + **WirePlumber**.
- **NetworkManager**, **nftables**, **WireGuard**.
- **Flatpak** + **Bubblewrap** + **XDG Desktop Portals**.
- **Podman** (rootless default) + **Distrobox** + **Toolbx** (Flatpak-installable, unofficial path).
- **LUKS2** + TPM 2.0 stack (`tpm2-tools`, `clevis` or equivalent for TPM-sealed unlock).
- **IBus** input-method framework (Pinyin, Mozc, Hangul, and other engines as needed).
- **CUPS** + **SANE** (driverless-first printing/scanning).
- **fwupd** / LVFS (firmware updates).
- **mise**, **uv**, **ruff**, **mypy**, **pnpm** (developer tooling stack).
- **k3d**, `kubectl`, `helm`, `k9s`, `stern` (local Kubernetes).

### 4.2 Programming Languages

- **Rust** — Arbor Terminal renderer (`wgpu`), performance-sensitive first-party tooling, and any GNOME Shell Extension API v1 components that benefit from memory-safety guarantees.
- **C/Vala/JS (GJS)** — GNOME Shell extensions (Extension API v1 is built on GNOME's native extension model).
- **Python** — build/CI tooling, `arbor devtools` CLI, image-build automation scripts.
- **Shell (Bash/Fish)** — system scripts, packaging glue, install-time hooks.
- **TOML** — Arbor Terminal configuration format (declarative, diffable).

### 4.3 Build Tools

- CI pipeline capable of producing **signed OCI/OSTree artifacts** (no on-device signing).
- **QEMU** harness for automated boot-time/RAM/CPU benchmark gates.
- **Reproducible-build tooling** for both the OS image and first-party apps (bit-identical rebuild verification).
- Flatpak manifest tooling + a **CI sandbox-permission linter** for Arbor Store submissions.
- Container image tooling (Podman/Buildah) for Distrobox image maintenance and Arbor Container Studio.

### 4.4 Development Environment

- Fedora Atomic (or a close derivative) as the reference dev environment for OS engineers, ideally dogfooding Arbor itself once Phase 2 is daily-drivable.
- Access to a **hardware lab** covering the Minimum/Recommended/Reference tiers (§3.10 of the spec) plus NVIDIA, Intel, and AMD GPU variants, laptop hybrid-graphics models, and MIPI-CSI camera hardware for HCL validation.
- TPM 2.0-equipped test hardware for encryption/Secure-Boot validation.
- A dedicated **NVIDIA QA lane** and a dedicated **Wayland/Mutter explicit-sync QA lane**, run separately from general hardware QA given their release-cadence risk (spec §3.12).

---

## 5. REPOSITORY STRATEGY

### 5.1 Folder Structure (proposed)

```
arbor-os/
├── image/                  # bootc/rpm-ostree image build definitions (Containerfile-based)
│   ├── base/                # trunk: kernel, systemd, filesystem layout, security baseline
│   ├── kernel-hardened/      # arbor-kernel-hardened variant overlay
│   └── variants/             # KDE spin, future ARM target, etc.
├── shell/                  # Arbor Shell: Extension API v1, first-party extensions, theme
│   ├── extension-api-v1/
│   ├── extensions/
│   └── theme/                # libadwaita theme, icon set, wallpaper assets
├── apps/                   # first-party applications, one subfolder per app
│   ├── files/
│   ├── terminal/
│   ├── settings/
│   ├── backup/
│   ├── timeline/
│   ├── store/
│   └── container-studio/
├── devtools/                # arbor devtools CLI, SDK manager, scaffolding templates
├── store-pipeline/          # Arbor Store submission/review/lint automation
├── hcl/                     # Hardware Compatibility List data + per-model test results
├── ci/                      # CI pipeline definitions, QEMU benchmark harness, gate configs
├── docs/                    # developer documentation site source (built to Arbor Pages)
└── recovery/                # Recovery OSTree deployment definition + GUI wizard
```

### 5.2 Source Management

- **Monorepo-per-domain** rather than a single mega-repo: the image build, shell/extensions, first-party apps, and devtools each live in their own repository with independent CI, but are versioned together against a shared Arbor release train (Development → Canary → Beta → Stable) so a given Stable release has a reproducible, pinned set of component versions.
- **Upstream-first contribution model:** fixes to `bootc`, GNOME Shell, Mutter, or other upstream projects are contributed upstream rather than patched locally wherever feasible — this is explicitly the risk mitigation for avoiding "custom shell fork drifting from upstream" (spec Conflict 7 / Phase 2 risk).
- **Explicit upstream-tracking ownership** for the three fastest-moving, highest compatibility-break-risk dependencies (spec §3.12), each with a named review cadence:
  - sched-ext (SCX) BPF schedulers — re-validated on every `arbor-kernel` point release.
  - GNOME Shell Extension API v1 wrapping — re-validated on every GNOME Shell minor bump, before Canary→Beta promotion.
  - NVIDIA Wayland explicit-sync — re-validated on every NVIDIA driver release and every Mutter minor bump.
- Signed commits / signed tags for anything that feeds the image-build pipeline, consistent with the multi-layer signature chain requirement (Secure Boot → OSTree commit → Flatpak repo → layered RPM → firmware).

### 5.3 Documentation Management

- A single **developer documentation site** (Arbor Pages, static-hosted) is the canonical external-facing doc source, built from the `docs/` repo, reaching 1.0 in Phase 7/Phase 5-of-spec.
- Internal engineering documentation (this plan, the spec, and future amendments) lives alongside `image/` in version control so architecture decisions and their rationale never drift out of sync with the code that implements them.
- Per the spec's own governance note: amendments to the underlying specification require Core OS Team review and must be reflected in the spec document before taking effect in implementation — this plan should be treated the same way. Any deviation discovered during implementation should be resolved by amending the spec first, not by silently diverging in code.

---

## 6. TESTING STRATEGY

### 6.1 Automated Testing (CI-gated, continuous from Phase 1 onward)

| Metric | Target | Gate type | Tooling |
|---|---|---|---|
| Cold boot to login (NVMe) | < 8.0 s | Block release | `systemd-analyze` in QEMU |
| Idle RAM — Standard Mode | ≤ 1.2 GB PSS (block at ≥1.4 GB) | Block release | `smem` in QEMU |
| Idle RAM — Developer Mode | ≤ 1.6 GB PSS (block at ≥1.8 GB) | Block release | `smem` in QEMU |
| Compositor frame time (1080p) | < 4 ms avg | Block release | Mutter frame log |
| Idle CPU | < 1.5% avg | Warning + review | `pidstat` 60s window |
| Gaming FPS overhead (general titles) | < 6% avg vs. Windows 11 | Warning + review | Benchmark suite average |
| Gaming FPS overhead (anti-cheat/Proton titles) | < 12% avg vs. Windows 11 | Informational | Benchmark suite average |
| Kernel compile regression | < 10% vs. baseline | Warning + review | `time make -j16` |
| SELinux false-positive rate | Team-reviewed threshold | Block Phase 4→5 (spec) promotion | Dogfooding + policy audit |
| WCAG 2.1 AA contrast | Zero failures | Block 1.0 GA | Automated contrast checker + manual audit |
| Screen-reader pass | Zero regressions | Block 1.0 GA | Manual + automated accessibility test pass |

The +200 MB tolerance between the marketed idle-RAM *target* and the CI *block* threshold is a deliberate measurement-noise buffer, not a looser real target — a build landing in that band ships but triggers a mandatory PR review comment.

Every OSTree promotion (Development→Canary→Canary→Beta→Beta→Stable) re-runs this full gate suite; a regression blocks promotion regardless of which tier it's caught in.

### 6.2 Hardware Testing

- **HCL validation lab** covering the Minimum (8 GB/64 GB/4-core), Recommended (16 GB/256 GB/6-core+), and Reference (32 GB/NVMe/Ryzen 7 7840U) tiers, plus dedicated Intel, AMD, and NVIDIA GPU test rigs.
- **Suspend/resume matrix** per laptop model (both s2idle and S3 paths tested, not just one).
- **Dedicated NVIDIA QA lane**: proprietary-driver auto-install, open-kernel-module path for Turing+, and confirmed fallback-to-nouveau-without-boot-failure, re-run on every NVIDIA driver release.
- **Dedicated Wayland/Mutter explicit-sync QA lane**, re-run on every Mutter minor version bump, gating promotion independently of the general graphics lane.
- **Camera HCL testing** across both UVC and MIPI-CSI/IPU stacks, since the Privacy Indicator's correctness depends on reliable camera-in-use detection.
- **Printer/scanner testing**: driverless IPP/eSCL auto-discovery as the primary path, per-model fallback-driver testing as the secondary path.
- HCL entries are published with explicit tiering (Verified / Supported / Limited — DKMS-only hardware always flagged "Limited — may break on kernel updates"), growing from 20 machines (Phase 1 exit) to 200+ (GA exit).

### 6.3 Release Testing

- **Boot-counting rollback verification**: every release candidate must demonstrate that N consecutive boot failures trigger automatic fallback to the previous deployment, with no manual intervention required.
- **Signature-chain verification testing**: a release with a broken/missing signature at any layer (Secure Boot, OSTree commit, Flatpak repo, layered RPM, firmware) must fail to install/deploy rather than falling back to "install anyway."
- **Snapshot/rollback dry-runs**: pre-update and pre-layering Btrfs snapshots must be provably restorable, and the 15% free-space pruning safeguard must be exercised (including the guarantee that in-progress-rollback-required snapshots are never pruned).
- **Localization/accessibility release gate**: top-15-language pass, WCAG 2.1 AA, and screen-reader regression testing, gating final GA sign-off — begun during the Phase 5/Beta window specifically so failures surface with runway to fix them, not at the door of GA.
- **Third-party security audit**: commissioned at the end of the Applications phase so findings land early in the Optimization phase, with results published alongside GA rather than withheld.
- **Feature-freeze enforcement**: a hard freeze date is enforced release-over-release; anything not landed goes onto a published post-1.0 roadmap rather than silently slipping the date.

---

## 7. RISKS

| Difficult component | Possible failure mode | Solution / mitigation |
|---|---|---|
| `bootc`/OSTree tooling (still maturing upstream) | Upstream instability blocks the image pipeline | Contribute fixes upstream rather than fork; track the Fedora Atomic Desktops roadmap closely rather than diverging |
| NVIDIA proprietary driver auto-install | Driver install fails and bricks boot on a large hardware segment | Dedicated NVIDIA QA lane; automatic fallback to open kernel modules/`nouveau`; boot must never be allowed to brick regardless of driver-install outcome |
| GNOME Shell Extension API v1 | API surface proves insufficient for planned UX, or upstream Shell churn breaks the wrapper | Design v1 conservatively; treat v2 as an explicit post-1.0 item; re-validate the wrapper on every GNOME Shell minor bump *before* Canary→Beta promotion, blocking promotion on a break |
| NVIDIA + Wayland explicit-sync | Regression on any Mutter or NVIDIA driver release | Dedicated QA lane re-run on every relevant release; fallback to open modules/`nouveau` if unresolved before a release deadline |
| `systemd-homed` multi-user edge cases | Home encryption/unlock failures in uncommon multi-user configurations | Extensive multi-user testing matrix; Recovery Mode explicitly covers `systemd-homed` unlock failure as a first-class recovery path |
| SELinux enforcing-by-default flip | Real-world false positives break workflows on the verified app catalog | Extensive dogfooding period against the full catalog is the mitigation itself — the enforcing flip is gated on an acceptable false-positive rate, not a calendar date |
| Third-party security audit | Findings reveal architectural issues discovered too late to fix pre-GA | Engage auditors at the end of the Applications phase (not the start of Optimization) so there's runway to remediate before GA |
| Hardened kernel toggle | Driver/gaming regressions if hardened settings leak into the default kernel | Default kernel stays gaming-friendly; hardened mode is strictly opt-in, never the default |
| Chicken-and-egg app availability | New distro launches with an empty app catalog | Flatpak/Flathub compatibility means the entire existing catalog is available on day one — this is a structural solution, not a marketing claim |
| Store review backlog | Manual review queue can't keep pace with submissions | Automate as much of the lint/verification pipeline as possible; Community tier provides a no-review-required path that doesn't block on the reviewed tiers |
| Scope creep delaying GA | Continuous feature addition prevents ever reaching a stable 1.0 | Hard feature-freeze date; post-1.0 roadmap published simultaneously with 1.0 to contain expectations |
| Localization quality | Machine-translated strings ship as the effective final quality bar | Community translation program launched well before GA (Applications phase); machine translation treated as a floor, not a ceiling |
| Accessibility gaps discovered late | Accessibility issues found only at the GA gate, with no time to fix | Accessibility testing begins during the Beta window (Applications/Optimization phases), not deferred to the final Testing phase |
| Idle-RAM/boot-time CI flakiness | Benchmark noise causes false-positive CI blocks and erodes trust in the gate | Explicit +200 MB tolerance buffer between marketed target and CI block threshold, documented as intentional so it isn't mistaken for target-softening |
| ARM (aarch64) port | Attempting a full parity port before profiling data exists leads to premature, wrong targets | Build system stays multi-arch from day one; actual performance-parity targets are deliberately deferred until real profiling data exists post-evaluation |

---

*This plan is an implementation-planning artifact derived from `FINAL_OS_SPECIFICATION_V2.md`. It does not supersede the specification; where any detail here appears to conflict with the spec, the spec's ruling governs and this plan should be corrected to match.*
