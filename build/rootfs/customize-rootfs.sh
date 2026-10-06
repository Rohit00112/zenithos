#!/bin/bash
# Zenith OS — Root Filesystem Customization
#
# This script runs INSIDE the chroot to configure the system.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set -euo pipefail

echo "[CHROOT] Customizing Zenith OS root filesystem..."

# ============================================================================
# Locale and timezone
# ============================================================================
echo "en_US.UTF-8 UTF-8" > /etc/locale.gen
locale-gen
echo "LANG=en_US.UTF-8" > /etc/default/locale

ln -sf /usr/share/zoneinfo/UTC /etc/localtime

# ============================================================================
# Hostname
# ============================================================================
echo "zenith" > /etc/hostname
cat > /etc/hosts << 'EOF'
127.0.0.1   localhost
127.0.1.1   zenith
::1         localhost ip6-localhost ip6-loopback
EOF

# ============================================================================
# Create default user
# ============================================================================
if ! id "zenith" &>/dev/null; then
    useradd -m -s /bin/bash -G sudo,audio,video,input,render,netdev zenith
    echo "zenith:zenith" | chpasswd
    echo "[CHROOT] Created user 'zenith' (password: zenith)"
fi

# Allow sudo without password for zenith user (dev convenience)
echo "zenith ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/zenith
chmod 440 /etc/sudoers.d/zenith

# ============================================================================
# Autologin to Wayland session (for development)
# ============================================================================
mkdir -p /etc/systemd/system/getty@tty1.service.d
cat > /etc/systemd/system/getty@tty1.service.d/autologin.conf << 'EOF'
[Service]
ExecStart=
ExecStart=-/sbin/agetty --autologin zenith --noclear %I $TERM
Type=idle
EOF

# Auto-start Zenith session on login
cat >> /home/zenith/.bashrc << 'BASHEOF'

# Auto-start Zenith desktop on TTY1
if [ -z "$DISPLAY" ] && [ -z "$WAYLAND_DISPLAY" ] && [ "$(tty)" = "/dev/tty1" ]; then
    if command -v zenith-session &>/dev/null; then
        exec zenith-session
    elif command -v zenith-compositor &>/dev/null; then
        exec zenith-compositor -s "foot"
    fi
fi
BASHEOF
chown zenith:zenith /home/zenith/.bashrc

# ============================================================================
# Enable services
# ============================================================================
systemctl enable NetworkManager 2>/dev/null || true
systemctl enable systemd-resolved 2>/dev/null || true

# Disable services we don't need
systemctl disable apt-daily.timer 2>/dev/null || true
systemctl disable apt-daily-upgrade.timer 2>/dev/null || true

# ============================================================================
# OS branding
# ============================================================================
cat > /etc/os-release << 'EOF'
NAME="Zenith OS"
VERSION="0.1.0 (Summit)"
ID=zenith
ID_LIKE=debian
VERSION_ID="0.1.0"
PRETTY_NAME="Zenith OS 0.1.0 (Summit)"
HOME_URL="https://zenith-os.dev"
BUG_REPORT_URL="https://github.com/zenith-os/zenith/issues"
SUPPORT_URL="https://zenith-os.dev/support"
VERSION_CODENAME=summit
EOF

cat > /etc/issue << 'EOF'

  ⬢ Zenith OS 0.1.0 (Summit)
  \n \l

EOF

cat > /etc/issue.net << 'EOF'
Zenith OS 0.1.0 (Summit)
EOF

# ============================================================================
# Cleanup
# ============================================================================
apt-get clean
rm -rf /var/cache/apt/archives/*
rm -rf /tmp/*

echo "[CHROOT] Customization complete"
