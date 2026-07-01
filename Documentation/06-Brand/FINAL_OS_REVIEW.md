# FINAL OS REVIEW — Arbor OS Specification v1.0

**Reviewer role:** Senior Linux distribution reviewer / QA pass
**Scope:** Targeted quality control against `FINAL_OS_SPECIFICATION.md`. This is not a redesign — only weak points, contradictions, and gaps are flagged, with a minimal fix for each.

---

## 1. Technical Feasibility

### 1.1 ESP filesystem contradiction — one version is not bootable
**Issue:** §2 Conflict 6's final decision states desktop filesystems are "Btrfs for root and home, **ext4 for the ESP**, no ZFS." But §3.5 and §10.1 both correctly specify the ESP as **FAT32/vfat**.
**Why it matters:** This isn't a style disagreement — it's a hard UEFI firmware requirement. Standard UEFI implementations can only read FAT12/16/32 from the EFI System Partition. An ext4 ESP would not be recognized by firmware and the machine would not boot via the documented Secure Boot chain (§3.6, §7.2). If any build script or installer partitioner is written against the Conflict 6 text instead of §3.5, it will produce unbootable images.
**Recommended improvement:** Correct the Conflict 6 resolution text to read "FAT32 for the ESP" so all three references agree, and add a one-line note explaining *why* (UEFI firmware constraint), so future editors don't "fix" it back to ext4 for consistency with the rest of the Btrfs-everywhere story.

### 1.2 Boot menu timeout conflicts with the sub-8-second boot claim
**Issue:** §3.6 specifies a boot menu with a "5-second timeout" as a normal part of every boot. §6.2 sets a CI-gated target of **< 8.0 s cold boot to login**, and the benchmark table reports **6.8 s** achieved.
**Why it matters:** A boot menu that waits 5 seconds before auto-selecting the default deployment would, by itself, consume most or all of the 8-second budget on every single boot — which contradicts the 6.8 s measured result. This reads as if the "always show menu, 5s timeout" behavior and the "fast, invisible boot" behavior were written by different sections without reconciling them.
**Recommended improvement:** Clarify that the menu is **hidden by default** (immediate boot into the current deployment) and only becomes visible — with its timeout — when a key is held during POST or a previous boot failed. This is standard `systemd-boot` behavior; the spec should say so explicitly rather than implying the timeout is always in the critical path.

### 1.3 Scheduler default is stated twice, inconsistently
**Issue:** §6.3 states "EEVDF... as the process scheduler," then in the same subsection lists `scx_rustland` (a sched-ext/BPF scheduler that fully takes over scheduling from EEVDF/CFS when loaded) as the "Default" activation for general desktop use.
**Why it matters:** sched_ext schedulers don't run alongside EEVDF for the same tasks — when active, they replace it for the tasks in that class. Stating both as "the default" is internally contradictory and will confuse whoever implements the scheduler-switching logic for GameMode/Battery Saver: is the resting state EEVDF, or is `scx_rustland` always loaded and EEVDF only a fallback?
**Recommended improvement:** State one resting default explicitly, e.g.: "EEVDF is the kernel's built-in scheduler and the fallback if no SCX policy is loaded; `scx_rustland` is loaded as the desktop-session default at session start, with `scx_lavd`/`scx_bpfland` swapped in per GameMode/Battery Saver."

---

## 2. Architecture Consistency

### 2.1 RAM CI-gate thresholds don't match the stated targets
**Issue:** §6.2 sets the *target* at ≤ 1.2 GB (Standard) / ≤ 1.6 GB (Developer). §10.2's CI Gates Summary lists the *blocking* threshold as ≥ 1.4 GB / ≥ 1.8 GB for the same two metrics — a 200 MB gap in each case that is never explained anywhere in the document.
**Why it matters:** Without an explicit rationale, it's unclear whether this gap is an intentional tolerance buffer (target vs. hard-fail line) or a drafting error where one table wasn't updated when the other changed. Either interpretation should be a one-sentence statement, not left to inference — CI configuration will be built directly off one of these two numbers.
**Recommended improvement:** Add a short note under §6.2 or §10.2: "CI blocks only at target + 200 MB tolerance to avoid flaking on benchmark noise; the target itself is what's marketed and reviewed each release." If that's not the intended reasoning, reconcile the numbers to match.

