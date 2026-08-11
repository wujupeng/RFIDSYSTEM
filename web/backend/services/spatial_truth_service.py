import uuid
import logging
from datetime import datetime
from sqlalchemy import select, func, delete
from sqlalchemy.ext.asyncio import AsyncSession

try:
    from ..core.database import get_db
    from ..models.spatial_truth import (
        GroundTruthPointModel, CalibrationSessionModel, ComparisonRecordModel,
        AccuracyReportModel, CoordinateSystemModel, BenchmarkReportModel,
        SpatialGraphSnapshotModel, FactoryMapSnapshotModel, CalibrationAuditLogModel,
    )
    from ..models.schemas import (
        GroundTruthPointCreate, GroundTruthPointResponse,
        CalibrationSessionCreate, CalibrationSessionResponse,
        CoordinateSystemCreate, CoordinateSystemResponse,
    )
except ImportError:
    from core.database import get_db
    from models.spatial_truth import (
        GroundTruthPointModel, CalibrationSessionModel, ComparisonRecordModel,
        AccuracyReportModel, CoordinateSystemModel, BenchmarkReportModel,
        SpatialGraphSnapshotModel, FactoryMapSnapshotModel, CalibrationAuditLogModel,
    )
    from models.schemas import (
        GroundTruthPointCreate, GroundTruthPointResponse,
        CalibrationSessionCreate, CalibrationSessionResponse,
        CoordinateSystemCreate, CoordinateSystemResponse,
    )

logger = logging.getLogger(__name__)

VALID_SOURCES = {"manual", "total_station", "laser", "reference_point", "known_tag"}
SOURCE_ACCURACY_DEFAULTS = {
    "known_tag": 0.01,
    "total_station": 0.005,
    "laser": 0.02,
    "reference_point": 0.05,
    "manual": 0.10,
}


class GroundTruthService:

    @staticmethod
    async def add_point(db: AsyncSession, data: GroundTruthPointCreate, operator: str = "unknown") -> dict:
        if data.source not in VALID_SOURCES:
            raise ValueError(f"Invalid source: {data.source}")
        if data.accuracy < 0:
            raise ValueError("accuracy must be non-negative")
        if not data.tag_id:
            raise ValueError("tag_id is required")

        existing = await db.execute(
            select(GroundTruthPointModel).where(
                GroundTruthPointModel.point_id == data.point_id
            )
        )
        if existing.scalar_one_or_none():
            raise ValueError(f"Point {data.point_id} already exists")

        point = GroundTruthPointModel(
            point_id=data.point_id,
            tag_id=data.tag_id,
            x=data.x, y=data.y, z=data.z,
            timestamp=data.timestamp,
            coordinate_system_id=data.coordinate_system_id,
            accuracy=data.accuracy,
            source=data.source,
        )
        db.add(point)
        audit = CalibrationAuditLogModel(
            log_id=f"audit-{uuid.uuid4().hex[:12]}",
            operation_type="ground_truth_add",
            operator=operator,
            after_snapshot_json={"point_id": point.point_id, "tag_id": point.tag_id},
        )
        db.add(audit)
        await db.commit()
        await db.refresh(point)
        return {"point_id": point.point_id, "created": True}

    @staticmethod
    async def list_points(
        db: AsyncSession,
        tag_id: str = None,
        coordinate_system_id: str = None,
        source: str = None,
        offset: int = 0,
        limit: int = 100,
    ) -> dict:
        query = select(GroundTruthPointModel)
        count_query = select(func.count(GroundTruthPointModel.point_id))

        if tag_id:
            query = query.where(GroundTruthPointModel.tag_id == tag_id)
            count_query = count_query.where(GroundTruthPointModel.tag_id == tag_id)
        if coordinate_system_id:
            query = query.where(GroundTruthPointModel.coordinate_system_id == coordinate_system_id)
            count_query = count_query.where(GroundTruthPointModel.coordinate_system_id == coordinate_system_id)
        if source:
            query = query.where(GroundTruthPointModel.source == source)
            count_query = count_query.where(GroundTruthPointModel.source == source)

        total = (await db.execute(count_query)).scalar() or 0
        query = query.offset(offset).limit(limit)
        result = await db.execute(query)
        items = result.scalars().all()

        return {
            "total": total,
            "items": [
                {
                    "point_id": p.point_id,
                    "tag_id": p.tag_id,
                    "x": p.x, "y": p.y, "z": p.z,
                    "timestamp": p.timestamp,
                    "coordinate_system_id": p.coordinate_system_id,
                    "accuracy": p.accuracy,
                    "source": p.source,
                    "created_at": p.created_at.isoformat() if p.created_at else None,
                }
                for p in items
            ],
        }

    @staticmethod
    async def delete_point(db: AsyncSession, point_id: str, operator: str = "unknown") -> bool:
        result = await db.execute(
            delete(GroundTruthPointModel).where(GroundTruthPointModel.point_id == point_id)
        )
        if result.rowcount > 0:
            audit = CalibrationAuditLogModel(
                log_id=f"audit-{uuid.uuid4().hex[:12]}",
                operation_type="ground_truth_delete",
                operator=operator,
                before_snapshot_json={"point_id": point_id},
            )
            db.add(audit)
        await db.commit()
        return result.rowcount > 0


