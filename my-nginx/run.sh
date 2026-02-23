#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

PORT="${PORT:-8081}"
CONF="${CONF:-conf/test.conf}"
MODULE_DIR="${MODULE_DIR:-modules/ngx_http_x_cache_key}"
EXTRA_CONFIGURE_ARGS="${EXTRA_CONFIGURE_ARGS:-}"

echo "==> Killing listeners on port $PORT (tcp)..."
PIDS="$(lsof -nP -iTCP:"$PORT" -sTCP:LISTEN -t 2>/dev/null || true)"

if [[ -n "${PIDS}" ]]; then
  echo "Found PIDs: ${PIDS}"
  kill ${PIDS} 2>/dev/null || true
  sleep 0.3

  STILL="$(lsof -nP -iTCP:"$PORT" -sTCP:LISTEN -t 2>/dev/null || true)"
  if [[ -n "${STILL}" ]]; then
    echo "Still listening, killing -9: ${STILL}"
    kill -9 ${STILL} 2>/dev/null || true
  fi
else
  echo "No listeners on port $PORT."
fi

echo "==> Preparing logs/ (for -p prefix run)..."
mkdir -p logs
touch logs/error.log logs/access.log

echo "==> Cleaning build (objs/)..."
rm -rf objs

echo "==> Configuring nginx (+ addon module: $MODULE_DIR)..."
./auto/configure --add-module="$MODULE_DIR" $EXTRA_CONFIGURE_ARGS

echo "==> Building..."
JOBS="$(sysctl -n hw.ncpu 2>/dev/null || nproc)"
make -j"$JOBS"

echo "==> Testing config: $CONF"
./objs/nginx -t -p "$ROOT" -c "$CONF"

echo "==> Starting nginx with $CONF"
if [[ "${FOREGROUND:-0}" == "1" ]]; then
  exec ./objs/nginx -p "$ROOT" -c "$CONF" -g "daemon off;"
else
  ./objs/nginx -p "$ROOT" -c "$CONF"
  echo "==> Started. Try: curl -I http://127.0.0.1:${PORT}/"
fi