### 2.2 Distrobox's `unconfined_u` exception is asserted but not bounded
**Issue:** §8.1 and §8.9 both state Distrobox containers run in `unconfined_u` SELinux context in Developer Mode, described as "the established Linux container security model," while §7.5 states SELinux "governs all Arbor-maintained system daemons and kernel interfaces" as a hard rule.
**Why it matters:** This isn't wrong (unconfined containers are indeed common practice), but the spec doesn't document what *is* still enforced around an unconfined container — e.g., whether rootless Podman's user namespace remapping, the immutable host image, or `fapolicyd` still bound the blast radius. As written, the "hard rule" in §7.5 and the exception in §8.9 sit next to each other without the boundary between them being spelled out, which is exactly the ambiguity Conflict 9 was written to eliminate for AppArmor.
**Recommended improvement:** Add 2–3 lines to §8.9 stating explicitly which controls remain active around an `unconfined_u` Distrobox container (user namespace isolation, host immutability, no root daemon) so the exception reads as scoped rather than as a silent carve-out.

---

## 3. Performance Realism

### 3.1 No documented minimum hardware spec against aggressive RAM targets
**Issue:** The spec sets a ≤ 1.2 GB idle RAM target and describes zswap pool sizing "20% of RAM (1.6 GB **on an 8 GB machine**)" — implying 8 GB is a supported configuration — but there is no HCL minimum-RAM (or minimum storage/CPU) requirement stated anywhere in §3.10 or §10.1.
**Why it matters:** On an 8 GB machine, 1.2 GB idle OS footprint plus a zswap pool that can grow to 1.6 GB before spilling to disk leaves a materially different usable-memory picture than on the 32 GB reference hardware used for all published benchmarks (§6.2). Without a stated floor, HCL certification, QA, and marketing claims ("11h 20m battery," "980 MB idle") have no defined lower bound to be tested against, and low-RAM users have no way to know if Arbor OS is realistic for their hardware.
**Recommended improvement:** Publish an explicit minimum/recommended hardware tier (e.g., "8 GB RAM minimum, 16 GB recommended for Developer Mode") in §3.10, and note that published benchmarks are on recommended-tier hardware, not minimum-tier.

---

## 4. Hardware Compatibility

### 4.1 Modern laptop camera stacks are absent from the HCL
**Issue:** §3.10's HCL table covers Intel/AMD/NVIDIA GPUs, Wi-Fi, Bluetooth, fingerprint, and touchscreens, but has no entry for camera hardware. Many 2022+ laptops ship MIPI-CSI cameras requiring `libcamera` + platform-specific IPU drivers (e.g., Intel IPU6) rather than plain UVC, and driver maturity varies significantly across vendors.
**Why it matters:** The Privacy Indicator feature (§4.2, §5.4) is explicitly built around "camera or microphone in use" detection — a headline privacy feature that depends on the camera stack actually working. Silently shipping on hardware where the camera doesn't function (a common real-world Linux laptop pain point) undermines a feature the spec treats as a differentiator.
**Recommended improvement:** Add a "Camera (MIPI/IPU vs. UVC)" row to the HCL table in §3.10, following the same "Verified / Limited / Roadmap" tiering used for other hardware classes.

### 4.2 No input method framework for non-Latin scripts, despite a 15-language localization target
**Issue:** Phase 5 (§9) commits to "Localization: top 15 languages," but no section of the spec mentions an input method framework (e.g., IBus or Fcitx5) for CJK, Indic, or other non-Latin-script input.
**Why it matters:** For a meaningful subset of the top-15-language list (Chinese, Japanese, Korean, likely Hindi), a working input method is not an optional nicety — the OS is largely unusable for text entry in that language without one. This is a gap between a stated roadmap commitment and the architecture that would support it.
**Recommended improvement:** Add an input-method framework decision to §3 or §8 (e.g., "IBus, pre-installed, auto-configured per selected language in onboarding Step 1") alongside the existing localization commitment.

---

## 5. Maintenance Complexity

### 5.1 Snapshot retention has no disk-space safeguard
**Issue:** §3.9 describes automatic Btrfs snapshots before every update/layering operation plus hourly/daily/weekly `/home` snapshots, with retention "configurable in Settings." No section defines a default disk-space-based pruning behavior — retention appears to be time-based only.
**Why it matters:** Time-based-only retention on a small/aging SSD (which the spec otherwise cares deeply about, e.g., write-amplification tuning in §3.5) can silently fill the disk with old snapshots, especially for users who never open Settings. This is a common real-world failure mode for Btrfs/Snapper-based systems that don't ship with an out-of-the-box space guard, and it directly threatens the "atomic updates never fail messily" promise in §3.9 if an update can't proceed due to a full disk.
**Recommended improvement:** Add a default free-space threshold (e.g., "oldest non-pinned snapshots are pruned automatically if free space drops below X%") alongside the time-based retention, and surface a low-space warning in the Updates & Rollback settings panel (§4.5).