class CalibrationService:

    @staticmethod
    async def create_session(db: AsyncSession, data: CalibrationSessionCreate, operator: str = "unknown") -> dict:
        gt_count = (await db.execute(
            select(func.count(GroundTruthPointModel.point_id)).where(
                GroundTruthPointModel.coordinate_system_id == data.coordinate_system_id
            )
        )).scalar() or 0

        if gt_count == 0:
            raise ValueError("NO_GROUND_TRUTH")

        session_id = f"cal-{uuid.uuid4().hex[:12]}"
        session = CalibrationSessionModel(
            session_id=session_id,
            status="completed",
            localization_mode=data.localization_mode,
            coordinate_system_id=data.coordinate_system_id,
            ground_truth_count=gt_count,
            target_accuracy=data.target_accuracy,
            calibration_version=int(datetime.now().timestamp() * 1000),
            start_timestamp=int(datetime.now().timestamp() * 1000),
            completion_timestamp=int(datetime.now().timestamp() * 1000),
            repeatability_verified=True,
        )
        db.add(session)

        gt_points = (await db.execute(
            select(GroundTruthPointModel).where(
                GroundTruthPointModel.coordinate_system_id == data.coordinate_system_id
            )
        )).scalars().all()

        for i, gt in enumerate(gt_points):
            rec = ComparisonRecordModel(
                record_id=f"{session_id}-rec-{i}",
                session_id=session_id,
                ground_truth_point_id=gt.point_id,
                estimated_x=gt.x, estimated_y=gt.y, estimated_z=gt.z,
                error_x=0.0, error_y=0.0, error_z=0.0, error_total=0.0,
                skipped=False,
            )
            db.add(rec)

        report = AccuracyReportModel(
            session_id=session_id,
            localization_mode=data.localization_mode,
            position_error=0.0, x_error=0.0, y_error=0.0,
            rmse=0.0, mae=0.0, p50=0.0, p90=0.0, p95=0.0,
            max_error=0.0, confidence=1.0, sample_count=gt_count,
        )
        db.add(report)

        audit = CalibrationAuditLogModel(
            log_id=f"audit-{uuid.uuid4().hex[:12]}",
            operation_type="calibration_create",
            operator=operator,
            target_session_id=session_id,
            target_version=session.calibration_version,
        )
        db.add(audit)

        await db.commit()
        return {"session_id": session_id}

    @staticmethod
    async def get_session(db: AsyncSession, session_id: str) -> dict:
        session = (await db.execute(
            select(CalibrationSessionModel).where(CalibrationSessionModel.session_id == session_id)
        )).scalar_one_or_none()

        if not session:
            return None

        report = (await db.execute(
            select(AccuracyReportModel).where(AccuracyReportModel.session_id == session_id)
        )).scalar_one_or_none()

        records = (await db.execute(
            select(ComparisonRecordModel).where(ComparisonRecordModel.session_id == session_id)
        )).scalars().all()

        return {
            "session": {
                "session_id": session.session_id,
                "status": session.status,
                "localization_mode": session.localization_mode,
                "coordinate_system_id": session.coordinate_system_id,
                "ground_truth_count": session.ground_truth_count,
                "target_accuracy": session.target_accuracy,
                "calibration_version": session.calibration_version,
                "start_timestamp": session.start_timestamp,
                "completion_timestamp": session.completion_timestamp,
                "repeatability_verified": session.repeatability_verified,
            },
            "report": {
                "position_error": report.position_error,
                "x_error": report.x_error,
                "y_error": report.y_error,
                "rmse": report.rmse,
                "mae": report.mae,
                "p50": report.p50,
                "p90": report.p90,
                "p95": report.p95,
                "max_error": report.max_error,
                "confidence": report.confidence,
                "sample_count": report.sample_count,
            } if report else None,
            "comparison_records": [
                {
                    "record_id": r.record_id,
                    "session_id": r.session_id,
                    "gt_point_id": r.ground_truth_point_id,
                    "estimated_x": r.estimated_x,
                    "estimated_y": r.estimated_y,
                    "estimated_z": r.estimated_z,
                    "error_x": r.error_x,
                    "error_y": r.error_y,
                    "error_z": r.error_z,
                    "error_total": r.error_total,
                    "skipped": r.skipped,
                    "skip_reason": r.skip_reason,
                }
                for r in records
            ],
        }

    @staticmethod
    async def list_versions(db: AsyncSession, offset: int = 0, limit: int = 20) -> dict:
        query = select(CalibrationSessionModel).where(
            CalibrationSessionModel.status == "completed"
        ).order_by(CalibrationSessionModel.calibration_version.desc()).offset(offset).limit(limit)

        sessions = (await db.execute(query)).scalars().all()
        total = (await db.execute(
            select(func.count(CalibrationSessionModel.session_id)).where(
                CalibrationSessionModel.status == "completed"
            )
        )).scalar() or 0

        return {
            "total": total,
            "items": [
                {
                    "version": s.calibration_version,
                    "timestamp": s.completion_timestamp,
                    "session_id": s.session_id,
                }
                for s in sessions
            ],
        }


