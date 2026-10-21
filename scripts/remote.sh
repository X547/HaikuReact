#!/bin/sh
# Develop on another machine, build and run on Haiku over ssh.
#
# Usage: scripts/remote.sh <command> [args]
#   sync             upload sources (no node_modules/build output)
#   install          sync + npm install
#   build            sync + native and TypeScript build
#   test             build + run tests
#   run <script.js>  build + run a compiled script, e.g. dist/examples/hello.js
#   exec <command>   run a shell command in the remote project directory
#
# Configuration (environment, or local/remote.env which is not committed):
#   HAIKU_HOST  ssh destination, e.g. user@haiku-vm
#   HAIKU_DIR   project directory on the remote machine, relative to home

set -e

root=$(cd "$(dirname "$0")/.." && pwd)
if [ -f "$root/local/remote.env" ]; then
	. "$root/local/remote.env"
fi
: "${HAIKU_HOST:?set HAIKU_HOST or create local/remote.env}"
: "${HAIKU_DIR:?set HAIKU_DIR or create local/remote.env}"

remote() {
	ssh -o BatchMode=yes "$HAIKU_HOST" "cd '$HAIKU_DIR' && $1"
}

sync() {
	ssh -o BatchMode=yes "$HAIKU_HOST" "mkdir -p '$HAIKU_DIR'"
	tar -C "$root" -czf - \
		--exclude=./node_modules \
		--exclude=./build \
		--exclude=./dist \
		--exclude=./spike/build \
		--exclude=./local \
		--exclude=./.git \
		. | ssh -o BatchMode=yes "$HAIKU_HOST" "tar -C '$HAIKU_DIR' -xzf -"
}

build() {
	sync
	remote "npm run build"
}

command=${1:-}
[ $# -gt 0 ] && shift

case "$command" in
	sync)
		sync
		;;
	install)
		sync
		remote "npm install --no-audit --no-fund"
		;;
	build)
		build
		;;
	test)
		build
		remote "npm test"
		;;
	run)
		build
		remote "node $*"
		;;
	exec)
		remote "$*"
		;;
	*)
		sed -n '2,15p' "$0"
		exit 1
		;;
esac
