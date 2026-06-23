# ARBOR OS — SECURITY ARCHITECTURE

**Document version:** 1.0 · **Status:** Security specification, companion to the OS Architecture Blueprint
**Scope:** This document expands §5 (Security) and §6 (Privacy) of the Arbor OS blueprint into a complete, implementable security architecture — the goal being the most secure *mainstream* Linux desktop OS: secure enough for journalists and lawyers, usable enough for switchers who've never opened a terminal.

**Design thesis:** Security must be the default path, not the hardened path. Every control below is on by default, reversible only through explicit, informed user action — never a silent downgrade.

---
## 0. THREAT MODEL

Before designing controls, define what Arbor OS defends against, in priority order:

| Threat | In scope | Notes |
|---|---|---|
| Opportunistic malware (malicious Flatpak, trojanized script, drive-by download) | ✅ Primary | Sandboxing + allowlisting is the main line of defense |
| Physical theft / "evil maid" (stolen/seized laptop) | ✅ Primary | Full-disk encryption + Secure Boot + measured boot |
| Supply-chain compromise (poisoned package, compromised build) | ✅ Primary | Reproducible builds, signed images, tiered app trust |
| Network-based attacks (MITM, rogue Wi-Fi, DNS spoofing) | ✅ Primary | Encrypted DNS, WPA3, firewall, VPN-first-class |
| Post-exploitation lateral movement (compromised app pivoting to host) | ✅ Primary | SELinux + namespaces limit blast radius |
| Targeted state-level exploitation (zero-day kernel exploits, hardware implants) | ⚠️ Partial / best-effort | Hardened kernel variant, Secure Workstation edition narrows but doesn't eliminate this |
| Nation-state supply-chain interdiction of hardware | ❌ Out of scope | Outside a software distribution's control |

This threat model drives a principle used throughout: **defense in depth with usable defaults** — no single control is assumed sufficient, but no control should require the user to understand *why* it's there to benefit from it.

---
## 1. SECURITY FOUNDATION

### 1.1 Secure Boot

**Goal:** guarantee that only code Arbor has signed — or that the user has explicitly trusted — executes before the kernel hands off to a measured, verified userspace.

**Chain of trust:**
```
UEFI Firmware (OEM root of trust)
   ↓ verifies
Arbor-signed shim (Microsoft 3rd-party UEFI CA signed, for out-of-box compatibility)
   ↓ verifies
Arbor-signed systemd-boot
   ↓ verifies
Signed kernel + initramfs (signed in the CI build pipeline, not on-device)
   ↓ measures into
TPM PCRs (see §1.2)
```

- **Signing pipeline:** kernel, initramfs, and bootloader are signed centrally during the bootc/mkosi image build in CI — never on the end-user device. Signing keys live in an offline/HSM-backed process; CI only holds a signing *capability*, not the key material.
- **MOK (Machine Owner Key) enrollment** is exposed through a guided GUI wizard for the one legitimate reason a user needs it: installing an out-of-tree kernel module (rare, discouraged — see Kernel Strategy in the blueprint, DKMS as last resort). The wizard explains, in plain language, what enrolling a MOK means and what it weakens.
- **Legacy BIOS fallback (GRUB)** is supported but explicitly labeled in the installer and in Settings as *"Secure Boot unavailable on this hardware"* — never silently degraded without the user seeing that label.
- **Boot menu integrity:** the boot menu itself (current deployment, rollback entries, recovery) is part of the signed chain; an attacker who can write to the ESP without breaking Secure Boot still can't add an unsigned, executable boot entry.

### 1.2 TPM 2.0 Integration

TPM is used for two distinct, complementary purposes — attestation and key sealing — and the UI never conflates them.

**a) Sealed disk-encryption keys (`systemd-cryptenroll`)**
- The LUKS2 volume key is sealed against a defined set of PCRs (typically 0, 2, 4, 7, 11 — firmware, option ROMs, boot loader, Secure Boot state, kernel/initrd measurement).
- If the measured boot chain matches, the disk unlocks with **no password prompt** — a Windows Hello/BitLocker-equivalent UX.
- If *any* measured component changes (firmware update, tampered bootloader, swapped disk in a different chassis), the TPM refuses to release the key and the user is prompted for the **recovery passphrase**, generated at install time and shown once for the user to store (printed / password manager / paper).
- Firmware and kernel updates that legitimately change PCR values trigger an automatic, authenticated re-seal as part of the atomic update transaction (the new deployment re-enrolls the TPM binding using the *currently unlocked* key before the old measurement is invalidated) — so routine updates never surprise the user with an unexpected passphrase prompt.

