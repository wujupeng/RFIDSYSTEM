from pydantic import BaseModel, Field
from typing import Optional
from datetime import datetime


class DeviceInfo(BaseModel):
    device_id: str
    device_name: str
    ip_address: str
    firmware_version: str = ""
    status: str = "offline"


class BeamConfig(BaseModel):
    azimuth_deg: float
    elevation_deg: float
    power_dbm: float = 30.0
    weight_mode: str = "uniform"


class BeamResult(BaseModel):
    beam_id: int
    azimuth_deg: float
    elevation_deg: float
    power_dbm: float
    confidence: float


class InventoryTaskRequest(BaseModel):
    region_id: str
    initiator: str = "OPERATOR"
    ai_route_planning: bool = True


class InventoryReport(BaseModel):
    task_id: str
    status: str
    total_tags: int = 0
    read_tags: int = 0
    duration_ms: float = 0.0
    found_epcs: list[str] = []
    missing_epcs: list[str] = []
    extra_epcs: list[str] = []


class TrajectoryPoint(BaseModel):
    x: float
    y: float
    z: float
    timestamp: int


class TrajectoryResponse(BaseModel):
    epc: str
    points: list[TrajectoryPoint] = []


class SystemConfig(BaseModel):
    system_name: str = "PA-RFID"
    freq_min_mhz: float = 920.0

    freq_max_mhz: float = 925.0
    default_power_dbm: float = 30.0
    inventory_timeout_s: int = 300
    heartbeat_interval_s: int = 5


class TokenResponse(BaseModel):
    access_token: str
    token_type: str = "bearer"


class LoginRequest(BaseModel):
    username: str
    password: str


class GroundTruthPointCreate(BaseModel):
    point_id: str = Field(..., max_length=64)
    tag_id: str = Field(..., max_length=128)
    x: float = Field(..., ge=-100000, le=100000)
    y: float = Field(..., ge=-100000, le=100000)
    z: float = Field(0.0, ge=-100000, le=100000)
    timestamp: int = Field(..., gt=0)
    coordinate_system_id: str = Field("global", max_length=64)
    accuracy: float = Field(..., ge=0, le=100)
    source: str = Field(..., max_length=32)


class GroundTruthPointResponse(BaseModel):
    point_id: str
    tag_id: str
    x: float
    y: float
    z: float
    timestamp: int
    coordinate_system_id: str
    accuracy: float
    source: str
    created_at: Optional[datetime] = None


class GroundTruthPointListResponse(BaseModel):
    total: int
    items: list[GroundTruthPointResponse] = []


class CalibrationSessionCreate(BaseModel):
    localization_mode: str = "rssi_triangulation"
    coordinate_system_id: str = "global"
    target_accuracy: float = 0.1
    source_filter: Optional[str] = None


class AccuracyReportResponse(BaseModel):
    position_error: float
    x_error: float
    y_error: float
    rmse: float
    mae: float
    p50: float
    p90: float
    p95: float
    max_error: float
    confidence: float
    sample_count: int


class ComparisonRecordResponse(BaseModel):
    record_id: str
    session_id: str
    gt_x: float
    gt_y: float
    gt_z: float
    estimated_x: Optional[float] = None
    estimated_y: Optional[float] = None
    estimated_z: Optional[float] = None
    error_x: Optional[float] = None
    error_y: Optional[float] = None
    error_z: Optional[float] = None
    error_total: Optional[float] = None
    skipped: bool = False
    skip_reason: Optional[str] = None


class CalibrationSessionResponse(BaseModel):
    session_id: str
    status: str
    localization_mode: str
    coordinate_system_id: str
    ground_truth_count: int
    target_accuracy: float
    calibration_version: Optional[int] = None
    start_timestamp: Optional[int] = None
    completion_timestamp: Optional[int] = None
    repeatability_verified: bool = False


class CalibrationSessionDetailResponse(BaseModel):
    session: CalibrationSessionResponse
    report: Optional[AccuracyReportResponse] = None
    comparison_records: list[ComparisonRecordResponse] = []


class CalibrationVersionListResponse(BaseModel):
    total: int
    items: list[dict] = []


class CoordinateSystemCreate(BaseModel):
    id: str = Field(..., max_length=64)
    parent_id: str = Field("", max_length=64)
    origin_x: float = Field(0.0, ge=-100000, le=100000)
    origin_y: float = Field(0.0, ge=-100000, le=100000)
    origin_z: float = Field(0.0, ge=-100000, le=100000)
    rotation: float = Field(0.0, ge=-6.28319, le=6.28319)
    unit: str = Field("meter", max_length=16)


class CoordinateSystemResponse(BaseModel):
    id: str
    version: int
    parent_id: Optional[str] = None
    origin_x: float
    origin_y: float
    origin_z: float
    rotation: float
    unit: str
    is_deprecated: bool = False


class TransformRequest(BaseModel):
    x: float
    y: float
    z: float = 0.0
    source_cs_id: str
    target_cs_id: str


class TransformResponse(BaseModel):
    x: float
    y: float
    z: float
    round_trip_valid: Optional[bool] = None


class BenchmarkRunRequest(BaseModel):
    scenario_ids: list[str] = Field(..., max_length=10)
    algorithms: list[str] = Field(..., max_length=10)


class BenchmarkCellResponse(BaseModel):
    p50: float = 0.0
    p90: float = 0.0
    p95: float = 0.0
    max_error: float = 0.0
    na: bool = False
    na_reason: Optional[str] = None


class BenchmarkReportResponse(BaseModel):
    report_id: str
    scenarios: list[dict] = []
    algorithms: list[str] = []
    comparison_table: dict = {}
    generated_at: int


class ScenarioListResponse(BaseModel):
    scenarios: list[dict]


class SpatialGraphResponse(BaseModel):
    nodes: list[dict] = []
    edges: list[dict] = []
    timestamp: int = 0


class FactoryMapResponse(BaseModel):
    spatial_graph: Optional[SpatialGraphResponse] = None
    node_status: dict = {}
    update_timestamp: int = 0