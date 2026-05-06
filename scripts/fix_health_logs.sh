#!/bin/bash
PGPASSWORD=123456 psql -U postgres -d rfid <<EOF
ALTER TABLE health_logs ADD COLUMN component VARCHAR(50);
EOF
