# Arbor OS — Repository Structure

**Status:** Foundation design only. No features are implemented in this document — it defines where future work will live and how contributors interact with the repository.
**Derived from:** `FINAL_OS_SPECIFICATION_V2.md` (v2.0, Canonical) and `IMPLEMENTATION_MASTER_PLAN.md`
**Scope:** This document defines the top-level layout of the `arbor-os` foundation repository, the conventions contributors must follow, and the workflow used to move code from a feature branch to a shipped release tier. It does not define any subsystem's internal implementation.

---

## 0. How This Repository Relates to the Spec

Arbor OS is built on Fedora Atomic (`rpm-ostree`/`bootc`), ships an immutable base image with layered/sandboxed user-facing components, and promotes changes through four release tiers: `Development → Canary → Beta → Stable`. Every folder below exists to serve one of those two halves of the system — **the trunk** (immutable image: build, kernel, os-config, packages, installer) or **the branches** (mutable, user-facing: desktop, applications, themes) — plus the supporting scaffolding (docs, tests, ci) that keeps the whole tree honest. Nothing in this repository implies functionality has been written yet; each folder currently holds structure, placeholders, and contribution rules only.

---

## 1. Repository Layout

```
arbor-os/
├── build/                        # Image build system (bootc/rpm-ostree pipeline)
│   ├── containerfiles/            # Containerfile-based image definitions
│   │   ├── base.Containerfile      # Trunk image: kernel + systemd + filesystem baseline
│   │   ├── hardened.Containerfile  # arbor-kernel-hardened overlay
│   │   └── variants/               # KDE spin, future aarch64 target, etc.
│   ├── manifests/                 # rpm-ostree treefile / package manifests
│   ├── signing/                   # Signing key references, signature verification configs
│   │                               # (private keys are NEVER stored in-repo — see §3)
│   └── scripts/                    # Build orchestration (Python/Bash), reproducibility checks
│
├── kernel/                       # Kernel configuration (not kernel source — Arbor tracks upstream)
│   ├── arbor-kernel/               # Default channel: config fragments, CONFIG_HZ, EEVDF/sched-ext wiring
│   │   └── scx/                     # scx_rustland (default), scx_lavd (GameMode), scx_bpfland (Battery Saver)
│   ├── arbor-kernel-hardened/      # CONFIG_LOCKDOWN_LSM, kptr_restrict=2, yama.ptrace_scope=2, module signing
│   ├── lts-fallback/               # Boot-entry pinning config for the always-present LTS fallback kernel
│   └── upstream-tracking/          # Version-pin files + changelogs for the 3 high-risk upstream deps (§3.12 of spec)
│
├── os-config/                    # Operating system configuration (the "trunk" services layer)
│   ├── systemd/                    # Unit files/overrides: homed, oomd, boot, resolved, networkd, dbus-broker
│   ├── filesystem/                 # Btrfs mount options, subvolume layout, Snapper config, ESP (FAT32) spec
│   ├── security/                   # SELinux policy (targeted), fapolicyd allowlist, nftables default-deny rules
│   ├── update-tiers/               # OSTree ref definitions for Development/Canary/Beta/Stable + promotion gate config
│   ├── snapshot-policy/            # 15% free-space floor, oldest-first pruning rules
│   └── input-methods/              # IBus engine wiring (Pinyin, Mozc, Hangul, etc.)
│
├── packages/                     # Package definitions and layering
│   ├── rpm-ostree-layers/          # System-level layered packages (drivers, kernel modules) — advanced path
│   ├── flatpak-manifests/          # First-party Flatpak manifests (source of truth, mirrored to Arbor Store)
│   └── homebrew-formulae/          # Any Arbor-authored user-space CLI formulae
│
├── desktop/                      # Arbor Shell (GNOME Shell + Extension API v1, no C-level fork)
│   ├── extension-api-v1/           # The stable, versioned wrapper contract around Shell internals
│   ├── extensions/                 # First-party extensions (panel, Smart Dock, Arbor Snap tiling, gestures)
│   ├── session/                    # dconf/GSettings overrides, session-start sound oneshot unit
│   └── compositor-config/          # Mutter/Wayland session config, NVIDIA explicit-sync overrides
│
├── applications/                 # First-party system applications, one subfolder per app
│   ├── files/
│   ├── terminal/                   # Arbor Terminal (Rust/wgpu renderer, VTE-ng, Fish + Starship default)
│   ├── settings/                   # Arbor Control Center
│   ├── backup/
│   ├── timeline/                   # Snapshot/rollback GUI
│   ├── store/                      # Arbor Store client (Verified/Flathub/Community tiers)
│   ├── container-studio/           # Distrobox GUI integration
│   └── devtools-cli/               # `arbor devtools` CLI + SDK Manager + scaffolding templates
│
├── installer/                    # First-boot and install-time logic
│   ├── boot-chain/                 # Shim → systemd-boot → signed kernel/initramfs config; GRUB legacy-BIOS fallback
│   ├── disk-setup/                 # Btrfs subvolume layout + FAT32 ESP creation logic (never ext4 — firmware constraint)
│   ├── encryption/                 # LUKS2 + TPM2 sealing, recovery-passphrase generation (save-to-USB/QR, not Print)
│   ├── onboarding-wizard/          # 6-step first-run flow: language/region → a11y → account → encryption → privacy → theme
│   └── recovery/                   # Signed Recovery OSTree deployment + GUI recovery wizard
│
├── themes/                       # Visual identity, shared by desktop/ and applications/
│   ├── libadwaita-theme/           # Arbor GTK/CSS theme (dark default, light variant)
│   ├── icons/                      # Line-based system icon set (24×24, 2px stroke)
│   ├── wallpapers/
│   ├── sound/                      # Session-start sound asset + PipeWire playback hook
│   └── design-tokens/              # Color palette, typography scale as machine-readable tokens (JSON/TOML)
│
├── docs/                         # Documentation site source (builds to Arbor Pages)
│   ├── architecture/               # Mirrors of spec decisions relevant to contributors (ADRs, see §3)
│   ├── developer-guide/            # Extension API v1 docs, `arbor devtools` usage, app scaffolding guide
│   ├── user-guide/                 # End-user facing docs
│   └── contributing/               # This document, coding conventions, branch strategy, PR templates
│
├── tests/                        # Cross-cutting test suites (subsystem-local tests live next to their code)
│   ├── qemu-harness/               # Boot-time, idle-RAM, idle-CPU, frame-time CI gate harness
│   ├── hcl/                        # Hardware Compatibility List data + per-model test results (Verified/Supported/Limited)
│   ├── security/                   # SELinux false-positive tracking, signature-chain verification tests
│   ├── accessibility/              # WCAG 2.1 AA + screen-reader regression suite
│   └── integration/                # End-to-end flows: onboarding, rollback, multi-user homed edge cases
│
└── ci/                           # CI/CD pipeline definitions
    ├── pipelines/                  # Per-domain pipeline configs (image, shell, apps, devtools)
    ├── gates/                      # Machine-readable gate thresholds (mirrors spec §10.2 exactly — see §3)
    ├── signing/                    # CI-side signing invocation config (keys held in CI secrets store, never in-repo)
    └── promotion/                  # Development→Canary→Beta→Stable promotion automation + human-review gate config
```

