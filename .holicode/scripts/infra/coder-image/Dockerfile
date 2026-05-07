FROM ubuntu:24.04

# Declared early so the conditional build-tools layer can branch on it
ARG VK_SOURCE=npm
ENV DEBIAN_FRONTEND=noninteractive
ENV HOME=/home/coder

# Base tooling — lean, always installed
# bubblewrap: required by Claude Code /sandbox and Codex CLI sandbox on Linux
# shellcheck/make/tree/fzf/tmux/sqlite3: agents call these unprompted; not in stock Ubuntu minimal
# xz-utils/p7zip-full/pv: compression + pipe progress agents reach for
RUN apt-get update && apt-get install -y --no-install-recommends \
    curl wget git ca-certificates openssh-client sudo \
    rsync jed jq socat netcat-openbsd ripgrep fd-find unzip zip \
    openssl dnsutils iputils-ping less lsof tini \
    python3 python3-pip postgresql-client \
    bubblewrap shellcheck make tree fzf tmux sqlite3 \
    xz-utils p7zip-full pv cron gawk \
    && rm -rf /var/lib/apt/lists/* \
    && ln -sf "$(command -v fdfind)" /usr/local/bin/fd

# yq (Go, mikefarah) — binary download. Apt's `yq` is the deprecated Python wrapper,
# not the Go tool agents expect.
ARG YQ_VERSION=4.47.1
RUN curl -fsSL -o /usr/local/bin/yq \
    "https://github.com/mikefarah/yq/releases/download/v${YQ_VERSION}/yq_linux_$(dpkg --print-architecture)" \
    && chmod +x /usr/local/bin/yq

# Build tools — only for VK_SOURCE=source (Rust compilation)
# Skipped in npm/prebuilt mode: saves ~450MB (build-essential + clang + libclang-dev)
RUN if [ "${VK_SOURCE}" = "source" ]; then \
    apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    pkg-config libssl-dev libclang-dev clang libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*; \
    fi

# Node.js 24 — download directly from nodejs.org (not NodeSource).
# Node 22.22.2 (both NodeSource and nodejs.org tarballs) ships npm 10.9.7 which
# has a broken arm64 bundling: promise-retry is missing from npm/node_modules/,
# making ALL npm commands fail with MODULE_NOT_FOUND. Node 24 ships npm 11.x
# which does not have this issue.
ARG NODE_VERSION=24.15.0
RUN NODE_ARCH="$(dpkg --print-architecture | sed 's/amd64/x64/')" && \
    curl -fsSL "https://nodejs.org/dist/v${NODE_VERSION}/node-v${NODE_VERSION}-linux-${NODE_ARCH}.tar.gz" \
    | tar xz -C /usr/local --strip-components=1

# Global npm tools — clean cache immediately to avoid ~1GB bloat in this layer
RUN npm install -g pnpm@10.33.1 opencode-ai @openai/codex @google/gemini-cli \
    && npm cache clean --force

# GitHub CLI
RUN curl -fsSL https://cli.github.com/packages/githubcli-archive-keyring.gpg \
    | dd of=/usr/share/keyrings/githubcli-archive-keyring.gpg \
    && echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" \
    | tee /etc/apt/sources.list.d/github-cli.list > /dev/null \
    && apt-get update && apt-get install -y gh && rm -rf /var/lib/apt/lists/*

# Docker Engine — full install for Sysbox DinD (inner dockerd); also works for DooD (socket mount).
# In DooD mode: only the CLI is used (dockerd never starts). In Sysbox mode: inner dockerd runs.
RUN apt-get update && apt-get install -y --no-install-recommends gnupg \
    && curl -fsSL https://download.docker.com/linux/ubuntu/gpg \
    | gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg \
    && echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] \
    https://download.docker.com/linux/ubuntu $(. /etc/os-release && echo $VERSION_CODENAME) stable" \
    | tee /etc/apt/sources.list.d/docker.list > /dev/null \
    && apt-get update && apt-get install -y --no-install-recommends \
    docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin \
    && rm -rf /var/lib/apt/lists/*

# 1Password CLI — required for `op` service-account token consumption in workspaces
RUN curl -sS https://downloads.1password.com/linux/keys/1password.asc \
    | gpg --dearmor -o /usr/share/keyrings/1password-archive-keyring.gpg \
    && echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/1password-archive-keyring.gpg] https://downloads.1password.com/linux/debian/$(dpkg --print-architecture) stable main" \
    | tee /etc/apt/sources.list.d/1password-cli.list > /dev/null \
    && apt-get update && apt-get install -y --no-install-recommends 1password-cli \
    && rm -rf /var/lib/apt/lists/* \
    && which op && op --version

# Bitwarden CLI — alternative password vault for users on Bitwarden instead of 1P
RUN npm install -g @bitwarden/cli && npm cache clean --force \
    && which bw && bw --version

# Go toolchain — official tarball from go.dev/dl; installs to /usr/local/go
ARG GO_VERSION=1.26.2
RUN curl -fsSL "https://go.dev/dl/go${GO_VERSION}.linux-$(dpkg --print-architecture).tar.gz" \
    | tar xz -C /usr/local
ENV PATH="/usr/local/go/bin:${PATH}"

# Tailscale — mesh networking; official install script sets up apt repo
RUN curl -fsSL https://tailscale.com/install.sh | sh

# cloudflared — Cloudflare Tunnels client (deb from GitHub releases; arch-neutral)
RUN curl -fsSL -o /tmp/cloudflared.deb \
    "https://github.com/cloudflare/cloudflared/releases/latest/download/cloudflared-linux-$(dpkg --print-architecture).deb" \
    && dpkg -i /tmp/cloudflared.deb \
    && rm /tmp/cloudflared.deb

# wrangler — Cloudflare Workers/Pages/R2/D1 CLI
RUN npm install -g wrangler && npm cache clean --force

# Supabase CLI — pinned binary; architecture-neutral via dpkg --print-architecture
ARG SUPABASE_VERSION=2.90.0
RUN curl -fsSL "https://github.com/supabase/cli/releases/download/v${SUPABASE_VERSION}/supabase_linux_$(dpkg --print-architecture).tar.gz" \
    | tar xz -C /usr/local/bin supabase \
    && chmod +x /usr/local/bin/supabase

# Coder CLI
RUN curl -fsSL https://coder.com/install.sh | sh -s -- --method standalone --prefix /usr/local

# Shell ergonomics — prompt, history, and directory navigation
# All tools are static binaries; init is gated on interactive shells so
# non-interactive agent subshells (bash -c, build hooks) are unaffected.
ARG STARSHIP_VERSION=1.21.1
RUN ARCH="$(dpkg --print-architecture | sed 's/amd64/x86_64/; s/arm64/aarch64/')" && \
    curl -fsSL "https://github.com/starship/starship/releases/download/v${STARSHIP_VERSION}/starship-${ARCH}-unknown-linux-musl.tar.gz" \
    | tar xz -C /usr/local/bin

ARG ATUIN_VERSION=18.3.0
RUN ARCH="$(dpkg --print-architecture | sed 's/amd64/x86_64/; s/arm64/aarch64/')" && \
    mkdir /tmp/atuin-dl && \
    curl -fsSL "https://github.com/atuinsh/atuin/releases/download/v${ATUIN_VERSION}/atuin-${ARCH}-unknown-linux-musl.tar.gz" \
        | tar xz -C /tmp/atuin-dl && \
    find /tmp/atuin-dl -name atuin -type f | xargs -I{} install -m755 {} /usr/local/bin/atuin && \
    rm -rf /tmp/atuin-dl

ARG ZOXIDE_VERSION=0.9.4
RUN ARCH="$(dpkg --print-architecture | sed 's/amd64/x86_64/; s/arm64/aarch64/')" && \
    mkdir /tmp/zoxide-dl && \
    curl -fsSL "https://github.com/ajeetdsouza/zoxide/releases/download/v${ZOXIDE_VERSION}/zoxide-${ZOXIDE_VERSION}-${ARCH}-unknown-linux-musl.tar.gz" \
        | tar xz -C /tmp/zoxide-dl && \
    install -m755 /tmp/zoxide-dl/zoxide /usr/local/bin/zoxide && \
    rm -rf /tmp/zoxide-dl

# ble.sh — Bash inline ghost-text (Tier 2, opt-in via HOLICODE_SHELL_BLESH=1 in ~/.bashrc)
ARG BLESH_VERSION=0.4.0-devel3
RUN git clone --depth 1 --branch "v${BLESH_VERSION}" \
        https://github.com/akinomyoga/ble.sh.git /tmp/blesh && \
    make -C /tmp/blesh install PREFIX=/usr/local && \
    rm -rf /tmp/blesh

# Shell init script — placed in a neutral library location, NOT /etc/profile.d.
# Rationale: /etc/profile.d/*.sh fires before ~/.bashrc on login shells, which
# would init the tools BEFORE user-level HOLICODE_SHELL_<TOOL>=0 opt-outs run.
# The seed coder_script appends a marker block to ~/.bashrc that sources this
# file exclusively — single canonical path, runs after user overrides.
COPY files/holicode-shell.sh /usr/local/share/holicode/holicode-shell.sh

# Default configs seeded into /etc/skel — coder_script.shell_dotfile_seed
# copies these to ~/.config/ on first workspace start (Docker volumes don't
# auto-populate from the image, so skel alone is not sufficient).
RUN mkdir -p /etc/skel/.config/atuin
COPY files/starship.toml     /etc/skel/.config/starship.toml
COPY files/atuin-config.toml /etc/skel/.config/atuin/config.toml

# NOTE: we deliberately do NOT install holicode-shell.sh under /etc/profile.d/
# and do NOT modify /etc/bash.bashrc. On Ubuntu, /etc/profile sources both
# /etc/bash.bashrc and /etc/profile.d/*.sh BEFORE ~/.bashrc — either path
# would fire init before user-level HOLICODE_SHELL_<TOOL>=0 overrides run.
# Instead, the coder_script.shell_dotfile_seed in the Coder template appends
# a marker block to ~/.bashrc that sources the script from its neutral
# library path. This guarantees user overrides placed above the marker take
# effect for both login and non-login interactive shells.

# GitHub App → gh CLI auth script (mints installation token from App credentials)
COPY files/github-app-gh-auth.sh /usr/local/bin/github-app-gh-auth

# Bake image version into a file readable at runtime (shown in Coder workspace metadata)
ARG CODER_IMAGE_TAG=dev
RUN echo "${CODER_IMAGE_TAG}" > /etc/coder-image-version

# docker group — update GID to match host so 'groups' resolves the name correctly.
# docker-ce package creates the group during install; use groupmod (not groupadd) to set the GID.
ARG DOCKER_GID=988
RUN groupmod -g ${DOCKER_GID} docker

# Workspace user — add to docker group so coder can use /var/run/docker.sock without sudo
# (works in both DooD mode and Sysbox mode where inner dockerd owns the socket)
RUN useradd -m -s /bin/bash -d $HOME coder \
    && echo "coder ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers.d/coder \
    && usermod -aG docker coder

RUN chown -R coder:coder $HOME

USER coder
WORKDIR $HOME

# Git defaults
RUN git config --global init.defaultBranch main \
    && git config --global pull.rebase true

# Claude Code — native binary installer (installs to ~/.local/bin)
RUN curl -fsSL https://claude.ai/install.sh | bash
ENV PATH="${PATH}:${HOME}/.local/bin"

# migra — PostgreSQL schema diff tool
RUN pip3 install --user --no-cache-dir "migra[pg]" --break-system-packages

# Hugging Face CLI — ML model/dataset ops (provides `hf` and `huggingface-cli`)
RUN pip3 install --user --no-cache-dir "huggingface_hub[cli]" --break-system-packages

# bun — fast JS runtime/bundler/package manager (per-user install to ~/.bun/bin)
RUN curl -fsSL https://bun.sh/install | bash

# deno — alternative JS/TS runtime (per-user install to ~/.deno/bin)
RUN curl -fsSL https://deno.land/install.sh | sh

ENV PATH="${PATH}:${HOME}/.bun/bin:${HOME}/.deno/bin"

# Vibe Kanban CLI — install from npm, pre-built tarball, or build from source
# VK_SOURCE=npm       → npm install from registry (fast, ~30s, official upstream)
# VK_SOURCE=prebuilt  → download tarball from GitHub Release (fast, ~30s)
# VK_SOURCE=source    → clone + build from source (slow, ~5-8min, always works)
ARG VK_GIT_REF=v0.1.43
ARG VK_API_BASE=https://vk-remote.holagence.com
ARG VK_REPO=https://github.com/holagence/vibe-kanban.git
ARG GITHUB_REPO=holagence/holicode
# GH_TOKEN required to download release asset from private repo
ARG GH_TOKEN=

RUN set -e && \
    VK_VERSION="${VK_GIT_REF#v}" && \
    if [ "${VK_SOURCE}" = "npm" ]; then \
        echo "Installing vibe-kanban from npm registry (v${VK_VERSION})..." && \
        sudo npm install -g "vibe-kanban@${VK_VERSION}"; \
    elif [ "${VK_SOURCE}" = "prebuilt" ]; then \
        echo "Installing vibe-kanban from pre-built tarball (${VK_GIT_REF})..." && \
        GITHUB_TOKEN="${GH_TOKEN}" gh release download "vk-prebuilt-${VK_VERSION}" \
            --repo "${GITHUB_REPO}" \
            --pattern "vibe-kanban-${VK_VERSION}.tgz" \
            --dir /tmp && \
        sudo npm install -g "/tmp/vibe-kanban-${VK_VERSION}.tgz" && \
        rm "/tmp/vibe-kanban-${VK_VERSION}.tgz"; \
    else \
        echo "Building vibe-kanban from source (${VK_GIT_REF}) — slow path, no Rust cache..." && \
        \
        # Install Rust toolchain inline (only for source builds; not kept in image for prebuilt)
        sudo curl https://sh.rustup.rs -sSf | sudo sh -s -- -y --no-modify-path && \
        export PATH="/root/.cargo/bin:${PATH}" && \
        sudo /root/.cargo/bin/rustup toolchain install nightly-2025-12-04 && \
        \
        git clone --depth 1 --branch "${VK_GIT_REF}" "${VK_REPO}" /tmp/vibe-kanban && \
        cd /tmp/vibe-kanban && \
        pnpm install --frozen-lockfile && \
        \
        # 1. Build frontend with custom API base
        cd frontend && \
        NODE_OPTIONS="--max-old-space-size=8192" \
            VITE_VK_SHARED_API_BASE="${VK_API_BASE}" \
            npm run build && \
        cd .. && \
        \
        # 2. Build Rust server binary (embeds frontend/dist at compile time)
        /root/.cargo/bin/cargo build --release --bin server && \
        /root/.cargo/bin/cargo build --release --bin vibe-kanban-mcp && \
        /root/.cargo/bin/cargo build --release --bin vibe-kanban-review && \
        \
        # 3. Package assembly — use Node.js arch names (x64, arm64) to match getPlatformDir()
        case "$(dpkg --print-architecture)" in \
          amd64) VK_ARCH="linux-x64" ;; \
          arm64) VK_ARCH="linux-arm64" ;; \
          *) echo "Unsupported arch: $(dpkg --print-architecture)" >&2; exit 1 ;; \
        esac && \
        mkdir -p "npx-cli/dist/${VK_ARCH}" && \
        mkdir -p /tmp/vk-pack && \
        cp target/release/server /tmp/vk-pack/vibe-kanban && \
        cd /tmp/vk-pack && zip -j "/tmp/vibe-kanban/npx-cli/dist/${VK_ARCH}/vibe-kanban.zip" vibe-kanban && \
        cp /tmp/vibe-kanban/target/release/vibe-kanban-mcp /tmp/vk-pack/ && \
        zip -j "/tmp/vibe-kanban/npx-cli/dist/${VK_ARCH}/vibe-kanban-mcp.zip" vibe-kanban-mcp && \
        cp /tmp/vibe-kanban/target/release/vibe-kanban-review /tmp/vk-pack/ && \
        zip -j "/tmp/vibe-kanban/npx-cli/dist/${VK_ARCH}/vibe-kanban-review.zip" vibe-kanban-review && \
        rm -rf /tmp/vk-pack && \
        \
        # 4. Pack and install
        cd /tmp/vibe-kanban/npx-cli && \
        TARBALL=$(pnpm pack) && \
        sudo npm install -g "$TARBALL" && \
        \
        # Cleanup — remove Rust toolchain to keep image lean
        cd / && \
        rm -rf /tmp/vibe-kanban /root/.cargo /root/.rustup; \
    fi && \
    # For prebuilt/source paths the package ships zip files that must be pre-extracted
    # as root (coder user has no write access to /usr/local/lib/node_modules/).
    # The npm path uses R2-downloaded binaries (pre-downloaded above); no zips to extract.
    if [ "${VK_SOURCE}" != "npm" ]; then \
        case "$(dpkg --print-architecture)" in \
          amd64) _VK_ARCH="linux-x64" ;; \
          arm64) _VK_ARCH="linux-arm64" ;; \
          *) echo "Unsupported arch" >&2; exit 1 ;; \
        esac && \
        VK_DIST="/usr/local/lib/node_modules/vibe-kanban/dist/${_VK_ARCH}" && \
        sudo unzip -o "${VK_DIST}/vibe-kanban.zip" -d "${VK_DIST}" && \
        sudo unzip -o "${VK_DIST}/vibe-kanban-mcp.zip" -d "${VK_DIST}" && \
        sudo unzip -o "${VK_DIST}/vibe-kanban-review.zip" -d "${VK_DIST}" && \
        sudo chmod +x "${VK_DIST}/vibe-kanban" "${VK_DIST}/vibe-kanban-mcp" "${VK_DIST}/vibe-kanban-review"; \
    fi && \
    echo "Vibe Kanban installed (${VK_SOURCE} mode)"

# Smoke test — fail the build loudly if any tool is missing or not on PATH.
# This prevents silent "tool installed but not found" issues like the op CLI gap.
RUN set -e && \
    echo "=== smoke test ===" && \
    op --version && \
    bw --version && \
    bwrap --version && \
    shellcheck --version && \
    make --version && \
    tree --version && \
    fzf --version && \
    yq --version && \
    fd --version && \
    tmux -V && \
    sqlite3 --version && \
    xz --version | head -1 && \
    7z --help >/dev/null && \
    pv --version | head -1 && \
    go version && \
    tailscale --version && \
    hf version && \
    wrangler --version && \
    cloudflared --version && \
    bun --version && \
    deno --version && \
    pnpm --version && \
    claude --version && \
    gh --version && \
    supabase --version && \
    docker --version && \
    docker compose version && \
    node --version && \
    command -v vibe-kanban && \
    command -v opencode && \
    command -v codex && \
    command -v gemini && \
    command -v coder && \
    command -v migra && \
    command -v psql && \
    command -v cron && \
    bash -n /usr/local/bin/github-app-gh-auth && \
    starship --version && \
    atuin --version && \
    zoxide --version && \
    test -f /usr/local/share/blesh/ble.sh && \
    test -f /usr/local/share/holicode/holicode-shell.sh && \
    bash -n /usr/local/share/holicode/holicode-shell.sh && \
    test -f /etc/skel/.config/starship.toml && \
    test -f /etc/skel/.config/atuin/config.toml && \
    # Behavioral test: sourcing the init script in a non-interactive shell
    # MUST return immediately without setting PROMPT_COMMAND (atuin/starship
    # would set it on init). Catches any regression of the interactive guard.
    [ "$(bash -c '. /usr/local/share/holicode/holicode-shell.sh; printf %s "$PROMPT_COMMAND"')" = "" ] && \
    # Negative test: profile.d MUST NOT contain the init script — its presence
    # there would re-introduce the login-shell race against ~/.bashrc overrides.
    [ ! -e /etc/profile.d/holicode-shell.sh ] && \
    echo "=== all tools OK ==="

EXPOSE 3000 4096 5173

ENTRYPOINT ["tini", "-g", "--"]