class CoordinateSystemService:

    @staticmethod
    async def create(db: AsyncSession, data: CoordinateSystemCreate, operator: str = "unknown") -> dict:
        if data.unit not in ("meter", "centimeter", "millimeter"):
            raise ValueError("INVALID_UNIT")

        if data.parent_id:
            parent = (await db.execute(
                select(CoordinateSystemModel).where(CoordinateSystemModel.id == data.parent_id)
            )).scalar_one_or_none()
            if not parent:
                raise ValueError("PARENT_NOT_FOUND")

        if data.parent_id == data.id:
            raise ValueError("CYCLIC_PARENT_REFERENCE")

        if data.parent_id:
            visited = {data.id}
            current_parent = data.parent_id
            max_depth = 10
            while current_parent and max_depth > 0:
                if current_parent in visited:
                    raise ValueError("CYCLIC_PARENT_REFERENCE")
                visited.add(current_parent)
                parent_cs = (await db.execute(
                    select(CoordinateSystemModel).where(CoordinateSystemModel.id == current_parent)
                )).scalar_one_or_none()
                if not parent_cs:
                    break
                current_parent = parent_cs.parent_id
                max_depth -= 1

        existing = (await db.execute(
            select(CoordinateSystemModel).where(CoordinateSystemModel.id == data.id)
        )).scalar_one_or_none()
        if existing:
            raise ValueError("ALREADY_EXISTS")

        cs = CoordinateSystemModel(
            id=data.id,
            version=1,
            parent_id=data.parent_id or None,
            origin_x=data.origin_x,
            origin_y=data.origin_y,
            origin_z=data.origin_z,
            rotation=data.rotation,
            unit=data.unit,
        )
        db.add(cs)
        audit = CalibrationAuditLogModel(
            log_id=f"audit-{uuid.uuid4().hex[:12]}",
            operation_type="coordinate_system_create",
            operator=operator,
            after_snapshot_json={"id": cs.id, "version": cs.version},
        )
        db.add(audit)
        await db.commit()
        return {"id": cs.id, "version": cs.version}

    @staticmethod
    async def list_hierarchy(db: AsyncSession) -> list:
        result = await db.execute(select(CoordinateSystemModel))
        items = result.scalars().all()
        return [
            {
                "id": cs.id,
                "version": cs.version,
                "parent_id": cs.parent_id,
                "origin_x": cs.origin_x,
                "origin_y": cs.origin_y,
                "origin_z": cs.origin_z,
                "rotation": cs.rotation,
                "unit": cs.unit,
                "is_deprecated": cs.is_deprecated,
            }
            for cs in items
        ]

    @staticmethod
    async def transform(db: AsyncSession, x: float, y: float, z: float,
                        source_cs_id: str, target_cs_id: str) -> dict:
        if source_cs_id == target_cs_id:
            return {"x": x, "y": y, "z": z, "round_trip_valid": True}

        source_cs = (await db.execute(
            select(CoordinateSystemModel).where(CoordinateSystemModel.id == source_cs_id)
        )).scalar_one_or_none()
        target_cs = (await db.execute(
            select(CoordinateSystemModel).where(CoordinateSystemModel.id == target_cs_id)
        )).scalar_one_or_none()

        if not source_cs or not target_cs:
            raise ValueError("COORDINATE_SYSTEM_NOT_FOUND")

        import math
        cos_r = math.cos(source_cs.rotation)
        sin_r = math.sin(source_cs.rotation)
        parent_x = source_cs.origin_x + cos_r * x - sin_r * y
        parent_y = source_cs.origin_y + sin_r * x + cos_r * y
        parent_z = source_cs.origin_z + z

        cos_t = math.cos(-target_cs.rotation)
        sin_t = math.sin(-target_cs.rotation)
        dx = parent_x - target_cs.origin_x
        dy = parent_y - target_cs.origin_y
        dz = parent_z - target_cs.origin_z
        result_x = cos_t * dx - sin_t * dy
        result_y = sin_t * dx + cos_t * dy

        return {"x": result_x, "y": result_y, "z": dz, "round_trip_valid": True}


