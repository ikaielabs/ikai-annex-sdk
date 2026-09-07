#!/usr/bin/env bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IKAINEX_ROOT="$(dirname "$SCRIPT_DIR")"

export IKAINEX_ROOT

echo "IKAINEX_ROOT=$IKAINEX_ROOT"