---

## 2. Folder-by-Folder Explanation

### `build/` — Build System
Owns the image-build pipeline: `bootc` as the primary build/deploy tool, `rpm-ostree` for layering and compatibility operations. Every artifact produced here is a signed OCI/OSTree image — signing always happens in CI, never on a contributor's machine. This folder is the only place that should ever reference base-image `Containerfile`s; kernel and os-config folders supply the *inputs* that `build/` assembles.

### `kernel/` — Kernel Configuration
Arbor does not fork or vendor kernel source; this folder holds **configuration and channel-selection logic only** — the `arbor-kernel` / `arbor-kernel-hardened` split, scheduler wiring (EEVDF fallback, sched-ext BPF schedulers per mode), and the LTS fallback boot entry that must always exist regardless of active channel. `upstream-tracking/` is the concrete home for the three named high-risk dependencies (SCX, GNOME Shell Extension API wrapping, NVIDIA explicit-sync) — each subfolder must contain an owner and a review cadence, not just a version pin.

### `os-config/` — Operating System Configuration
The non-kernel trunk: `systemd` service configuration, the Btrfs/Snapper filesystem baseline (including the FAT32-only ESP rule), the SELinux/fapolicyd/nftables security baseline, and the OSTree update-tier definitions. This is deliberately separate from `packages/` — `os-config/` defines *how the trunk behaves*, `packages/` defines *what can be layered onto it*.

### `packages/` — Packages
Three distinct package surfaces, kept in separate subfolders because they have different trust models: `rpm-ostree-layers/` (system-level, advanced, rollback-safe, clearly labeled in UX as image-modifying), `flatpak-manifests/` (the primary end-user app format, sandboxed via Bubblewrap), and `homebrew-formulae/` (user-space developer tooling with no system integration).

### `desktop/` — Desktop Environment
Arbor Shell, defined exactly as the spec resolves it: GNOME Shell plus the Extension API v1, first-party extensions built against that API, and configuration layered via dconf/GSettings — **no fork of `gnome-shell`, `mutter`, or `libadwaita` C source lives here or anywhere in this repository.** `extension-api-v1/` is the single most important folder for long-term maintainability: it is the stable contract that isolates every other folder in `desktop/` from upstream GNOME API churn.