**b) Measured boot attestation, surfaced in plain English**
- Settings → Security shows a single indicator: **"Boot Integrity: OK"** (green) or **"Boot Integrity: Changed since last login — verify this was you"** (amber), backed by PCR comparison against the last-known-good measurement log.
- Advanced users can view the raw PCR/event log; everyone else gets the plain-English summary. This is the desktop-Linux equivalent of remote attestation, but local and self-referential — no telemetry, no third party sees the measurement.

**c) No-TPM fallback:** on hardware without TPM 2.0 (or with it disabled in firmware), LUKS2 falls back to passphrase-only unlock, with a clear one-time notice explaining the reduced convenience (not security) — encryption itself is never contingent on TPM presence.

### 1.3 Kernel Hardening

Two kernel channels ship, both mainline-tracked for fast CVE turnaround (per the base blueprint):

| Setting | `arbor-kernel` (default) | `arbor-kernel-hardened` |
|---|---|---|
| `CONFIG_PREEMPT` | Desktop-latency tuned | Same |
| KASLR | On | On, stronger entropy source |
| Stack protector | Regular | `-fstack-protector-strong` |
| `/proc/kallsyms`, `/proc/<pid>/*` | Standard restrictions | Restricted (`kernel.kptr_restrict=2`, `kernel.dmesg_restrict=1`) |
| `ptrace` scope | `yama.ptrace_scope=1` (restricted to child processes) | `2` (admin-only) |
| BPF for unprivileged users | Enabled (needed by some sandboxing/tracing tools) | Disabled by default, per-app exception list |
| Module loading | Signed modules preferred | Signed modules **enforced** (`module.sig_enforce=1`) |
| Speculative execution mitigations | All enabled, default tuning | All enabled, no perf-over-security tradeoffs taken |
| SLAB/heap hardening | Distro default | grsecurity-inspired allocator hardening flags |
| `CONFIG_LOCKDOWN_LSM` | Integrity mode | Confidentiality mode (blocks `/dev/mem`, kexec of unsigned images, etc.) |

