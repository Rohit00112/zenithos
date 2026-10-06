#!/bin/bash
# Zenith OS — VM Configuration
# Sourced by VM scripts for common configuration.
# SPDX-License-Identifier: GPL-3.0-or-later

export QEMU_RAM="${QEMU_RAM:-2G}"
export QEMU_CPUS="${QEMU_CPUS:-2}"
export QEMU_DISK_SIZE="${QEMU_DISK_SIZE:-20G}"
export OVMF_PATH="${OVMF_PATH:-/usr/share/OVMF/OVMF_CODE.fd}"
