# Zenith OS — Kernel Configuration Notes

This directory contains kernel configuration and optional patches.

## Approach

Zenith OS uses the **upstream Debian kernel** (linux-image-amd64) in Phase 1-7.

Custom kernel compilation is a Phase 8 goal, primarily for:
- Removing unnecessary modules to reduce boot time
- Enabling specific security features (lockdown mode, etc.)
- Optimizing for the Zenith desktop workload

## Kernel Requirements

The following kernel features must be enabled:

- **Wayland/DRM:** `CONFIG_DRM`, `CONFIG_DRM_KMS_HELPER`
- **Btrfs:** `CONFIG_BTRFS_FS`, `CONFIG_BTRFS_FS_POSIX_ACL`
- **Namespaces:** `CONFIG_NAMESPACES`, `CONFIG_USER_NS`
- **Cgroups v2:** `CONFIG_CGROUP_*`
- **AppArmor:** `CONFIG_SECURITY_APPARMOR`
- **Seccomp:** `CONFIG_SECCOMP`, `CONFIG_SECCOMP_FILTER`
- **nftables:** `CONFIG_NF_TABLES`
- **KVM:** `CONFIG_KVM`, `CONFIG_KVM_INTEL`, `CONFIG_KVM_AMD`
- **Virtio:** `CONFIG_VIRTIO_*` (for VM testing)

All of these are enabled in the default Debian kernel.

## Files

- `config-zenith` — Custom kernel .config (Phase 8, not yet created)
- `patches/` — Kernel patches (none currently needed)
- `modules/` — Custom kernel modules (none currently needed)
