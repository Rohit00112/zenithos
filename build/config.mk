# Zenith OS Build Configuration

# Project info
PROJECT_NAME := zenith-os
VERSION := 0.1.0
CODENAME := summit

# Build type: debug, release, debugoptimized
BUILD_TYPE := debugoptimized

# Cargo flags
ifeq ($(BUILD_TYPE),release)
    CARGO_FLAGS := --release
else
    CARGO_FLAGS :=
endif

# Architecture
ARCH := x86_64

# Directories
DIST_DIR := $(PWD)/dist
ROOTFS_DIR := $(PWD)/build/rootfs/rootfs
ISO_NAME := $(PROJECT_NAME)-$(VERSION)-$(ARCH).iso

# Debian base
DEBIAN_SUITE := bookworm
DEBIAN_MIRROR := http://deb.debian.org/debian

# Kernel
KERNEL_VERSION := 6.1

# Root filesystem size
ROOTFS_SIZE := 4G

# QEMU
QEMU_RAM := 2G
QEMU_CPUS := 2
QEMU_DISK_SIZE := 20G
OVMF_PATH := /usr/share/OVMF/OVMF_CODE.fd