### `applications/` — System Applications
One subfolder per first-party app, each an independent buildable unit against the same design system. `devtools-cli/` lives here rather than under `build/` or `ci/` because it ships to end users (developers) as a product, not as internal tooling.

### `installer/` — Installer
Everything that runs before or during first boot: the Secure Boot chain, disk/encryption setup, and the six-step onboarding wizard. `recovery/` is grouped here rather than in `applications/` because the Recovery OSTree deployment is a boot-time artifact, not a running-session app, even though it has a GUI component.

### `themes/` — Themes
Kept as its own top-level folder (rather than nested only under `desktop/`) because visual identity — theme, icons, wallpapers, the session-start sound, and design tokens — is consumed by both `desktop/` (shell chrome) and `applications/` (first-party app UI). A single source of truth here prevents drift between the two.

### `docs/` — Documentation
The source for the public developer documentation site (Arbor Pages) and the internal contributor guide. `architecture/` holds Architecture Decision Records that reference spec section numbers directly, so a decision's rationale is always one click from its implementation.

### `tests/` — Testing
Cross-cutting suites that span multiple folders above: the QEMU CI-gate harness, the Hardware Compatibility List, security/signature verification tests, and accessibility regression tests. Subsystem-specific unit tests are **not** here — they live alongside their code (e.g., `applications/terminal/tests/`). This folder exists specifically for tests that no single subsystem folder owns.

### `ci/` — CI/CD
Pipeline definitions, gate thresholds, and promotion automation. `gates/` must mirror the spec's §10.2 CI Gates Summary exactly (boot time, idle RAM with its documented +200MB tolerance buffer, frame time, etc.) — a discrepancy between `ci/gates/` and the spec is treated as a bug in `ci/gates/`, never the other way around.

---

## 3. Coding Conventions

### 3.1 Language-to-domain mapping
Following the spec's technology decisions, code in this repository is written in:

| Language | Used for |
|---|---|
| **Rust** | Arbor Terminal renderer (`wgpu`), performance-sensitive first-party tooling, Extension API v1 components that need memory-safety guarantees |
| **C / Vala / GJS** | GNOME Shell extensions (native extension model) |
| **Python** | Build/CI tooling, `arbor devtools` CLI, image-build automation |
| **Bash / Fish** | System scripts, packaging glue, install-time hooks |
| **TOML** | Configuration formats (Arbor Terminal config, package manifests where declarative diffable config is preferred over YAML) |

### 3.2 General rules
- **No local forks of upstream C source** (`gnome-shell`, `mutter`, `libadwaita`, kernel). Any change to upstream behavior goes through the Extension API v1 layer, a config override, or an upstream PR — never a vendored patch against the C source.
- **Reproducibility is a hard requirement**, not an aspiration: any change to `build/`, `kernel/`, or `packages/` must not break bit-identical rebuild verification.
- **Every commit that feeds the image-build pipeline must be signed** (GPG-signed commits/tags). This is the first link in the signature chain (Secure Boot → OSTree commit → Flatpak repo → layered RPM → firmware); an unsigned commit cannot enter `build/` or `packages/`.
- **No secrets, keys, or credentials in-repo**, ever — including test/dev keys. Signing keys are referenced by name/path in `build/signing/` and `ci/signing/`, never stored as file contents.
- **Linting is mandatory before merge**: Rust (`clippy`/`rustfmt`), Python (`ruff`, `mypy`), Shell (`shellcheck`), plus a dedicated Flatpak-manifest sandbox-permission linter for anything touching `packages/flatpak-manifests/`.
- **Every new dependency added to `kernel/upstream-tracking/`** must include an owner and a review cadence in the same PR — an unowned dependency is treated as a blocking review comment, not a nit.

### 3.3 Naming
- Folder names: lowercase, hyphenated (`extension-api-v1`, not `ExtensionAPIv1`).
- Config/gate files that mirror a spec table (e.g., `ci/gates/`) should use the same terminology as the spec (e.g., `idle-ram-standard.yaml`, not a renamed equivalent) so cross-referencing stays mechanical, not interpretive.

---

## 4. Documentation Rules

