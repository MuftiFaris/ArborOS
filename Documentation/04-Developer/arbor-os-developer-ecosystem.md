# ARBOR OS — DEVELOPER ECOSYSTEM

**Document version:** 1.0 · **Status:** Pre-development design spec
**Related:** `01-Architecture/os-architecture-blueprint.md`, `03-Security/arbor-os-security-architecture.md`

---

## DESIGN PHILOSOPHY

Arbor OS targets developers as its second most important audience (behind privacy-conscious professionals, often the same people). The developer experience is not a bolt-on — it is a first-class design concern.

**Core developer promises:**

1. **Zero environment tax** — a new machine should reach full working environment in one session, not a weekend.
2. **Containers, not pollution** — development dependencies never contaminate the immutable host.
3. **Security that enables, not obstructs** — SELinux and sandboxing are configured to work *with* developer workflows, not fight them.
4. **Reproducible by default** — dev environments are declarative and version-controlled, so they survive machine changes and team onboarding.
5. **Privacy everywhere** — no telemetry leaks from dev tools; IDE analytics are opt-in at prompt, never silently enabled.

---

## 1. DEVELOPMENT TOOLS

### 1.1 Terminal — Arbor Terminal

Arbor ships its own first-party terminal emulator, **Arbor Terminal**, rather than bundling a third-party default with mismatched aesthetics.

**Technical foundation:**

| Property | Choice | Rationale |
|---|---|---|
| Renderer | GPU-accelerated (wgpu/Vulkan) | Sub-millisecond latency; consistent with Arbor's Vulkan-first compositor |
| Backend | VTE-ng (forked, maintained) | GNOME ecosystem compatibility; accessibility tree support |
| Protocol | Kitty terminal protocol (extended) | True-color, graphics, keyboard disambiguation |
| Configuration | TOML, hot-reloading | Declarative, diffable, versionable |
| Multiplexing | Native tabs + panes (no tmux required by default) | First-class session persistence across suspend/resume |

**Built-in features:**
- **Session persistence** — terminal sessions survive logout via systemd user units; reconnect on next login.
- **SSH integration** — known-hosts managed via `~/.config/arbor-terminal/ssh/`, GUI passphrase entry wired through GNOME Keyring/TPM.
- **URL/path detection** — file paths, URLs, and stack trace lines are clickable by default (Ctrl+click opens in default app).
- **Developer-palette** — a "Developer color scheme" ships as the default alongside the standard Arbor theme, with WCAG 2.1 AA contrast verified.
- **Wayland-native** — no XWayland dependency; full input method support.

**Shell defaults:**
- Default shell: **Fish** (new-user default, discoverable, good out-of-box UX) with **Bash** and **Zsh** fully supported.
- Starship prompt pre-configured with Arbor theme integration (shows git branch, virtualenv, container context, exit code).
- Shell history: `atuin` (encrypted, searchable, shell-history sync opt-in) pre-installed.

---

### 1.2 Git

Git is installed in the base OS image — it is a system-level primitive, not an optional package.

**Included out-of-box:**

```
git 2.x (latest stable from Fedora)
git-credential-manager   — GNOME Keyring/libsecret integration, zero-config auth
git-lfs                  — large file support pre-installed
git-delta                — diff syntax highlighting (set as default pager)
lazygit                  — terminal UI for git, accessible via `lg` alias
gh                       — GitHub CLI (in Developer Mode; see §2)
```

**Pre-configured defaults (`/etc/gitconfig` system-level baseline):**

```ini
[core]
    pager = delta
    autocrlf = input
    editor = $VISUAL   ; respects user preference

[delta]
    syntax-theme = Arbor
    navigate = true
    line-numbers = true

[init]
    defaultBranch = main

[pull]
    rebase = true

[rerere]
    enabled = true

[commit]
    gpgsign = false   ; off by default, enabled via Developer Mode GPG wizard
```

**GPG commit signing setup:**
- Developer Mode (§2) includes a first-run wizard: "Do you want to sign your commits with GPG?" → generates key, uploads to keyserver of choice, configures git, stores private key in TPM-backed GNOME Keyring. Single screen, no manual `gpg --gen-key` dance.

---

### 1.3 Python

Python is a first-class runtime on Arbor. The system Python (from the base image) is **never** the development Python — it is locked to system tooling and inaccessible to user `pip install`.

**Runtime management via `mise`:**

