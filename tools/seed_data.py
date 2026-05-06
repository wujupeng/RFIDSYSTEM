#!/usr/bin/env python3
import psycopg2
import random
import string

ASSET_TYPES = [
    "Laptop", "Desktop", "Monitor", "Printer", "Server",
    "Tablet", "Phone", "Router", "Switch", "Projector",
    "Camera", "Scanner", "UPS", "HardDrive", "SSD"
]

ASSET_BRANDS = [
    "Dell", "HP", "Lenovo", "Apple", "Samsung",
    "ASUS", "Acer", "Sony", "Canon", "Epson"
]

LOCATIONS = [
    "研发部-A区", "研发部-B区", "办公区-1楼", "办公区-2楼",
    "会议室-A", "会议室-B", "机房-主网络柜", "仓库-1号",
    "培训室", "接待区", "管理层办公室", "财务室"
]

STATUSES = ["IN_STOCK", "IN_USE", "REPAIR", "SCRAPPED"]

def generate_epc(index):
    return f"EPC-{index:06d}"

def generate_asset_code(index):
    return f"AST-{index:06d}"

def generate_name(index):
    brand = random.choice(ASSET_BRANDS)
    asset_type = random.choice(ASSET_TYPES)
    return f"{brand}-{asset_type}-{index}"

def get_random_location():
    return random.choice(LOCATIONS)

def get_random_type():
    return random.choice(ASSET_TYPES)

def get_random_status():
    weights = [0.6, 0.25, 0.1, 0.05]  # IN_STOCK, IN_USE, REPAIR, SCRAPPED
    return random.choices(STATUSES, weights=weights)[0]

def main():
    import sys
    count = 50000
    if len(sys.argv) > 1:
        count = int(sys.argv[1])
    
    print("========================================")
    print("     RFID SYSTEM DATA SEEDER")
    print("========================================")
    print(f"Target records to insert: {count}")
    print("========================================")
    
    try:
        conn = psycopg2.connect(
            dbname="rfid",
            user="postgres",
            password="123456",
            host="/var/run/postgresql"
        )
        
        cur = conn.cursor()
        
        print("Clearing existing data...")
        cur.execute("DELETE FROM assets")
        conn.commit()
        print("Existing data cleared.")
        
        batch_size = 1000
        insert_count = 0
        
        for i in range(1, count + 1):
            name = generate_name(i)
            asset_type = get_random_type()
            asset_code = generate_asset_code(i)
            epc = generate_epc(i)
            location = get_random_location()
            status = get_random_status()
            
            cur.execute(
                """INSERT INTO assets(name, type, asset_code, rfid_epc, location, status)
                   VALUES(%s, %s, %s, %s, %s, %s)""",
                (name, asset_type, asset_code, epc, location, status)
            )
            
            insert_count += 1
            
            if i % batch_size == 0:
                conn.commit()
                print(f"Inserted {i} records...")
        
        conn.commit()
        print("========================================")
        print(f"Successfully inserted {count} records!")
        print("========================================")
        
        cur.close()
        conn.close()
        
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()
        return 1
    
    return 0

if __name__ == "__main__":
    main()