### 5.2 Dual scheduler/theming/extension surfaces increase upstream-drift maintenance burden without a stated tracking owner
**Issue:** The spec commits to tracking three fast-moving upstream surfaces closely: sched-ext BPF schedulers (§6.3), GNOME Shell's Extension API wrapping (§3, Conflict 7), and NVIDIA's Wayland explicit-sync status (§3.7, §4.2 risk table) — each described as "tracked closely with upstream," but no section assigns an update cadence or a defined compatibility-break response process.
**Why it matters:** These are exactly the kinds of dependencies that historically break Linux desktop distributions between releases (GNOME Shell extension API churn, sched_ext being a relatively young kernel subsystem, NVIDIA/Wayland compositor sync issues). "Tracked closely" is a monitoring intent, not a maintenance plan, and the roadmap (§9) doesn't allocate recurring engineering time to it beyond initial bring-up.
**Recommended improvement:** Add a short "Upstream Tracking" subsection (or a table row per dependency) stating the review cadence (e.g., "re-validated every GNOME/kernel point release") and who owns the compatibility gate — this is lower-effort than it sounds and prevents these three items from becoming ownerless technical debt.

---

## 6. User Experience Consistency

### 6.1 Onboarding offers "Print" for the recovery passphrase before any printer exists
**Issue:** Step 4 of first-boot onboarding (§4.6) offers `[Copy] [Print]` for the recovery passphrase. At this point in the flow, no printer has been configured (printer setup isn't mentioned anywhere in onboarding or elsewhere in the UX spec).
**Why it matters:** A "Print" button that likely has no discoverable printer to target on a fresh install is a small but real first-impression UX inconsistency, particularly since this step is explicitly called out as one the installer "refuses to silently skip" — i.e., it's meant to be taken seriously, and a non-functional-looking button undercuts that.
**Recommended improvement:** Either add minimal printer discovery before this step, or replace/supplement "Print" with a QR-code or "save to USB drive" option that's guaranteed to work with no prior setup.

---

## 7. Development Difficulty

*(No new issues beyond those already covered in Architecture Consistency §2.2 and Maintenance §5.2, which both bear directly on implementation difficulty — the Extension API v1 boundary and the SELinux/Distrobox exception both need clearer contracts before engineers can build against them confidently.)*

---

## 8. Missing Important Components

### 8.1 No enterprise identity/fleet management story despite targeting "enterprise trust"
**Issue:** §8.5 Layer 3 describes an "Enterprise/team deployment model" (custom OS images via `bootc switch`), and Phase 4 (§9) explicitly targets "professional/enterprise trust." But there is no mention anywhere of directory/SSO integration (e.g., Kerberos/AD/LDAP), MDM/fleet enrollment, or remote wipe — all standard expectations for any OS claiming enterprise readiness.
**Why it matters:** Custom image builds alone don't answer how an IT department would centrally manage identity, compliance, or lost/stolen device response across a fleet of Arbor OS machines. Without at least a stated intent, "enterprise trust" in the Phase 4 goal is aspirational language without a corresponding architecture line item.
**Recommended improvement:** Add a short note to §8.5 or a new §7.12 scoping what enterprise identity/fleet management looks like (even if it's "out of scope for 1.0, revisit post-GA based on the same custom-image mechanism") so the gap is acknowledged rather than implied to be solved.

### 8.2 Printing/scanning end-user workflow is undocumented
**Issue:** CUPS and SANE are mentioned once each, only in the AppArmor exception list (§2, Conflict 9). There is no printing or scanning entry anywhere in §4 (User Experience) or §3.10 (HCL) — no Settings panel, no driverless-IPP story, no scan app.
**Why it matters:** Printing is unglamorous but universal; its complete absence from the UX and hardware-compatibility sections (while Bluetooth, display, and sound each get dedicated Settings panels in §4.5) suggests it was overlooked rather than deliberately deferred.
**Recommended improvement:** Add a "Printers & Scanners" row to the §4.5 Settings table (driverless IPP/AirPrint-style default, matching the "one good default" philosophy in §1.4) and a corresponding HCL entry.

---

## Summary

The specification is unusually thorough for a "final consolidated" document, and most of the ten explicit design-conflict resolutions in §2 are handled well — clear rationale, clear final ruling, no hedging. The issues above are concentrated in a few places: two hard contradictions worth fixing immediately (the ESP filesystem, the boot-menu-timeout-vs-boot-time claim), a couple of internally inconsistent numbers that need one clarifying sentence each (RAM gate tolerance, scheduler default), and a handful of scope gaps (enterprise fleet management, input methods, camera HCL, printing) that are common blind spots in Linux desktop specs and worth closing before Phase 1 implementation locks in assumptions around them.