- **Default channel** stays compatibility/gaming-friendly (this matters for the Steam/Proton user journey in the UX spec) — hardening that would break anti-cheat or GPU passthrough isn't forced on everyone.
- **Hardened channel** is one toggle in Settings → Security ("Secure Workstation Mode") or selectable at install time; switching is a normal atomic deployment swap (test-boot, rollback-safe, per the update model — see §5).
- **LTS fallback kernel** is always kept as a boot entry regardless of channel, for regression safety.
- **Compiler-level hardening** applied to the whole base image build: `-D_FORTIFY_SOURCE=3`, full RELRO, PIE by default, CFI where toolchain support allows (Fedora's existing hardening flag set as the floor, not the ceiling).

### 1.4 Mandatory Access Control (SELinux)

- **SELinux, targeted policy, enforcing by default from GA** — not permissive-with-a-warning like many distros ship out of caution. Phase 4 of the roadmap exists specifically to make this safe (extensive policy audit + dogfooding against the full curated app catalog before flipping to enforcing).
- **Confines system daemons** regardless of whether an app is also Flatpak-sandboxed — a compromised system service (e.g., a bug in `udisks2` or NetworkManager) is contained by SELinux even though it never crosses a Flatpak boundary.
- **Policy transparency:** `sealert`-equivalent GUI notification when SELinux denies something a user would reasonably expect to work, with one-click "report as false positive" that both fixes it locally (targeted local policy module, clearly logged) and optionally submits the denial pattern upstream for the next image's policy tuning — this directly targets the historical reason distros disable SELinux (breakage with no feedback loop).
- **AppArmor as a fallback layer**, not a competing primary MAC — used for specific third-party integrations (e.g., certain browser sandboxes, print/scan daemons) where upstream projects ship AppArmor profiles Arbor doesn't want to fork into SELinux policy. The two coexist without conflict because they confine different, non-overlapping subjects.

### 1.5 Sandboxing

Layered, not single-point:

1. **Flatpak + Bubblewrap** is the primary app isolation boundary for GUI applications: each app gets its own mount namespace, PID namespace, and (unless granted) no network namespace access, no arbitrary filesystem access, no device access.
2. **XDG Desktop Portals** mediate every cross-boundary request (file open, screenshot, camera, location) so that even a fully-sandboxed app can still *function* — the user grants access per-file or per-session rather than the app losing capability entirely (the "sandboxed but not useless" balance that has historically hurt Flatpak adoption when done badly).
3. **Distrobox/Toolbx containers** for CLI/dev work get their own isolation (via Podman) — a compromised dependency inside a dev container can't touch the immutable host image, only the container's own overlay.
4. **`fapolicyd`** allowlists what's permitted to *execute at all* on the base system outside sandboxes — binaries must be part of the signed image or explicitly layered/approved; this stops "download and run" attacks (fake update binaries, malicious shell scripts) that sandboxing alone doesn't prevent, because sandboxing only helps once something is already running *inside* a sandbox.
5. **Kernel namespaces** (cgroups v2, PID/mount/network/user namespaces) underpin all of the above — no legacy cgroups v1 hybrid mode, which both simplifies the security model and removes historical cgroups v1 escape classes.

---
## 2. APPLICATION SECURITY

### 2.1 App Permissions

- **Portal-mediated, per-permission, revocable at any time** — camera, microphone, location, contacts-equivalent, filesystem scope (not "all files," but per-folder or per-file grants via the file picker portal), network, screen capture, background execution, autostart.
- **Static manifest declaration + runtime prompt**, mirroring the mobile permission model users already understand: an app declares in its Flatpak manifest what it *might* ask for; the actual grant happens at first use, with a native system dialog (not an in-app one an attacker could spoof).
- **Granularity example — filesystem:** rather than "Files: Allowed," the dashboard shows *which* folders (`~/Documents/Projects`, one specific PDF) an app can reach, each individually revocable, with a "reveal in Arbor Files" shortcut to see exactly what that grant covers.
- **Time-boxed grants:** any permission can optionally be scoped to "this session only" — camera/mic access for a video-call app that reverts to prompt-again next launch, useful for occasionally-used apps a privacy-conscious user doesn't want holding a standing grant.
- **No permission is silently widened by an app update** — a Flatpak update that adds a new manifest permission (e.g., an app newly requests network access) requires re-confirmation before the updated version can access that resource, surfaced clearly in the update notification rather than buried in a changelog.

### 2.2 Application Isolation

- **Default posture: zero trust between apps.** No shared temp directories, no implicit D-Bus session bus exposure beyond what portals require, no X11-era "any app can screenshot/keylog any window" — Wayland's per-app input isolation is a hard architectural requirement (X11 session isn't shipped by default, per the base blueprint).
- **Inter-app communication only through mediated channels:** portals for file/media exchange, `xdg-mime` for "open with" flows that go through a picker rather than direct filesystem handoff, and explicit D-Bus policy allowlists for apps that legitimately need to talk to each other (e.g., a password manager's browser-extension bridge).
- **Network isolation per app** is a first-class, visible property — the App Store's "Network Isolation" badge (per the UX spec) reflects a real manifest constraint, not marketing copy: an offline note-taking app can be shipped with no `--share=network` permission at all, verifiable by anyone inspecting the manifest.
- **GPU/device isolation:** apps needing GPU acceleration get scoped DRI device access, not a blanket `/dev` bind-mount; apps needing USB devices go through `udev`+portal-brokered access rather than raw device nodes.

### 2.3 Package Verification

- **Reproducible builds** for all first-party Arbor apps and the base OS image itself — anyone can rebuild from source and get a bit-identical artifact, closing the "trust the binary because you trust the signer" gap with "verify the binary matches the source."
- **Tiered trust, always visible, never a false binary:**
  - **Arbor Verified** — reviewed by Arbor's security team, reproducible build confirmed, manifest permissions audited against actual behavior.
  - **Flathub Verified** — upstream Flathub's own developer-identity verification.
  - **Community** — sandboxed like everything else, but unreviewed; clearly labeled, opt-in via the "Enable unverified apps" toggle rather than the new-user default.
- **Signature verification at every layer:** OSTree commits (base image) are GPG/signify-signed and verified on pull; Flatpak repos are signed and verified per Flatpak's existing model; RPM layering (the system-extension escape hatch) verifies package signatures against Arbor's and Fedora's keyrings before allowing a layer.
- **Build provenance metadata** surfaced in the Store's app detail page (build commit hash, maintainer identity, source repo link — already specified in the UX design) so a security-conscious user can independently audit before installing, without needing a terminal.

### 2.4 Malware Protection

- **Allowlisting over blocklisting as the base-system philosophy:** `fapolicyd` means unknown/unsigned binaries simply don't execute on the host, which is structurally stronger than signature-based AV scanning for anything outside a sandbox.
- **Sandbox containment as the primary anti-malware boundary for third-party apps** — a malicious Flatpak app can misbehave only within the blast radius its manifest permissions allow, which the permission dashboard (§2.1) makes visible and revocable after the fact, not just at install time.
- **No bundled AV telemetry suite** — deliberate: bundling a scanning daemon that phones home signature updates and scan telemetry conflicts with the zero-telemetry stance. **ClamAV offered as an optional Flatpak** for the specific, real use case of Linux users who need to scan files before handing them to Windows-using colleagues — opt-in, not a background service.
- **CI-based sandbox lint** for Store submissions (per the blueprint's §7) — automatically flags apps requesting excessive permissions relative to their stated function (e.g., a calculator app requesting filesystem + network + camera), feeding both the privacy-label generation and a manual review queue before an app can carry any trust badge above "Community."
- **Execution-time script protection:** shell scripts, downloaded installers, and similar "run this" artifacts outside the sandboxed app model are subject to the same `fapolicyd` allowlist — a browser download of a malicious `.sh` file doesn't get an implicit execute bit; Arbor Files' "Shred File" and general download handling never auto-chmod +x downloaded content.

---
## 3. PRIVACY

### 3.1 No Telemetry

- **Zero telemetry transmitted by default, full stop.** No anonymous usage statistics, no silent crash reporting, no "help improve Arbor OS" pre-checked box.
- **Crash reports are opt-in and reviewed before sending** — when a crash is detected, the user sees exactly what would be sent (stack trace, relevant log excerpt) in a dialog before any network call happens, sent to Arbor's own infrastructure, never a third-party crash-analytics vendor.
- **Update check-ins are privacy-preserving by design:** no unique machine identifier is generated or transmitted; the check-in reveals only a country/architecture bucket (Debian popcon / Fedora countme style), and the check-in mechanism's source is published and auditable — "trust us" is replaced with "verify it yourself."
- **No first-party app ships an ad-tech or analytics SDK**, full stop — this is a Store submission gate, not just a first-party policy, enforced by the same CI sandbox lint that checks permissions.

### 3.2 Network Privacy

- **Encrypted DNS by default** (DoH/DoT), user-selectable resolver, with a hard architectural rule: the system **never silently falls back to plaintext DNS** — if encrypted resolution fails, the user sees a clear "DNS privacy degraded" state rather than a silent downgrade.
- **MAC address randomization on by default** for Wi-Fi, matching modern mobile OS posture — a real default competitors don't ship, closing a common physical-tracking vector (retail Wi-Fi presence tracking, etc.).
- **Per-app network kill switch**, integrated into the same "Arbor Firewall" GUI described in the base blueprint (§5) — allow/block/always-ask per app, not just system-wide.
- **First-class WireGuard VPN**, built into Settings rather than requiring a third-party client — removes the usual "install an app, trust its permission model" step for a control that's itself privacy infrastructure.
- **WPA3 verified per chipset in the HCL**, with WPA2 fallback clearly flagged (not silently accepted) when hardware/AP doesn't support WPA3.

### 3.3 Permission Management

(Cross-referenced with §2.1 — this is the same system viewed from the privacy angle, not a separate mechanism.)

- **Single Settings panel, per-app, per-permission, live-toggle** — exactly the dashboard specified in the UX design's Notification & Control Center and Settings flows.
- **Access history log**, human-readable: *"Spotify accessed your microphone at 3:41 PM"* — this closes the gap between *granting* a permission once and *knowing* when it's actually used, which is where most desktop privacy models (including historically Linux) fall short of mobile.
- **Live indicator in the top bar** (the "Privacy: Green/dot" element in the UX spec's status pill) turns active whenever camera or microphone is in use system-wide, regardless of which app or sandbox triggered it — a hardware-truthful indicator, not just an app-reported one, ideally backed by a kernel-level "is this device node open" check rather than trusting the requesting app's own state.

### 3.4 User Transparency

- **Nothing enforced without disclosure.** Every default (telemetry off, MAC randomization on, encrypted DNS on) is shown and explained during onboarding (per the UX spec's Privacy Configuration step), not buried in a EULA.
- **Privacy labels per app** in the Store — network access, data-collection claims, permissions requested — generated from manifest analysis where possible (objective) and developer self-report where not (subjective, but community-flaggable for inaccuracy), same "nutrition label" model referenced in the UX spec.
- **Full local admin retained.** Arbor deliberately does not adopt ChromeOS-style lockdown — every privacy default can be inspected, understood, and if the user genuinely wants to, disabled. "Your machine, your rules" is a stated design principle, not just marketing: a security architecture that can't be audited or overridden by its owner isn't actually serving that owner's security.
- **Update/telemetry mechanism source is public** — the check-in code, the crash-report payload format, the privacy-label generator — all auditable, so "no telemetry" is a verifiable property, not a trust claim.

---
## 4. USER SECURITY

### 4.1 Security Dashboard

A single Settings → Security home screen, information-dense but plain-language first, consolidating:

- **Boot Integrity** status (§1.2c): OK / Changed.
- **Disk Encryption** status: encrypted, TPM-bound or passphrase-only, per-volume (root, each `systemd-homed` user home).
- **Update status**: current deployment version, signature-verified, last successful rollback test.
- **SELinux mode**: enforcing (green) with a link to any recent denials that were auto-resolved vs. need review.
- **Firewall summary**: apps with active network permissions, recent blocked attempts.
- **Account security**: 2FA/hardware-key enrollment status, active login sessions, fingerprint enrollment.
- **Recent security events feed**: new device authorized, permission granted to a newly-installed app, firmware update applied via `fwupd`, failed unlock attempts.
- **One-click "Run Security Check"**: an on-demand pass that verifies Secure Boot chain integrity, checks for any `fapolicyd` policy drift, confirms LUKS/TPM binding health, and checks for pending critical security updates — surfaced as a simple pass/fail-with-detail report, not a raw log dump.

Advanced users can drill from any card into the underlying raw state (PCR values, `semanage`/`audit2allow` output, `nft` ruleset) — the dashboard is a layer on top of real system state, never a fictional simplification of it.

### 4.2 Encryption Management

- **LUKS2 full-disk encryption on by default, opt-out not opt-in**, matching (and in the TPM-bound case, exceeding the friction of) macOS FileVault's default posture — this is set at install time per the UX onboarding flow, no separate "turn on encryption later" step most users never take.
- **`systemd-homed`-managed per-user home encryption** on top of full-disk encryption for multi-user machines — one user's compromised session or forgotten-unlocked device doesn't expose another user's home directory.
- **Recovery key handling:** generated at install, shown once, with explicit guidance (print it, save to a password manager, do *not* store it unencrypted on the same disk) — the installer refuses to silently skip this step.
- **Re-key / passphrase rotation** available anytime from the dashboard without requiring a full re-encryption pass (LUKS2's multiple-keyslot model makes this cheap).
- **Encrypted external media support** integrated into Arbor Files — plugging in an unencrypted drive prompts an optional "Encrypt this drive" flow (LUKS2), and previously-encrypted drives from Arbor or other LUKS2-aware systems mount transparently after unlock.
- **Per-file/folder "Encrypt with Vault"** (from the UX spec's Files app) for cases where full-disk encryption isn't enough on its own — e.g., a shared/multi-boot machine, or an extra layer for a specific sensitive project folder, backed by `gocryptfs`-or-equivalent sandboxed folder encryption rather than a second full LUKS container per folder.

### 4.3 Recovery System

- **Dedicated minimal Recovery OSTree deployment** (per the blueprint's §4), reachable from the signed boot menu even if the main deployment is unbootable: includes network access, a terminal, the Btrfs snapshot browser/restore tool, and a "repair boot entries" utility.
- **Account recovery without cloud dependency:** because Arbor deliberately has no mandatory cloud account (per the blueprint's zero-telemetry, no-account-requirement stance), account recovery is local-first — LUKS recovery passphrase, plus optional (user-opted-in) local-network or user-owned-cloud (via the same Restic/B2/S3 backend as Arbor Backup) recovery-key escrow, never an Arbor-run identity service by default.
- **"Repair, don't reinstall" as the default recovery posture:** the Timeline/Snapshot system (§5.2) plus Recovery Mode's tooling means the first recovery instinct is "roll back to a known-good snapshot," not "wipe and reinstall" — this matters for security specifically because it preserves forensic evidence of what changed, useful if a user suspects compromise rather than mere instability.
- **Guided recovery flow for non-technical users:** Recovery Mode's terminal-first tools are wrapped in a minimal GUI wizard for the common cases (roll back update, restore a file, reset a forgotten password via recovery key) so "recovery environment" doesn't mean "you need to know Btrfs CLI syntax" for the switcher persona.

### 4.4 Account Protection

- **Local accounts by default**, `systemd-homed`-managed, with password + optional fingerprint (`fprintd`) and/or hardware security key (FIDO2/YubiKey) enrollment offered at account creation (per the UX onboarding spec).
- **Hardware key support end-to-end:** login, sudo/Polkit elevation, and optionally full-disk unlock can all be bound to a FIDO2 token, not just a web-login convenience — `pam_u2f` integration across the PAM stack.
- **Polkit-mediated privilege escalation** with clear, app-identified prompts (which app/action is requesting elevation, not just a generic "authenticate" dialog) — reduces the phishing-style risk of a malicious app spoofing a legitimate elevation request.
- **Failed-unlock throttling** with exponential backoff at the LUKS/login layer, and an optional (opt-in, clearly explained) data-wipe-after-N-failed-attempts policy for high-risk users (journalists/lawyers persona) — off by default because it's a footgun for the general population, available in Secure Workstation Mode.
- **Session security:** automatic screen lock on suspend/lid-close (configurable delay, never "never" as the hidden default), Wayland's native per-app input isolation preventing cross-app keylogging, and a visible "N other devices" list for any account with sync/portable-home features enabled via `systemd-homed`.
- **No mandatory cloud account, ever** — account protection here means protecting *the local account*, not gatekeeping OS functionality behind an online identity, which is itself a security property (no cloud account = no cloud-account takeover vector for the OS login itself).

---
## 5. UPDATES

### 5.1 Secure Update Process

- **Entire OS ships as a signed, versioned OCI-style image** (bootc), built centrally in CI — never assembled or compiled from packages on the end-user device. This removes an entire class of "compromised local package database/mirror" attacks that affect traditional package-manager distros.
- **Signature verification before any deployment is created:** the client verifies the new image's signature against Arbor's signing keys before pulling/creating a new OSTree deployment; a failed verification aborts the update with a clear error, never falls back to "install anyway."
- **Update transport is HTTPS + content-addressed OSTree commits** — even a compromised CDN edge can't serve a modified image without the signature check failing, and the content-addressing means partial/corrupted transfers are detected, not silently applied.
- **fwupd/LVFS integration** brings firmware updates (BIOS, SSD, peripheral firmware) through the same signed, atomic, UI-unified update story — closing a historically separate and often-neglected update surface.
- **No live-patching of a running kernel/base system.** Updates always build a new deployment and test-boot into it; the running system is genuinely never mutated in place, which structurally prevents the "half-updated, inconsistent state" bug class that's also a security liability (mismatched library/kernel ABI assumptions).

### 5.2 Rollback

- **Automatic boot-counting fallback:** `systemd-boot` tracks failed boot attempts into a new deployment; after N failures, it automatically falls back to the last-known-good deployment without user intervention — critical for the case where an update itself introduces a security regression or simply fails to boot.
- **Instant, pointer-swap rollback** — not a restore-from-backup operation. Selecting a previous deployment (from the boot menu or `Settings → System → Rollback`) is an OSTree commit pointer swap plus a Btrfs subvolume swap, typically single-digit seconds, matching the blueprint's demonstrated "8 second rollback before a client presentation" journey.
- **Pre-update snapshots are automatic and mandatory**, not opt-in: a Btrfs snapshot is taken before every OS image update and every layered-package operation, independent of the OSTree deployment mechanism, giving two independent rollback paths (deployment-level and filesystem-level).
- **Rollback preserves user data by design** — home directory snapshots are managed on their own retention schedule (§ Snapshots in the base blueprint), so rolling back the OS deployment after a bad update doesn't roll back a user's subsequent document edits unless they explicitly also restore a home snapshot.

### 5.3 Signature Verification

- **Multi-layer signature chain, each layer independently verified:**
  - Boot chain: shim → bootloader → kernel/initramfs (Secure Boot, §1.1).
  - OS image: OSTree commit signatures verified on every pull (§5.1).
  - Flatpak apps: repo-level signing verified per Flatpak's model, plus Arbor's own additional reproducible-build verification for the "Arbor Verified" tier (§2.3).
  - Layered RPM packages (system-extension escape hatch): package signatures verified against Arbor's and Fedora's keyrings before a layer is permitted.
  - Firmware: LVFS-signed firmware payloads verified by `fwupd` before flashing.
- **No update path bypasses verification, including the escape hatches** — `rpm-ostree install` layering is clearly labeled in the UI as an advanced, rollback-relevant action specifically *because* it's the one path where a user can introduce unverified system-level code; the labeling itself is a security control (informed consent, not silent risk).
- **Key management transparency:** which keys sign which artifact classes is documented publicly; key rotation events are themselves published and, where feasible, cross-signed with the previous key to avoid a "trust cliff" for offline devices catching up on updates after a long gap.

---
## 6. SUMMARY TABLE

| Layer | Primary control | Backstop |
|---|---|---|
| Boot | Secure Boot signed chain | LTS fallback kernel, boot-counting rollback |
| Disk | LUKS2 + TPM-sealed key | Recovery passphrase, no-TPM passphrase fallback |
| Kernel | Hardened build flags, mainline CVE cadence | Optional `arbor-kernel-hardened` channel |
| Access control | SELinux enforcing (targeted) | AppArmor for select third-party integrations |
| App isolation | Flatpak + Bubblewrap + portals | `fapolicyd` execution allowlist on host |
| Malware | Sandbox containment + allowlisting | Optional ClamAV, CI sandbox-permission lint |
| Network | Encrypted DNS, WPA3, MAC randomization, per-app firewall | First-class WireGuard VPN |
| Privacy | Zero telemetry by default | Auditable, source-available check-in mechanism |
| Permissions | Portal-mediated, per-app, revocable | Access history log, live hardware indicator |
| Updates | Signed atomic image, verified before deploy | Automatic rollback on boot failure |
| Recovery | Instant snapshot/deployment rollback | Dedicated signed Recovery OSTree environment |
| Accounts | Local `systemd-homed`, FIDO2/fingerprint support | Polkit-mediated elevation, no mandatory cloud account |

---
## 7. OPEN QUESTIONS FOR PHASE 4 (Security Hardening)

Per the blueprint's roadmap, these are the items this document intentionally leaves as "best current answer, open to revision" pending real-world testing:

1. **SELinux enforcing-by-default false-positive rate** against the full curated app catalog — needs the extensive beta/dogfooding period called out in the roadmap before GA.
2. **Exact PCR set for TPM sealing** — balancing "reseal too often" (annoying) against "reseal too rarely" (weaker attestation) needs hardware-diversity testing across the HCL.
3. **Secure Workstation Mode's exact default toggles** (ptrace scope, unprivileged BPF, data-wipe-after-N-failures) — needs input from the actual target persona (journalists/lawyers) rather than being finalized in the abstract.
4. **Third-party security audit** — commissioned per the roadmap, results will likely revise specifics in §1–§2 without changing the overall architecture.

---
*This document should be read alongside the Arbor OS Architecture Blueprint (base system, update model) and the UX/UI Design System (how these controls surface to the user). Security controls that exist but aren't legible to the user in the interface don't count as shipped — every mechanism above has a corresponding UI touchpoint by design.*
