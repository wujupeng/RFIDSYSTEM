#!/bin/bash

set -e

echo "========================================"
echo "  RFID Server Initialization Script"
echo "========================================"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_ROOT"

echo ""
echo "[1/5] Checking prerequisites..."
command -v cmake >/dev/null 2>&1 || { echo "Error: cmake not found. Please install cmake."; exit 1; }
command -v make >/dev/null 2>&1 || { echo "Error: make not found. Please install build-essential."; exit 1; }

echo "[2/5] Setting up database..."
export RFID_DB_NAME="${RFID_DB_NAME:-rfid}"
export RFID_DB_USER="${RFID_DB_USER:-postgres}"
export RFID_DB_PASS="${RFID_DB_PASS:-123456}"
export RFID_DB_HOST="${RFID_DB_HOST:-localhost}"

if command -v createdb >/dev/null 2>&1; then
    createdb "$RFID_DB_NAME" 2>/dev/null || echo "Database '$RFID_DB_NAME' may already exist, continuing..."
    echo "Running init_db.sql..."
    psql "$RFID_DB_NAME" -f "$SCRIPT_DIR/init_db.sql" || echo "Warning: psql script failed, database may already be initialized"
else
    echo "Warning: PostgreSQL client not found. Please ensure database is manually set up."
fi

echo "[3/5] Creating logs directory..."
mkdir -p logs

echo "[4/5] Building server..."
cd "$PROJECT_ROOT/server"
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

echo ""
echo "========================================"
echo "  Build completed successfully!"
echo "========================================"
echo ""
echo "To start the server, run:"
echo "  ./build/server"
echo ""
echo "Environment variables:"
echo "  RFID_DB_NAME=$RFID_DB_NAME"
echo "  RFID_DB_USER=$RFID_DB_USER"
echo "  RFID_DB_HOST=$RFID_DB_HOST"
echo ""