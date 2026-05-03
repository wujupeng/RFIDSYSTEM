#!/bin/bash

# RFID Asset Management System - One-Click Installation Script
# Usage: sudo ./install.sh

set -e

echo "=============================================="
echo "RFID Asset Management System v1.0 Installation"
echo "=============================================="

# Check if running as root
if [ "$(id -u)" != "0" ]; then
    echo "Error: This script must be run as root"
    exit 1
fi

# Update package list
echo "Updating package list..."
apt-get update -y

# Install dependencies
echo "Installing dependencies..."
apt-get install -y \
    build-essential \
    cmake \
    libprotobuf-dev \
    protobuf-compiler \
    libgrpc-dev \
    libgrpc++-dev \
    grpc-tools \
    libpqxx-dev \
    libssl-dev \
    libspdlog-dev \
    qt5-default \
    qttools5-dev-tools

# Create directories
echo "Creating directories..."
mkdir -p /opt/rfid-server
mkdir -p /var/log/rfid
mkdir -p /backup/rfid/daily
mkdir -p /backup/rfid/weekly

# Build server
echo "Building server..."
cd /opt/rfid-server
git clone https://github.com/your-repo/rfid-system.git .
mkdir -p build && cd build
cmake ..
make -j$(nproc)

# Install binary
echo "Installing binary..."
cp server /usr/local/bin/rfid-server

# Install systemd service
echo "Installing systemd service..."
cp ../deploy/rfid-server.service /etc/systemd/system/
systemctl daemon-reload
systemctl enable rfid-server

# Initialize database
echo "Initializing database..."
su - postgres -c "createdb rfid"
su - postgres -c "psql rfid < /opt/rfid-server/scripts/init_db.sql"

# Create backup cron job
echo "Setting up backup schedule..."
cp ../scripts/backup.sh /usr/local/bin/rfid-backup.sh
chmod +x /usr/local/bin/rfid-backup.sh
cat ../scripts/crontab_backup | crontab -

# Create default config
echo "Creating default configuration..."
cat > /etc/rfid-server/config.ini << EOF
[database]
host = localhost
port = 5432
database = rfid
username = postgres
password = 
max_connections = 20
connection_timeout = 3000

[server]
port = 50051
max_concurrent_requests = 100
request_timeout_ms = 2000

[logging]
level = info
file = /var/log/rfid/server.log
max_size = 10485760
max_files = 30

[rfid]
max_epc_buffer_size = 10000
duplicate_window_ms = 1000
processing_interval_ms = 500
EOF

echo "=============================================="
echo "Installation completed successfully!"
echo "=============================================="
echo ""
echo "To start the service:"
echo "  systemctl start rfid-server"
echo ""
echo "To check status:"
echo "  systemctl status rfid-server"
echo ""
echo "Configuration file: /etc/rfid-server/config.ini"
echo "Log file: /var/log/rfid/server.log"