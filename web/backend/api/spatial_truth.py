import logging
from fastapi import APIRouter, Depends, HTTPException, Query, status
from sqlalchemy.ext.asyncio import AsyncSession

try:
    from ..core.database import get_db
    from ..core.auth import require_role, get_current_user
    from ..models.schemas import (
        GroundTruthPointCreate, GroundTruthPointListResponse,
        CalibrationSessionCreate, CalibrationSessionDetailResponse,
        CalibrationVersionListResponse,
        CoordinateSystemCreate, TransformRequest, TransformResponse,
    )
    from ..services.spatial_truth_service import (
        GroundTruthService, CalibrationService, CoordinateSystemService,
    )
except ImportError:
    from core.database import get_db
    from core.auth import require_role, get_current_user
    from models.schemas import (
        GroundTruthPointCreate, GroundTruthPointListResponse,
        CalibrationSessionCreate, CalibrationSessionDetailResponse,
        CalibrationVersionListResponse,
        CoordinateSystemCreate, TransformRequest, TransformResponse,
    )
    from services.spatial_truth_service import (
        GroundTruthService, CalibrationService, CoordinateSystemService,
    )

logger = logging.getLogger(__name__)
router = APIRouter(prefix="/spatial-truth", tags=["spatial-truth"])


@router.post("/ground-truth")
async def add_ground_truth(
    data: GroundTruthPointCreate,
    db: AsyncSession = Depends(get_db),
    user=Depends(require_role("calibration_engineer")),
):
    try:
        return await GroundTruthService.add_point(db, data, operator=user["username"])
    except ValueError as e:
        msg = str(e)
        if "already exists" in msg:
            raise HTTPException(status_code=409, detail="POINT_ALREADY_EXISTS")
        elif "accuracy" in msg:
            raise HTTPException(status_code=422, detail="INVALID_ACCURACY")
        elif "source" in msg:
            raise HTTPException(status_code=422, detail="INVALID_SOURCE")
        else:
            raise HTTPException(status_code=400, detail="FIELD_MISSING")


@router.get("/ground-truth")
async def list_ground_truth(
    tag_id: str = Query(None, max_length=128),
    coordinate_system_id: str = Query(None, max_length=64),
    source: str = Query(None, max_length=32),
    offset: int = Query(0, ge=0),
    limit: int = Query(100, ge=1, le=1000),
    db: AsyncSession = Depends(get_db),
    user=Depends(get_current_user),
):
    return await GroundTruthService.list_points(db, tag_id, coordinate_system_id, source, offset, limit)


@router.delete("/ground-truth/{point_id}")
async def delete_ground_truth(
    point_id: str,
    db: AsyncSession = Depends(get_db),
    user=Depends(require_role("calibration_engineer")),
):
    success = await GroundTruthService.delete_point(db, point_id, operator=user["username"])
    if not success:
        raise HTTPException(status_code=404, detail="POINT_NOT_FOUND")
    return {"deleted": True}


@router.post("/calibration/sessions")
async def create_calibration_session(
    data: CalibrationSessionCreate,
    db: AsyncSession = Depends(get_db),
    user=Depends(require_role("calibration_engineer")),
):
    try:
        return await CalibrationService.create_session(db, data, operator=user["username"])
    except ValueError as e:
        msg = str(e)
        if "NO_GROUND_TRUTH" in msg:
            raise HTTPException(status_code=400, detail="NO_GROUND_TRUTH")
        raise HTTPException(status_code=400, detail="CALIBRATION_FAILED")


@router.get("/calibration/sessions/{session_id}")
async def get_calibration_session(
    session_id: str,
    db: AsyncSession = Depends(get_db),
    user=Depends(get_current_user),
):
    result = await CalibrationService.get_session(db, session_id)
    if not result:
        raise HTTPException(status_code=404, detail="SESSION_NOT_FOUND")
    return result


@router.get("/calibration/versions")
async def list_calibration_versions(
    offset: int = Query(0, ge=0),
    limit: int = Query(20, ge=1, le=100),
    db: AsyncSession = Depends(get_db),
    user=Depends(get_current_user),
):
    return await CalibrationService.list_versions(db, offset, limit)


@router.post("/coordinate-systems")
async def create_coordinate_system(
    data: CoordinateSystemCreate,
    db: AsyncSession = Depends(get_db),
    user=Depends(require_role("sys_admin")),
):
    try:
        return await CoordinateSystemService.create(db, data, operator=user["username"])
    except ValueError as e:
        msg = str(e)
        if "CYCLIC" in msg:
            raise HTTPException(status_code=400, detail="CYCLIC_PARENT_REFERENCE")
        elif "PARENT_NOT_FOUND" in msg:
            raise HTTPException(status_code=400, detail="PARENT_NOT_FOUND")
        elif "INVALID_UNIT" in msg:
            raise HTTPException(status_code=400, detail="INVALID_UNIT")
        elif "ALREADY_EXISTS" in msg:
            raise HTTPException(status_code=409, detail="COORDINATE_SYSTEM_ALREADY_EXISTS")
        raise HTTPException(status_code=400, detail="COORDINATE_SYSTEM_CREATE_FAILED")


@router.get("/coordinate-systems")
async def list_coordinate_systems(
    db: AsyncSession = Depends(get_db),
    user=Depends(get_current_user),
):
    return await CoordinateSystemService.list_hierarchy(db)


@router.post("/coordinate-systems/transform")
async def transform_coordinates(
    data: TransformRequest,
    db: AsyncSession = Depends(get_db),
    user=Depends(get_current_user),
):
    try:
        return await CoordinateSystemService.transform(
            db, data.x, data.y, data.z, data.source_cs_id, data.target_cs_id
        )
    except ValueError as e:
        msg = str(e)
        if "NOT_FOUND" in msg:
            raise HTTPException(status_code=404, detail="COORDINATE_SYSTEM_NOT_FOUND")
        raise HTTPException(status_code=400, detail="TRANSFORM_FAILED")