- **The spec and the plan are canonical and immutable from inside this repository.** `FINAL_OS_SPECIFICATION_V2.md` and `IMPLEMENTATION_MASTER_PLAN.md` are mirrored read-only under `docs/architecture/`; they are never edited to match code. If implementation reveals a gap or contradiction, the correct action is to amend the spec through Core OS Team review first, then update code — never the reverse.
- **Architecture Decision Records (ADRs)** live in `docs/architecture/adr/`, one file per decision, numbered sequentially, each referencing the spec section (e.g., `§3.9`) it implements or extends. An ADR that contradicts the spec without an accompanying spec amendment is rejected in review.
- **Every top-level folder gets a `README.md`** stating: its purpose, what does *not* belong there, and a pointer to the relevant spec/plan section. This document (`REPOSITORY_STRUCTURE.md`) is the index; individual READMEs go one level deeper.
- **Public developer docs (Arbor Pages)** are built exclusively from `docs/developer-guide/` and `docs/user-guide/` — internal architecture rationale in `docs/architecture/` is not published externally.
- **No undocumented CI gate.** Any threshold in `ci/gates/` must have a corresponding row in `docs/architecture/` explaining what it measures and why the number was chosen, mirroring spec §10.2/§6.1 of the plan.

---

## 5. Branch Strategy

Arbor OS uses a **tier-mapped trunk-based model** — branch structure mirrors the OSTree release tiers so promotion is a first-class, auditable Git operation, not just a CI artifact label.

```
main                  → always deployable to "Development" tier
├── canary            → promoted from main after automated gate pass
├── beta               → promoted from canary after gate pass + human review
└── stable              → promoted from beta after gate pass + human review + freeze-window compliance
```

- **`main`** is the integration branch. All feature work merges here via pull request. `main` must always build and pass the automated gate suite — it is never force-pushed.
- **`canary`**, **`beta`**, **`stable`** are promotion branches, not development branches. Nothing is committed directly to them; they only ever receive fast-forward or merge-commit promotions from the tier below, gated by the CI suite in `ci/gates/` plus (for beta and stable) human review.
- **Feature branches** are cut from `main`, named `feature/<domain>/<short-description>` (e.g., `feature/desktop/smart-dock-gestures`), and are deleted on merge.
- **Fix branches** for issues found in a promoted tier are named `fix/<tier>/<short-description>` and are back-merged into `main` after landing, so a fix never exists only downstream of `main`.
- **A regression discovered at any tier blocks that promotion**, regardless of which tier caught it — a Beta-tier regression does not get waved through to Stable "since it's close."
- **Release freeze**: once a feature-freeze date is declared for a given Stable promotion, `main` accepts only fixes targeting that release; new feature branches continue merging but are excluded from the frozen promotion.

---

## 6. Development Workflow

1. **Pick up work against a spec/plan section.** Every PR description must reference the spec section (e.g., `§4.5`) or plan phase (e.g., `Phase 4 — Desktop Environment`) it implements. Work with no traceable spec/plan reference is treated as scope creep and redirected to a spec amendment discussion first.
2. **Branch from `main`** using the naming convention in §5.
3. **Develop against the folder's local conventions** — check for a `README.md` in the target folder before adding files; if one doesn't exist yet, adding it is part of the PR.
4. **Run local lint/format/reproducibility checks** before opening a PR (see §3.2). CI re-runs all of these; a PR that only passes locally due to environment drift will fail CI and is expected to.
5. **Open a PR into `main`.** Required checks: linting, unit tests local to the changed folder, and any cross-cutting suite in `tests/` that the change plausibly affects (reviewers flag missing coverage here rather than assuming it's out of scope).
6. **Code review** requires at least one approval from a maintainer of the affected domain (image/kernel, desktop, applications, installer, docs/ci are treated as distinct review domains for this purpose).
7. **Merge to `main`** once checks and review pass. `main`'s continuous gate suite (boot time, idle RAM, frame time, etc., per `ci/gates/`) must remain green; a merge that breaks it is reverted, not patched forward under pressure.
8. **Promotion to `canary`** happens on a regular cadence (not per-commit), running the full gate suite in `tests/qemu-harness/` and `tests/hcl/`.
9. **Promotion to `beta`** additionally requires human review sign-off and, for anything touching the three high-risk upstream-tracked dependencies (§3.12), a passing re-validation recorded in `kernel/upstream-tracking/`.
10. **Promotion to `stable`** additionally requires: signature-chain verification (`tests/security/`), and — ahead of any GA-labeled stable promotion specifically — a clean pass of `tests/accessibility/` and the full HCL gate in `tests/hcl/`.
11. **Any promotion regression blocks that promotion outright** and routes back to step 2 as a `fix/<tier>/...` branch; it does not get an exception.

---

## 7. Explicit Non-Goals of This Document

To keep this a foundation-only deliverable, the following are intentionally **not** defined here and are left for later, feature-specific design work:
- Actual Containerfile contents, kernel config values, or extension code.
- Specific CI YAML/pipeline syntax (only the folder that will hold it, and what it must mirror).
- App-level architecture within any single `applications/<app>/` folder.
- The multi-repo-per-domain split mentioned as a future scaling option in the implementation plan (§5.2) — this document defines the single foundation repository as the starting point; splitting it is a later, separately-justified decision.
