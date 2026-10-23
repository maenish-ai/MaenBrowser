#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")"
python3 integrate_maenbrowser.py --clone --output-dir ./EXPORT
printf '\nPrepared under EXPORT. Run tests and review before Git commit/push.\n'
