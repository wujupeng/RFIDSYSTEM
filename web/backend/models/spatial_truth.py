from sqlalchemy import String, Float, BigInteger, Integer, Boolean, Text, Index
from sqlalchemy.dialects.postgresql import JSONB
from sqlalchemy.orm import Mapped, mapped_column
try:
    from .base import TimestampMixin, Base
except ImportError:
    from base import TimestampMixin, Base


class GroundTruthPointModel(TimestampMixin, Base):
    __tablename__ = "ground_truth_points"
    __table_args__ = (
        Index("ix_gt_tag_cs_ts", "tag_id", "coordinate_system_id", "timestamp"),
        Index("ix_gt_source", "source"),
    )

    point_id: Mapped[str] = mapped_column(String(64), primary_key=True)
    tag_id: Mapped[str] = mapped_column(String(128), nullable=False, index=True)
    x: Mapped[float] = mapped_column(Float, nullable=False)
    y: Mapped[float] = mapped_column(Float, nullable=False)
    z: Mapped[float] = mapped_column(Float, nullable=False, default=0.0)
    timestamp: Mapped[int] = mapped_column(BigInteger, nullable=False)
    coordinate_system_id: Mapped[str] = mapped_column(String(64), nullable=False, index=True, default="global")
    accuracy: Mapped[float] = mapped_column(Float, nullable=False)
    source: Mapped[str] = mapped_column(String(32), nullable=False)


class CalibrationSessionModel(TimestampMixin, Base):
    __tablename__ = "calibration_sessions"
    __table_args__ = (
        Index("ix_cal_session_status", "status"),
    )

    session_id: Mapped[str] = mapped_column(String(64), primary_key=True)
    status: Mapped[str] = mapped_column(String(32), nullable=False, default="created")
    localization_mode: Mapped[str] = mapped_column(String(32), nullable=False)
    coordinate_system_id: Mapped[str] = mapped_column(String(64), nullable=False, default="global")
    ground_truth_count: Mapped[int] = mapped_column(Integer, nullable=False, default=0)
    target_accuracy: Mapped[float] = mapped_column(Float, nullable=False, default=0.1)
    calibration_version: Mapped[int] = mapped_column(BigInteger, nullable=True, unique=True)
    start_timestamp: Mapped[int] = mapped_column(BigInteger, nullable=True)
    completion_timestamp: Mapped[int] = mapped_column(BigInteger, nullable=True)
    repeatability_verified: Mapped[bool] = mapped_column(Boolean, nullable=False, default=False)


class ComparisonRecordModel(TimestampMixin, Base):
    __tablename__ = "comparison_records"

    record_id: Mapped[str] = mapped_column(String(64), primary_key=True)
    session_id: Mapped[str] = mapped_column(String(64), nullable=False, index=True)
    ground_truth_point_id: Mapped[str] = mapped_column(String(64), nullable=False, index=True)
    estimated_x: Mapped[float] = mapped_column(Float, nullable=True)
    estimated_y: Mapped[float] = mapped_column(Float, nullable=True)
    estimated_z: Mapped[float] = mapped_column(Float, nullable=True)
    error_x: Mapped[float] = mapped_column(Float, nullable=True)
    error_y: Mapped[float] = mapped_column(Float, nullable=True)
    error_z: Mapped[float] = mapped_column(Float, nullable=True)
    error_total: Mapped[float] = mapped_column(Float, nullable=True)
    skipped: Mapped[bool] = mapped_column(Boolean, nullable=False, default=False)
    skip_reason: Mapped[str] = mapped_column(Text, nullable=True)