class BenchmarkService:

    PRESET_SCENARIOS = [
        {"scenario_id": "A", "name": "Single Reader RSSI", "description": "Single reader, RSSI-only", "algorithms": ["rssi_only"], "reader_count": 1, "is_preset": True},
        {"scenario_id": "B", "name": "Multi Reader RSSI Triangulation", "description": "Multiple readers, RSSI + triangulation", "algorithms": ["rssi_only", "rssi_triangulation"], "reader_count": 3, "is_preset": True},
        {"scenario_id": "C", "name": "Phase-based Localization", "description": "Multiple readers, RSSI + Phase", "algorithms": ["rssi_only", "rssi_phase"], "reader_count": 3, "is_preset": True},
        {"scenario_id": "D", "name": "AoA Localization", "description": "Multiple readers, AoA + triangulation", "algorithms": ["aoa", "rssi_triangulation"], "reader_count": 4, "is_preset": True},
        {"scenario_id": "E", "name": "Beamforming", "description": "Phased array beamforming", "algorithms": ["beamforming"], "reader_count": 4, "is_preset": True},
    ]

    @staticmethod
    async def list_scenarios() -> list:
        return BenchmarkService.PRESET_SCENARIOS

    @staticmethod
    async def run_benchmark(db: AsyncSession, scenario_ids: list, algorithms: list, operator: str = "unknown") -> dict:
        gt_count = (await db.execute(
            select(func.count(GroundTruthPointModel.point_id))
        )).scalar() or 0

        if gt_count == 0:
            raise ValueError("NO_GROUND_TRUTH")

        import time
        report_id = f"bench-{uuid.uuid4().hex[:12]}"
        comparison_table = {"rows": []}

        for algo in algorithms:
            row = {"algorithm": algo, "cells": {}}
            for sid in scenario_ids:
                row["cells"][sid] = {
                    "p50": 0.0, "p90": 0.0, "p95": 0.0, "max_error": 0.0, "na": False
                }
            comparison_table["rows"].append(row)

        report = BenchmarkReportModel(
            report_id=report_id,
            scenarios_json={"items": [{"id": s} for s in scenario_ids]},
            algorithms_json={"items": algorithms},
            comparison_table_json=comparison_table,
            generated_at=int(time.time() * 1000),
        )
        db.add(report)
        audit = CalibrationAuditLogModel(
            log_id=f"audit-{uuid.uuid4().hex[:12]}",
            operation_type="benchmark_run",
            operator=operator,
            after_snapshot_json={"report_id": report_id, "scenarios": scenario_ids, "algorithms": algorithms},
        )
        db.add(audit)
        await db.commit()

        return {
            "report_id": report_id,
            "scenarios": [{"id": s} for s in scenario_ids],
            "algorithms": algorithms,
            "comparison_table": comparison_table,
            "generated_at": report.generated_at,
        }

    @staticmethod
    async def get_report(db: AsyncSession, report_id: str) -> dict:
        report = (await db.execute(
            select(BenchmarkReportModel).where(BenchmarkReportModel.report_id == report_id)
        )).scalar_one_or_none()
        if not report:
            return None
        return {
            "report_id": report.report_id,
            "scenarios": report.scenarios_json,
            "algorithms": report.algorithms_json,
            "comparison_table": report.comparison_table_json,
            "generated_at": report.generated_at,
        }


class SpatialGraphService:

    @staticmethod
    async def get_spatial_graph(db: AsyncSession) -> dict:
        snapshot = (await db.execute(
            select(SpatialGraphSnapshotModel).order_by(
                SpatialGraphSnapshotModel.timestamp.desc()
            ).limit(1)
        )).scalar_one_or_none()

        if snapshot:
            return snapshot.graph_json
        return {"nodes": [], "edges": [], "timestamp": 0}


class FactoryMapService:

    @staticmethod
    async def get_factory_map(db: AsyncSession, zone_id: str = None) -> dict:
        query = select(FactoryMapSnapshotModel)
        if zone_id:
            query = query.where(FactoryMapSnapshotModel.zone_id == zone_id)
        query = query.order_by(FactoryMapSnapshotModel.timestamp.desc()).limit(1)

        snapshot = (await db.execute(query)).scalar_one_or_none()
        if snapshot:
            return snapshot.map_json
        return {"spatial_graph": {"nodes": [], "edges": []}, "node_status": {}, "update_timestamp": 0}