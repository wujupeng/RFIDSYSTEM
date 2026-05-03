#!/bin/bash

# RFID Asset Management System - Backup Script
# Usage: ./backup.sh [daily|weekly]
# 
# Retention Policy:
# - Daily backups: keep 7 days
# - Weekly backups: keep 4 weeks

set -e

BACKUP_DIR="/backup/rfid"
DB_NAME="rfid"
DAYS_TO_KEEP_DAILY=7
WEEKS_TO_KEEP_WEEKLY=4

mkdir -p "$BACKUP_DIR/daily"
mkdir -p "$BACKUP_DIR/weekly"

case "$1" in
    daily)
        BACKUP_FILE="$BACKUP_DIR/daily/rfid_$(date +%Y%m%d_%H%M%S).sql"
        ;;
    weekly)
        BACKUP_FILE="$BACKUP_DIR/weekly/rfid_weekly_$(date +%Y%m%d).sql"
        ;;
    *)
        echo "Usage: $0 [daily|weekly]"
        exit 1
        ;;
esac

echo "Starting backup to $BACKUP_FILE..."

pg_dump "$DB_NAME" > "$BACKUP_FILE"

if [ $? -eq 0 ]; then
    echo "Backup completed successfully"
    
    # Compress backup
    gzip "$BACKUP_FILE"
    echo "Backup compressed"
    
    # Cleanup old backups
    if [ "$1" = "daily" ]; then
        find "$BACKUP_DIR/daily" -name "*.sql.gz" -mtime +$DAYS_TO_KEEP_DAILY -delete
        echo "Cleaned up daily backups older than $DAYS_TO_KEEP_DAILY days"
    else
        find "$BACKUP_DIR/weekly" -name "*.sql.gz" -mtime +$((WEEKS_TO_KEEP_WEEKLY * 7)) -delete
        echo "Cleaned up weekly backups older than $WEEKS_TO_KEEP_WEEKLY weeks"
    fi
else
    echo "Backup failed"
    exit 1
fi