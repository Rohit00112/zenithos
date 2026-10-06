# Zenith OS Build System
# Top-level Makefile for orchestrating all build steps

SHELL := /bin/bash
.ONESHELL:

include build/config.mk

# ============================================================================
# Default target
# ============================================================================

.PHONY: all
all: compositor panel session cli
	@echo "========================================="
	@echo " Zenith OS — All components built"
	@echo "========================================="

# ============================================================================
# Component builds
# ============================================================================

.PHONY: compositor
compositor:
	@echo "[BUILD] zenith-compositor"
	cd desktop/zenith-compositor && \
		meson setup builddir --prefix=/usr -Dbuildtype=$(BUILD_TYPE) 2>/dev/null || true && \
		meson compile -C builddir

.PHONY: panel
panel:
	@echo "[BUILD] zenith-panel"
	cd desktop/zenith-panel && \
		meson setup builddir --prefix=/usr -Dbuildtype=$(BUILD_TYPE) 2>/dev/null || true && \
		meson compile -C builddir

.PHONY: dock
dock:
	@echo "[BUILD] zenith-dock"
	cd desktop/zenith-dock && \
		meson setup builddir --prefix=/usr -Dbuildtype=$(BUILD_TYPE) 2>/dev/null || true && \
		meson compile -C builddir

.PHONY: launcher
launcher:
	@echo "[BUILD] zenith-launcher"
	cd desktop/zenith-launcher && \
		meson setup builddir --prefix=/usr -Dbuildtype=$(BUILD_TYPE) 2>/dev/null || true && \
		meson compile -C builddir

.PHONY: notifications
notifications:
	@echo "[BUILD] zenith-notifications"
	cd desktop/zenith-notifications && \
		meson setup builddir --prefix=/usr -Dbuildtype=$(BUILD_TYPE) 2>/dev/null || true && \
		meson compile -C builddir

.PHONY: wallpaper
wallpaper:
	@echo "[BUILD] zenith-wallpaper"
	cd desktop/zenith-wallpaper && \
		meson setup builddir --prefix=/usr -Dbuildtype=$(BUILD_TYPE) 2>/dev/null || true && \
		meson compile -C builddir

.PHONY: session
session:
	@echo "[BUILD] zenith-session"
	cd core/zenith-session && \
		meson setup builddir --prefix=/usr -Dbuildtype=$(BUILD_TYPE) 2>/dev/null || true && \
		meson compile -C builddir

.PHONY: cli
cli:
	@echo "[BUILD] osctl"
	cd cli && cargo build $(CARGO_FLAGS)

.PHONY: apps
apps: file-manager terminal system-monitor settings network-center

.PHONY: file-manager
file-manager:
	@echo "[BUILD] zenith-file-manager"
	cd apps/file-manager && \
		meson setup builddir --prefix=/usr -Dbuildtype=$(BUILD_TYPE) 2>/dev/null || true && \
		meson compile -C builddir

.PHONY: terminal
terminal:
	@echo "[BUILD] zenith-terminal"
	cd apps/terminal && \
		meson setup builddir --prefix=/usr -Dbuildtype=$(BUILD_TYPE) 2>/dev/null || true && \
		meson compile -C builddir

.PHONY: system-monitor
system-monitor:
	@echo "[BUILD] zenith-system-monitor"
	cd apps/system-monitor && \
		meson setup builddir --prefix=/usr -Dbuildtype=$(BUILD_TYPE) 2>/dev/null || true && \
		meson compile -C builddir

.PHONY: settings
settings:
	@echo "[BUILD] zenith-settings"
	cd apps/settings && \
		meson setup builddir --prefix=/usr -Dbuildtype=$(BUILD_TYPE) 2>/dev/null || true && \
		meson compile -C builddir

.PHONY: network-center
network-center:
	@echo "[BUILD] zenith-network-center"
	cd apps/network-center && \
		meson setup builddir --prefix=/usr -Dbuildtype=$(BUILD_TYPE) 2>/dev/null || true && \
		meson compile -C builddir