`mise` (previously `rtx`) is Arbor's chosen universal runtime version manager. It handles Python, Node.js, Ruby, Go, Rust, and more under one tool — reducing the fragmentation of having pyenv + nvm + rbenv + goenv all separately configured.

```bash
# Install a Python version
mise use python@3.12

# Project-local version (creates .mise.toml, committed to git)
mise use --pin python@3.11.9

# List available
mise ls-remote python
```

**Python tooling stack (pre-installed in Developer Mode):**

```
mise               — runtime version management
uv                 — package manager + virtualenv (replaces pip + venv, ~10x faster)
ruff               — linter + formatter (replaces black + flake8 + isort)
mypy               — type checker
ipython            — enhanced REPL
jupyter            — notebooks (available via mise or Flatpak)
```

**Virtual environment behavior:**
- `uv venv` creates `.venv/` in project root by default.
- Shell integration activates the venv automatically when entering a project directory (Starship shows active venv).
- No global `pip install` — Arbor's Python policy enforces project isolation to avoid the "works on my machine" failure mode.

---

### 1.4 Node.js

Managed via `mise`, consistent with the Python approach.

```bash
mise use node@lts          # LTS (recommended default)
mise use node@22           # specific version
```

**Pre-installed tooling (Developer Mode):**

```
mise                  — version management (same tool as Python)
pnpm                  — default package manager (faster, better disk usage than npm)
npm                   — available (not blocked, just not the default)
yarn                  — available via corepack
bun                   — available via mise (optional, for Bun-native projects)
```

**Reasoning for pnpm as default:** Shared dependency store means multi-project monorepos don't duplicate `node_modules`; compatible with npm package ecosystem without lock-in.

---

### 1.5 Containers

Containers are a first-class citizen of Arbor's architecture, used for both the OS's own immutability model and developer workflows.

#### Podman (Rootless, Default)

**Podman** is the default container runtime — rootless by default, daemonless, and fully compatible with the Docker CLI surface.

```bash
podman run -it ubuntu bash
podman build -t myapp .
podman compose up           # podman-compose bundled
docker                      # aliased to podman; Dockerfile support complete
```

**Why Podman over Docker daemon:**
- Rootless by default → safer in SELinux-enforcing environment, no root daemon attack surface.
- Systemd-integrated: containers can be managed as systemd user units (`podman generate systemd`).
- OCI-compliant: fully interoperable with Docker Hub, GHCR, Arbor Registry.

