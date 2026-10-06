#!/bin/bash
# Zenith OS — VM Launch Wrapper
# SPDX-License-Identifier: GPL-3.0-or-later

set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")/.."
exec bash build/vm/run-vm.sh "$@"
