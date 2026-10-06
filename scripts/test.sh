#!/bin/bash
# Zenith OS — Run Tests
# SPDX-License-Identifier: GPL-3.0-or-later
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")/.."
make test
