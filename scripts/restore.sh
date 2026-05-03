#!/bin/bash

# RFID Asset Management System - Restore Script
# Usage: ./restore.sh <backup_file>

set -e

if [ -z "$1" ]; then
    echo "Usage: $0 <backup_file>"
    exit 1
fi

BACKUP_FILE="$1"
DB_NAME="rfid"

if [ ! -f "$BACKUP_FILE" ]; then
    echo "Error: Backup file not found: $BACKUP_FILE"
    exit 1
fi

echo "Stopping rfid-server service..."
systemctl stop rfid-server 2>/dev/null || true

echo "Restoring database from $BACKUP_FILE..."

if [[ "$BACKUP_FILE" == *.gz ]]; then
    gunzip -c "$BACKUP_FILE" | psql "$DB_NAME"
else
    psql "$DB_NAME" < "$BACKUP_FILE"
fi

if [ $? -eq 0 ]; then
    echo "Restore completed successfully"
    
    echo "Starting rfid-server service..."
    systemctl start rfid-server 2>/dev/null || true
    
    echo "Database restored and service restarted"
else
    echo "Restore failed"
    exit 1
fi