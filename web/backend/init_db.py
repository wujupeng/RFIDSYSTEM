import asyncio
import sys
import os

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from sqlalchemy import text
from core.database import engine, Base
from models.asset_ledger import AssetLedger, AssetCategoryDict, DepartmentDict, RegionDict, LedgerSnapshot
from models.import_batch import ImportBatch, ImportFailureDetail
from models.erp_sync import ErpConnectionConfig, ErpSyncTask, ErpConflictTicket
from models.diff_analysis import DiffReport, DiffRecord
from models.diff_work_order import DiffWorkOrder, AccountWritebackEvent, AuditLog
from models.spatial_truth import (
    GroundTruthPointModel, CalibrationSessionModel, ComparisonRecordModel,
    AccuracyReportModel, CoordinateSystemModel, BenchmarkReportModel,
    SpatialGraphSnapshotModel, FactoryMapSnapshotModel, CalibrationAuditLogModel,
)


async def init_db():
    async with engine.begin() as conn:
        await conn.run_sync(Base.metadata.create_all)
        print("All tables created successfully")

        try:
            await conn.execute(text("SELECT 1 FROM asset_category_dict LIMIT 1"))
        except Exception:
            cats = [
                ("IT", "IT设备", 1), ("OFFICE", "办公设备", 2), ("TOOL", "工具", 3),
                ("VEHICLE", "车辆", 4), ("FURNITURE", "家具", 5),
            ]
            for code, name, order in cats:
                await conn.execute(text(
                    f"INSERT INTO asset_category_dict (code, name, sort_order, enabled) VALUES ('{code}', '{name}', {order}, true)"
                ))

            depts = [
                ("IT", "IT部", 1), ("PROD", "生产部", 2), ("LOGI", "物流部", 3),
                ("FIN", "财务部", 4), ("HR", "人力资源部", 5),
            ]
            for code, name, order in depts:
                await conn.execute(text(
                    f"INSERT INTO department_dict (code, name, sort_order, enabled) VALUES ('{code}', '{name}', {order}, true)"
                ))

            regions = [
                ("W1", "1号仓库", 1), ("W2", "2号仓库", 2), ("W3", "3号仓库", 3),
                ("WS", "车间", 4), ("OFFICE", "办公区", 5),
            ]
            for code, name, order in regions:
                await conn.execute(text(
                    f"INSERT INTO region_dict (code, name, sort_order, enabled) VALUES ('{code}', '{name}', {order}, true)"
                ))
            print("Dictionary data inserted")

    await engine.dispose()
    print("Done!")


if __name__ == "__main__":
    asyncio.run(init_db())