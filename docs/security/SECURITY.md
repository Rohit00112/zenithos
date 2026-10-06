# Zenith OS — Security Policy

## Security Philosophy

Zenith OS follows these security principles:

1. **Secure by default** — The base installation should be secure without user intervention
2. **Least privilege** — Every process runs with the minimum privileges needed
3. **Defense in depth** — Multiple layers of security (AppArmor, seccomp, namespaces, nftables)
4. **Transparency** — Security status is always visible to the user
5. **No security theater** — Every security indicator reflects real system state

## Security Layers

### Kernel Level
- **AppArmor** — Mandatory Access Control for applications
- **seccomp** — System call filtering for sandboxed applications
- **namespaces** — Process isolation (mount, network, PID, user)
- **cgroups** — Resource limits
- **netfilter/nftables** — Network packet filtering

### System Level
- **polkit** — Authorization for privileged operations
- **systemd** — Service sandboxing (PrivateTmp, ProtectSystem, etc.)
- **LUKS** — Full disk encryption
- **Btrfs checksums** — Data integrity verification

### Application Level
- **Flatpak portals** — Controlled access to host resources
- **AppArmor profiles** — Per-application access control
- **Permission prompts** — User approval for sensitive operations

## Default Security Configuration

### Firewall (nftables)
- **Default policy:** DROP incoming, ACCEPT outgoing
- **Allowed inbound:** DHCP, mDNS, established connections
- **SSH:** Disabled by default

### AppArmor
- All Zenith applications have enforce-mode profiles
- Third-party applications use complain-mode profiles initially

### User Accounts
- Default user is in `sudo` group but requires password
- Guest sessions have no persistent storage
- Root login is disabled by default

## Reporting Security Issues

Please report security vulnerabilities privately.
Do NOT create public issues for security bugs.

Contact: security@zenith-os.dev (placeholder)