# ============================================================================
# Desktop (all shell components)
# ============================================================================

.PHONY: desktop
desktop: compositor panel dock launcher notifications wallpaper

# ============================================================================
# Root filesystem and ISO
# ============================================================================

.PHONY: rootfs
rootfs:
	@echo "[BUILD] Root filesystem"
	sudo bash build/rootfs/build-rootfs.sh

.PHONY: iso
iso: all rootfs
	@echo "[BUILD] ISO image"
	sudo bash build/iso/build-iso.sh
	@echo "========================================="
	@echo " ISO: $(DIST_DIR)/$(ISO_NAME)"
	@echo "========================================="

# ============================================================================
# VM testing
# ============================================================================

.PHONY: vm
vm:
	@echo "[VM] Launching QEMU..."
	bash build/vm/run-vm.sh

.PHONY: vm-uefi
vm-uefi:
	@echo "[VM] Launching QEMU (UEFI)..."
	bash build/vm/run-vm-uefi.sh

# ============================================================================
# Docker-based reproducible build
# ============================================================================

.PHONY: docker-build
docker-build:
	@echo "[DOCKER] Building in container..."
	docker build -t zenith-builder -f build/docker/Dockerfile.build .
	docker run --rm --privileged \
		-v $(PWD)/dist:/workspace/dist \
		zenith-builder make iso

# ============================================================================
# Testing
# ============================================================================

.PHONY: test
test: test-unit test-integration

.PHONY: test-unit
test-unit:
	@echo "[TEST] Unit tests"
	cd cli && cargo test
	# Add meson test commands for C components as they're implemented

.PHONY: test-integration
test-integration:
	@echo "[TEST] Integration tests"
	bash tests/integration/test_boot.sh

# ============================================================================
# Code quality
# ============================================================================

.PHONY: lint
lint:
	@echo "[LINT] Checking code..."
	cd cli && cargo clippy -- -D warnings
	# Add clang-tidy for C components as they're implemented

.PHONY: format
format:
	@echo "[FORMAT] Formatting code..."
	cd cli && cargo fmt
	# Add clang-format for C components as they're implemented

# ============================================================================
# Cleanup
# ============================================================================

.PHONY: clean
clean:
	@echo "[CLEAN] Removing build artifacts..."
	rm -rf desktop/zenith-compositor/builddir
	rm -rf desktop/zenith-panel/builddir
	rm -rf desktop/zenith-dock/builddir
	rm -rf desktop/zenith-launcher/builddir
	rm -rf desktop/zenith-notifications/builddir
	rm -rf desktop/zenith-wallpaper/builddir
	rm -rf core/zenith-session/builddir
	rm -rf apps/*/builddir
	cd cli && cargo clean 2>/dev/null || true
	@echo "[CLEAN] Done"

.PHONY: distclean
distclean: clean
	rm -rf $(DIST_DIR)
	rm -rf $(ROOTFS_DIR)

# ============================================================================
# Help
# ============================================================================

.PHONY: help
help:
	@echo "Zenith OS Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all             Build all components"
	@echo "  compositor      Build Wayland compositor"
	@echo "  panel           Build top panel"
	@echo "  dock            Build application dock"
	@echo "  launcher        Build application launcher"
	@echo "  desktop         Build all desktop shell components"
	@echo "  session         Build session manager"
	@echo "  cli             Build osctl CLI"
	@echo "  apps            Build all applications"
	@echo "  rootfs          Generate root filesystem (requires sudo)"
	@echo "  iso             Build bootable ISO (requires sudo)"
	@echo "  vm              Launch ISO in QEMU"
	@echo "  vm-uefi         Launch ISO in QEMU (UEFI mode)"
	@echo "  docker-build    Build ISO in Docker container"
	@echo "  test            Run all tests"
	@echo "  lint            Run linters"
	@echo "  format          Format code"
	@echo "  clean           Remove build artifacts"
	@echo "  distclean       Remove build artifacts and dist/"
	@echo "  help            Show this help"