**Container networking:**
- Netavark (Podman's default CNI replacement) — faster, rootless-compatible.
- DNS resolution in containers: `aardvark-dns` bundled.

#### Docker Compatibility

For developers whose workflows assume Docker:

```bash
# Docker CLI works as-is via podman socket compatibility layer
systemctl --user enable --now podman.socket
export DOCKER_HOST=unix://$XDG_RUNTIME_DIR/podman/podman.sock
```

A one-click "Enable Docker compatibility" toggle in Developer Mode settings activates this and writes the environment variable to the user's shell profile.

**Docker Desktop alternative:** Arbor provides **Arbor Container Studio** — a GUI for container and image management (built on Podman) that matches Docker Desktop's feature surface without requiring a license or root daemon. Includes:
- Image browser + pull UI
- Container log viewer
- Volume manager
- Compose file editor with validation

#### Distrobox / Toolbx (Built-in)

`distrobox` is pre-installed and is the recommended path for installing arbitrary Linux packages without touching the immutable host:

```bash
# Create a Debian development container
distrobox create --name dev-debian --image debian:bookworm
distrobox enter dev-debian

# Inside: full apt, sudo, X/Wayland apps work seamlessly
sudo apt install build-essential libssl-dev
```

Distrobox containers:
- Share the user's home directory (full file access).
- Export GUI apps and binaries to the host seamlessly (`distrobox-export`).
- Are managed via a GUI panel in Developer Mode settings.

---

### 1.6 Kubernetes

Kubernetes tooling targets the full workflow: local development → CI → production.

**Installed in Developer Mode:**

```
kubectl              — Kubernetes CLI (latest stable)
k3d                  — local k8s clusters via k3s in Docker/Podman
helm                 — package manager for Kubernetes
kustomize            — declarative config management
stern                — multi-pod log tailing
k9s                  — terminal UI for cluster management
kubectx / kubens     — fast context and namespace switching
```

**Local cluster setup (one command):**

```bash
# Create a local 3-node cluster
arbor devtools k8s create my-cluster

# Equivalent to:
k3d cluster create my-cluster --agents 2 --registry-create my-registry:5000
```

`arbor devtools k8s create` is a wrapper that:
1. Verifies Podman is running.
2. Creates the k3d cluster with Arbor-tuned defaults (resource limits, local registry pre-wired).
3. Merges kubeconfig into `~/.kube/config`.
4. Opens k9s in Arbor Terminal.

**Rootless Kubernetes:**
k3d runs inside Podman (rootless), consistent with Arbor's security posture. No root daemon required.

---

## 2. DEVELOPER MODE

### 2.1 Overview

**Developer Mode** is a single toggle in `Settings → System → Developer Mode`. Flipping it on is one action. No terminal required to enable it.

What it does:
1. Installs the `arbor-developer` meta-package (layered via rpm-ostree, triggers a background image update — user prompted to reboot at their convenience, not forced).
2. Unlocks Developer Mode UI panels throughout Settings.
3. Activates `arbor devtools` CLI suite.
4. Runs the **First-Run Developer Setup Wizard** on next login.

**Developer Mode does NOT:**
- Disable SELinux (it remains enforcing; developer policies are additive, not a bypass).
- Disable the firewall.
- Require a cloud account or license.
- Expose any telemetry.

---

### 2.2 First-Run Developer Setup Wizard

A single guided UI flow (modal overlay in the shell, not a separate app) that runs once after enabling Developer Mode.

**Steps:**

```
┌─────────────────────────────────────────────────────────┐
│  Welcome to Developer Mode                              │
│                                                         │
│  Let's set up your environment in a few steps.          │
│                                                         │
│  ① Identity         Name, email for git config         │
│  ② Git auth         GitHub / GitLab / Gitea / SSH key  │
│  ③ GPG signing      Sign commits? (optional)           │
│  ④ Runtime          Default Python + Node.js version   │
│  ⑤ IDE              Install your IDE (see §2.3)        │
│  ⑥ Environment      Import existing .mise.toml? (opt.) │
│                                                         │
│  [Skip all — I'll configure manually]   [Start setup → ]│
└─────────────────────────────────────────────────────────┘
```

Each step is independently skippable. The wizard writes to:
- `~/.gitconfig` (identity, signing config)
- `~/.config/mise/config.toml` (runtime versions)
- GNOME Keyring (credentials, SSH passphrases)

The wizard can be re-run at any time: `arbor devtools setup`.

---

### 2.3 IDE Setup

Arbor does not pick one IDE — it supports all major options with first-class integration. The wizard presents a curated list:

| IDE | Install method | Notes |
|---|---|---|
| **VS Code** | Flatpak (verified) | Default recommendation; Wayland-native via Electron Ozone |
| **VS Codium** | Flatpak (verified) | Open-source build; no Microsoft telemetry |
| **JetBrains Toolbox** | Flatpak (verified) | Manages all JetBrains IDEs (IDEA, PyCharm, etc.) |
| **Neovim** | Distrobox / Homebrew | Pre-configured Arbor `init.lua` starter config optionally applied |
| **Helix** | Distrobox / Homebrew | Modern modal editor; included in `arbor-developer` image |
| **Zed** | Flatpak (community) | GPU-accelerated; tracked for future Arbor Verified status |
| **Emacs** | Distrobox | PGTK build for Wayland; Doom/Spacemacs configs supported |

**VS Code / VSCodium integration specifics:**
- Arbor Terminal is automatically set as the default integrated terminal.
- Podman socket is pre-wired to the Docker extension.
- `mise` shims are on `PATH` inside the Flatpak sandbox via a portal extension.
- "Open in Distrobox" context menu action available for workspace folders.

**JetBrains integration specifics:**
- JetBrains Toolbox manages its own update lifecycle, isolated from Arbor's update cycle.
- Filesystem indexing speed: Btrfs + zstd means JetBrains index writes are fast and compressible.
- Remote Dev (SSH) configured automatically using credentials from GNOME Keyring.

---

### 2.4 SDK Management

`arbor devtools sdk` is the unified SDK management CLI. Under the hood it delegates to `mise` for runtimes and `distrobox` for SDKs that need system libraries.

```bash
# List available SDKs
arbor devtools sdk list

# Install a specific SDK
arbor devtools sdk install android-sdk
arbor devtools sdk install flutter
arbor devtools sdk install dotnet@8
arbor devtools sdk install rust
arbor devtools sdk install go@1.22
arbor devtools sdk install java@21-temurin
```

**SDK install destinations:**

| SDK type | Install destination | Rationale |
|---|---|---|
| Language runtimes (Python, Node, Go, Rust, Java) | `~/.local/share/mise/installs/` | User-scoped, no root required, mise-managed |
| Android SDK / Flutter | `~/.local/share/android-sdk/` | Standard Linux path |
| .NET | `~/.dotnet/` | Official Microsoft installer path, works in rootless context |
| Native build toolchains | Distrobox container | Prevents native library pollution of host |

**SDK GUI:** Settings → Developer → SDKs shows installed SDKs, available updates, and disk usage per SDK. One-click update for any SDK.

---

### 2.5 Environment Management

Development environments on Arbor are **project-scoped and declarative**.

#### Project Environments via mise

Every project gets a `.mise.toml` at its root that pins all runtime versions:

```toml
[tools]
python = "3.12.3"
node = "20.14.0"
go = "1.22.4"

[env]
DATABASE_URL = "postgres://localhost:5432/mydb"
NODE_ENV = "development"

[tasks]
dev = "npm run dev"
test = "pytest"
```

- Checked into git → every team member gets the same environment.
- `mise install` in a project directory installs all declared versions.
- Shell integration auto-activates on `cd` — no manual activation command.

#### Containerized Environments via Distrobox

For projects that need system-level dependencies (C libraries, specific glibc versions, OS-specific tooling):

```bash
# Create a project container
distrobox create --name myproject --image ubuntu:22.04 --home ~/projects/myproject
distrobox enter myproject

# Install system deps freely inside the container
sudo apt install libpq-dev libssl-dev cmake
```

An optional `distrobox.ini` file in the project root enables `arbor devtools env up` to recreate the container automatically — team members get an identical container on any machine.

#### `.env` File Management

- `direnv` is pre-installed and active by default.
- On `cd` into a project with a `.env` file, direnv loads environment variables automatically (with a one-time approval prompt per directory, per Arbor's security model).
- Secret `.env` files: `arbor devtools secrets` integrates with GNOME Keyring and optional external vaults (1Password CLI, Bitwarden Secrets) for secrets that shouldn't be in `.env` files on disk.

#### Snapshot-Backed Experiments

Because Arbor's home directory is on Btrfs, developers can snapshot their project state before a risky refactor:

```bash
# Snapshot current project state
arbor snapshot create "before-refactor-auth-module"

# List snapshots
arbor snapshot list

# Restore if the refactor went badly
arbor snapshot restore "before-refactor-auth-module"
```

This uses `btrfs subvolume snapshot` under the hood and is exposed through both CLI and the Timeline app (Arbor's graphical snapshot browser).

---

## 3. PACKAGE DEVELOPMENT

### 3.1 Creating Apps

Arbor's primary distributable format is **Flatpak**. This is a deliberate choice: Flatpak is already the Linux packaging standard with the widest cross-distro reach, and Arbor developers learn one format that works everywhere.

#### Arbor App Scaffolder

```bash
# Install the scaffolding tool (included in arbor-developer)
arbor devtools new app

# Interactive prompts:
# > App name: My Weather App
# > App ID: com.example.myweatherapp
# > Language/framework: [Python/GTK4] [Rust/GTK4] [JS/GNOME Builder] [C/GTK4] [Vala/GTK4]
# > License: GPL-3.0
# > Git init: yes

# Generates:
# myweatherapp/
# ├── com.example.myweatherapp.yml     # Flatpak manifest
# ├── src/
# ├── data/
# │   ├── com.example.myweatherapp.desktop
# │   ├── com.example.myweatherapp.metainfo.xml
# │   └── icons/
# ├── po/                              # i18n strings
# ├── meson.build
# └── README.md
```

The scaffolded project is immediately buildable:

```bash
cd myweatherapp
arbor devtools build          # wraps flatpak-builder, handles runtime deps
arbor devtools run            # installs locally and launches in sandbox
```

#### Flatpak Manifest (`com.example.myweatherapp.yml`) — Arbor Defaults

Arbor provides an opinionated Flatpak manifest template:

```yaml
app-id: com.example.myweatherapp
runtime: org.gnome.Platform
runtime-version: '46'
sdk: org.gnome.Sdk
command: myweatherapp

finish-args:
  - --share=network           # only if needed; scaffolder asks
  - --socket=wayland
  - --socket=fallback-x11
  - --device=dri

# Arbor adds sandboxing linter annotations:
# arbor-lint: no-home-access   ✓ passes
# arbor-lint: no-full-network  ✓ passes
# arbor-lint: metainfo-valid   ✓ passes

modules:
  - name: myweatherapp
    buildsystem: meson
    sources:
      - type: git
        url: .
        branch: HEAD
```

#### GNOME Builder Integration

For GUI app development, **GNOME Builder** is the recommended IDE (available in Arbor Store):
- Opens Flatpak manifest → builds in the correct Flatpak SDK automatically.
- Live preview for GTK4 UI files (Blueprint/XML).
- Integrated debugger attaches inside the Flatpak sandbox.
- Profiler integration (Sysprof) for performance work.

#### CLI / System Tool Development

For tools that don't need a GUI and will be distributed as Homebrew formulae or RPM packages:

```bash
arbor devtools new cli
# Generates a Rust or Python CLI template with:
# - Clap (Rust) or Typer (Python) argument parsing
# - GitHub Actions CI workflow
# - Homebrew formula template
# - RPM spec template
```

---

### 3.2 Publishing Apps

#### Publishing to the Arbor Store

The Arbor Store has a tiered trust model. Publishing targets **Arbor Verified** as the goal, with **Community** as a low-friction entry point.

**Submission workflow:**

```
Developer                     Arbor CI                  Arbor Review Team
    │                             │                             │
    ├─ git push manifest ────────►│                             │
    │                             ├─ sandboxing lint            │
    │                             ├─ permission audit           │
    │                             ├─ metainfo validation        │
    │                             ├─ reproducible build check   │
    │                             ├─ malware scan (ClamAV)      │
    │◄─ CI pass/fail + report ────┤                             │
    │                             │                             │
    ├─ submit for review ────────────────────────────────────►  │
    │                             │                  ├─ policy review
    │                             │                  ├─ UX review
    │                             │                  └─ sign manifest
    │◄─ published ───────────────────────────────────────────── │
```

**Submission steps:**

1. **Register** — create an Arbor Developer account (email only; no phone, no paid tier required for Community apps).
2. **Submit manifest** — `arbor devtools publish submit` or web dashboard upload.
3. **Automated CI** runs within minutes and returns a report.
4. **Fix issues** — the CI report maps every failure to a documentation page explaining the fix.
5. **Human review** — for Arbor Verified tier only; response time target: 5 business days.
6. **Published** — app appears in the Store.

**Privacy label (automatic):**
The CI pipeline auto-generates a machine-readable privacy label from the Flatpak manifest's `finish-args`. This label appears in the Store as:

```
Network access:      Yes — reasons: weather API
Home directory:      No
Camera:              No
Microphone:          No
Location:            No
Contacts:            No
```

Developers can annotate their manifest with justification strings that populate the "reasons" field:

```yaml
# arbor-privacy: network = "Required to fetch weather data from open-meteo.com"
```

#### Publishing CLI Tools to Homebrew

```bash
# Generate a Homebrew formula from your project
arbor devtools publish homebrew

# Outputs: Formula/myweatherapp.rb
# Auto-fills: description, URL, SHA256, dependencies from pyproject.toml/Cargo.toml

# Submit to Homebrew/homebrew-core or your own tap
```

#### Versioning and Updates

- Flatpak updates are delta-based (OSTree): only changed files are downloaded, typically KB not MB for point releases.
- `arbor devtools publish update <version>` bumps the manifest version, triggers CI, and queues for Store auto-approval (minor/patch versions with no permission changes).
- Breaking permission changes (new `finish-args`) always require re-review.

---

### 3.3 Documentation

Every Arbor app should include documentation. The tooling makes this the path of least resistance.

#### App Metadata (Required for Arbor Verified)

All apps must include a valid **AppStream MetaInfo** file (`com.example.myapp.metainfo.xml`). The scaffolder generates a template; `arbor devtools lint` validates it.

Required fields:
- App description (short + long)
- Screenshot(s)
- Release history with changelog
- Content rating (OARS)
- Project URL + issue tracker URL

#### Developer Documentation Site

`arbor devtools docs new` scaffolds a documentation site using **Arbor Docs** — a static site generator with the Arbor visual theme applied:

```bash
arbor devtools docs new
# Generates:
# docs/
# ├── index.md       # Getting started
# ├── api/           # API reference (auto-generated from source docstrings)
# ├── guides/        # How-to guides
# └── arbor-docs.toml

arbor devtools docs serve    # Local preview at http://localhost:4000
arbor devtools docs build    # Build static HTML
arbor devtools docs deploy   # Deploy to Arbor Pages (free static hosting for verified apps)
```

**Arbor Pages** provides free static hosting for documentation and project pages. Custom domains supported. Served via CDN. No tracking pixels, no analytics unless the developer opts in.

#### In-App Help

GTK4 apps get first-class help support via the GNOME Help system:

```bash
# The scaffolder creates a basic Mallard help book
docs/help/
└── C/
    ├── index.page
    └── getting-started.page
```

Accessible from the app's menu → Help (F1), consistent with all first-party Arbor apps.

---

## 4. SYSTEM CUSTOMIZATION

### 4.1 Theming

Arbor's design system is built on **libadwaita** (GNOME's modern UI library). Theming respects the "strong trunk, mutable edges" philosophy: the system's visual identity is consistent by default, and deviation is possible without breaking updates.

#### Arbor Theme Engine

Custom themes are expressed as **CSS overlays on top of the Arbor base theme**, not full replacements. This means:

- App authors never need to know if a custom theme is active — the overlay handles it.
- Updates to the base theme (bug fixes, accessibility improvements) are automatically inherited.
- Themes cannot break core system UI elements (Settings, Shell, Lock Screen).

**Theme format:**

```
~/.local/share/arbor/themes/my-theme/
├── theme.toml          # metadata: name, author, variant (light/dark/both)
├── gtk.css             # libadwaita overrides
├── shell.css           # Arbor Shell overrides (limited API, documented)
└── palette.json        # Named color tokens (used by both gtk.css and shell.css)
```

`palette.json` allows themes to define semantic color tokens that propagate everywhere:

```json
{
  "accent_color": "#7B68EE",
  "accent_fg_color": "#FFFFFF",
  "window_bg_color": "#1E1E2E",
  "window_fg_color": "#CDD6F4",
  "headerbar_bg_color": "#181825"
}
```

**Creating a theme:**

```bash
arbor devtools theme new "My Theme"
# Generates the directory structure above with commented template CSS

arbor devtools theme preview     # Live reload in a sandbox window
arbor devtools theme validate    # Checks contrast ratios (WCAG AA required for publishing)
arbor devtools theme package     # Packages as an Arbor theme archive (.arbortheme)
```

**Publishing themes:**
Themes are published to the **Arbor Store** under the "Themes" category. They use the same submission pipeline as apps, with a lighter review process (automated contrast validation replaces security review).

#### Shell Accent Colors

Users can set a system accent color in Settings → Appearance → Accent. Arbor propagates this through `palette.json`'s `accent_color` token to all libadwaita apps automatically. Developers don't need to hard-code accent colors — they inherit the user's choice.

---

### 4.2 Extensions

Arbor Shell supports extensions. Extensions are sandboxed differently from themes — they execute JavaScript in the shell process and have access to a **stable, versioned Arbor Extension API**, not the raw GNOME Shell internal API.

**Why a custom API (not raw GNOME Shell)?**

GNOME Shell extensions historically break on every GNOME version because they access internal APIs with no stability guarantee. Arbor's Extension API is:
- Documented and versioned (`api_version: 1`).
- Stable across Arbor OS minor releases.
- Enforced — extensions that try to access internals outside the API are rejected at load time.
- Smaller surface area — covers the 90% use case without exposing footguns.

**Extension API surface (v1):**

```
arbor.panel         — add items to the top bar (icons, menus, indicators)
arbor.quicksettings — add toggles to the Quick Settings panel
arbor.search        — add search result providers to the app launcher
arbor.notifications — intercept/transform notification display
arbor.shortcuts     — register global keyboard shortcuts
arbor.windows       — observe window events (open/close/focus), no manipulation
arbor.theme         — read current theme tokens (for visual consistency)
```

**Notably absent from v1 (intentionally):**
- Arbitrary window manipulation (security risk, breaks Wayland semantics).
- Access to other extensions' state.
- DOM/GJS internals not in the above list.

**Creating an extension:**

```bash
arbor devtools extension new "My Panel Widget"
# Generates:
# my-panel-widget@com.example/
# ├── extension.json    # metadata: name, description, api_version, uuid
# ├── extension.js      # main entry point
# ├── prefs.js          # optional preferences UI
# └── stylesheet.css    # optional CSS

arbor devtools extension run     # Load in a nested Arbor Shell session
arbor devtools extension lint    # API usage validation
arbor devtools extension package # Package as .arborext
```

`extension.json`:
```json
{
  "uuid": "my-panel-widget@com.example",
  "name": "My Panel Widget",
  "description": "Shows the current weather in the panel.",
  "api_version": 1,
  "arbor_version_min": "1.0",
  "permissions": ["arbor.panel", "arbor.shortcuts"],
  "settings_schema": "com.example.my-panel-widget"
}
```

**Extension permissions:** An extension must declare which API namespaces it uses in `extension.json`. The system only grants access to declared namespaces. Users see the permission list before installing.

**Extension distribution:**
- Primary: **Arbor Extension Store** (tab within Arbor Store, same verification pipeline).
- Secondary: sideloading via `arbor devtools extension install my-panel-widget@com.example.arborext`.

---

### 4.3 Modifying the System

Arbor's immutable base means `sudo rm /usr/bin/bash` is not a thing developers need to worry about — or do to themselves. System modification is done through three layers:

#### Layer 1: User-space (No Privileges Needed)

Everything in `~/.local/` and `~/.config/` is user-writable and immediately effective:
- Shell configs, dotfiles.
- Themes and extensions (see above).
- Distrobox containers.
- `mise`-managed runtimes.
- Flatpak apps.

This covers ~95% of developer customization needs.

#### Layer 2: System Layering (Requires Confirmation, Fully Reversible)

For packages that genuinely need system-level integration (kernel modules, custom drivers, system-wide daemons), `rpm-ostree` layering is the mechanism:

```bash
# Layer a system package (staged, takes effect after reboot)
sudo rpm-ostree install wireshark-cli
sudo rpm-ostree install kernel-modules-extra

# Remove a layered package
sudo rpm-ostree uninstall wireshark-cli

# View current layered packages
rpm-ostree status
```

**Arbor UX around layering:**
- Any `rpm-ostree install` from the CLI shows a warning: "This modifies the system image. A rollback is always available from the boot menu."
- The Settings → System → Image panel shows the current base image + all layered packages as a diff-style view.
- Layering triggers a background image rebuild. The new deployment boots automatically on next reboot (not forced) with a toast notification: "System update ready — [Reboot later] [Reboot now]".

#### Layer 3: Custom OS Images (Full Control, DevOps-Grade)

Advanced users and organizations can build their own Arbor-based OS images using `bootc`:

```Dockerfile
# ArborOS.containerfile
FROM arboros/base:1.0

# Add your organization's tools
RUN rpm-ostree install \
    wireshark \
    nmap \
    custom-vpn-client && \
    rpm-ostree cleanup -m

# Add your organization's CA certificates
COPY my-org-ca.crt /etc/pki/ca-trust/source/anchors/
RUN update-ca-trust

# Stamp the image
LABEL org.opencontainers.image.description="My Org Workstation"
```

```bash
# Build the image
podman build -t my-org-arboros:latest .

# Push to your registry
podman push my-org-arboros:latest registry.my-org.com/arboros:latest

# Deploy to machines
bootc switch registry.my-org.com/arboros:latest
```

This is the **enterprise/team deployment model**: treat the OS like any other OCI image. The same CI/CD pipelines used for app containers can build and deliver OS updates. Full rollback is always available via the boot menu.

#### Declarative Dotfile Management

For personal system-wide consistency, Arbor supports **Home Manager-style declarative dotfiles** via `arbor devtools dotfiles`:

```bash
# Initialize dotfile management
arbor devtools dotfiles init

# Links ~/.config/arbor-dotfiles/configs/ to their target locations
# Managed as a git repo, committed configuration changes apply on next login

arbor devtools dotfiles apply    # Apply current config
arbor devtools dotfiles diff     # Show uncommitted changes
arbor devtools dotfiles restore  # Roll back to last committed state
```

---

## 5. DEVELOPER SECURITY MODEL

Developers get full capability access without disabling the security model. This table summarizes how security features interact with developer workflows:

| Security feature | Default behavior | Developer Mode behavior |
|---|---|---|
| **SELinux** | Enforcing | Still enforcing; Distrobox containers run in `unconfined_u` context by default (escape hatch); custom policy modules can be loaded |
| **Flatpak sandbox** | Full isolation | IDE portal extensions provide host filesystem access where needed; explicitly granted per-project |
| **Firewall** | Deny by default | Developer Mode adds rules for common dev ports (3000, 8080, 5432, etc.) on loopback; external exposure is always explicit |
| **RPM signing** | Verified only | Unsigned local builds can be installed with explicit override; never automatic |
| **Rootless containers** | Rootless Podman | Privileged containers available via `--privileged` flag with a confirmation prompt; logged to audit log |
| **SSH agent** | GNOME Keyring | Developer Mode configures `SSH_AUTH_SOCK` to GNOME Keyring automatically; TPM-backed key storage for hardware SSH keys |

**Developer audit log:** All privilege-escalation events (sudo, rpm-ostree install, privileged container creation) are logged to `~/.local/share/arbor/dev-audit.log`. Viewable in Settings → Developer → Activity Log. Off by default; enabled with Developer Mode (opt-out available).

---

## 6. ARBOR DEVTOOLS CLI REFERENCE

The `arbor devtools` command is the single entry point for all developer-facing tooling. Subcommands:

```
arbor devtools setup              Run the developer setup wizard
arbor devtools new [app|cli|lib]  Scaffold a new project
arbor devtools build              Build the current project (Flatpak / Meson / Cargo / etc.)
arbor devtools run                Build and run locally in sandbox
arbor devtools lint               Run all linters (sandboxing, metainfo, permissions, a11y)
arbor devtools publish            App publishing subcommands
  arbor devtools publish submit   Submit to Arbor Store
  arbor devtools publish update   Push a new version
  arbor devtools publish homebrew Generate Homebrew formula
arbor devtools sdk                SDK management
  arbor devtools sdk list         List available SDKs
  arbor devtools sdk install <x>  Install an SDK
  arbor devtools sdk update       Update all SDKs
arbor devtools k8s                Kubernetes tooling
  arbor devtools k8s create       Create local cluster
  arbor devtools k8s delete       Delete local cluster
  arbor devtools k8s ui           Open k9s
arbor devtools env                Environment management
  arbor devtools env up           Start project environment
  arbor devtools env down         Stop project environment
  arbor devtools env export       Export environment spec
arbor devtools theme              Theme tooling
arbor devtools extension          Extension tooling
arbor devtools docs               Documentation tooling
arbor devtools dotfiles           Dotfile management
arbor devtools snapshot           Btrfs snapshot shortcuts
```

`arbor devtools help <subcommand>` opens the relevant documentation page in the browser.

---

## 7. DEVELOPER ROADMAP ALIGNMENT

| Arbor Phase | Developer Experience Milestones |
|---|---|
| **Phase 1** (0–6 mo) | Distrobox pre-installed and working; Homebrew auto-configured; base git + Podman stack in image |
| **Phase 2** (6–14 mo) | Arbor Terminal 1.0; Developer Mode toggle in Settings; First-Run Wizard; VS Code / JetBrains Flatpak verified |
| **Phase 3** (12–20 mo) | `arbor devtools` CLI GA; App scaffolding + publishing pipeline; Arbor Store developer submissions open; Extension API v1 published |
| **Phase 4** (18–26 mo) | SELinux developer policy profiles; audit log; SDK Manager GUI; Arbor Pages static hosting; security review of Extension API |
| **Phase 5** (24–30 mo) | Developer documentation site 1.0; Arbor Docs generator; `arbor devtools k8s` GA; 1.0 developer release |

---

## 8. SUMMARY TABLE

| Area | Choice | Key Tool(s) |
|---|---|---|
| Terminal | Arbor Terminal (custom, GPU-accelerated) | Fish + Starship + atuin |
| Runtime management | mise (universal) | Python, Node, Go, Rust, Java, Ruby |
| Python | uv + ruff + mypy | managed via mise |
| Node.js | pnpm (default) | managed via mise |
| Container runtime | Rootless Podman (Docker-compatible) | podman, podman-compose |
| Container environments | Distrobox | Per-project containers, home-directory shared |
| Kubernetes (local) | k3d (Podman-backed) | kubectl, helm, k9s, stern |
| App format | Flatpak | flatpak-builder, GNOME Builder |
| Developer Mode | Single toggle in Settings | First-Run Wizard, arbor devtools CLI |
| SDK management | arbor devtools sdk | Delegates to mise / distrobox |
| Environment management | mise + direnv + Distrobox | .mise.toml per project |
| Theming | CSS overlay on libadwaita | palette.json token system |
| Extensions | Arbor Extension API v1 | Sandboxed JS, declared permissions |
| System modification | Layer 1 (user) → Layer 2 (rpm-ostree) → Layer 3 (bootc image) | Progressive, always reversible |
| Documentation | Arbor Docs + Arbor Pages | Static site generator, free hosting |
| Publishing | Arbor Store submission pipeline | Automated CI, tiered trust |

---

*This document is a design specification. Tool names, API surface, and feature scope are subject to revision during implementation. Cross-reference the Architecture Blueprint for base system decisions this specification depends on.*