class AccuracyReportModel(TimestampMixin, Base):
    __tablename__ = "accuracy_reports"

    session_id: Mapped[str] = mapped_column(String(64), primary_key=True)
    localization_mode: Mapped[str] = mapped_column(String(32), nullable=False)
    position_error: Mapped[float] = mapped_column(Float, nullable=False)
    x_error: Mapped[float] = mapped_column(Float, nullable=False)
    y_error: Mapped[float] = mapped_column(Float, nullable=False)
    rmse: Mapped[float] = mapped_column(Float, nullable=False)
    mae: Mapped[float] = mapped_column(Float, nullable=False)
    p50: Mapped[float] = mapped_column(Float, nullable=False)
    p90: Mapped[float] = mapped_column(Float, nullable=False)
    p95: Mapped[float] = mapped_column(Float, nullable=False)
    max_error: Mapped[float] = mapped_column(Float, nullable=False)
    confidence: Mapped[float] = mapped_column(Float, nullable=False)
    sample_count: Mapped[int] = mapped_column(Integer, nullable=False)


class CoordinateSystemModel(TimestampMixin, Base):
    __tablename__ = "coordinate_systems"
    __table_args__ = (
        Index("ix_cs_parent", "parent_id"),
    )

    id: Mapped[str] = mapped_column(String(64), primary_key=True)
    version: Mapped[int] = mapped_column(BigInteger, primary_key=True, default=1)
    parent_id: Mapped[str] = mapped_column(String(64), nullable=True)
    origin_x: Mapped[float] = mapped_column(Float, nullable=False, default=0.0)
    origin_y: Mapped[float] = mapped_column(Float, nullable=False, default=0.0)
    origin_z: Mapped[float] = mapped_column(Float, nullable=False, default=0.0)
    rotation: Mapped[float] = mapped_column(Float, nullable=False, default=0.0)
    unit: Mapped[str] = mapped_column(String(16), nullable=False, default="meter")
    is_deprecated: Mapped[bool] = mapped_column(Boolean, nullable=False, default=False)


class BenchmarkReportModel(TimestampMixin, Base):
    __tablename__ = "benchmark_reports"

    report_id: Mapped[str] = mapped_column(String(64), primary_key=True)
    scenarios_json: Mapped[dict] = mapped_column(JSONB, nullable=False)
    algorithms_json: Mapped[dict] = mapped_column(JSONB, nullable=False)
    comparison_table_json: Mapped[dict] = mapped_column(JSONB, nullable=False)
    generated_at: Mapped[int] = mapped_column(BigInteger, nullable=False)


class SpatialGraphSnapshotModel(TimestampMixin, Base):
    __tablename__ = "spatial_graph_snapshots"

    snapshot_id: Mapped[str] = mapped_column(String(64), primary_key=True)
    graph_json: Mapped[dict] = mapped_column(JSONB, nullable=False)
    timestamp: Mapped[int] = mapped_column(BigInteger, nullable=False)


class FactoryMapSnapshotModel(TimestampMixin, Base):
    __tablename__ = "factory_map_snapshots"

    snapshot_id: Mapped[str] = mapped_column(String(64), primary_key=True)
    map_json: Mapped[dict] = mapped_column(JSONB, nullable=False)
    zone_id: Mapped[str] = mapped_column(String(64), nullable=True, index=True)
    timestamp: Mapped[int] = mapped_column(BigInteger, nullable=False)


class CalibrationAuditLogModel(TimestampMixin, Base):
    __tablename__ = "calibration_audit_logs"
    __table_args__ = (
        Index("ix_audit_session", "target_session_id"),
        Index("ix_audit_operator", "operator"),
    )

    log_id: Mapped[str] = mapped_column(String(64), primary_key=True)
    operation_type: Mapped[str] = mapped_column(String(32), nullable=False)
    operator: Mapped[str] = mapped_column(String(64), nullable=False)
    target_session_id: Mapped[str] = mapped_column(String(64), nullable=True)
    target_version: Mapped[int] = mapped_column(BigInteger, nullable=True)
    before_snapshot_json: Mapped[dict] = mapped_column(JSONB, nullable=True)
    after_snapshot_json: Mapped[dict] = mapped_column(JSONB, nullable=True)
    trace_id: Mapped[str] = mapped_column(String(64), nullable=True)