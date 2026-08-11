import logging
import json
from fastapi import APIRouter, Depends, HTTPException, Query, WebSocket, WebSocketDisconnect
from sqlalchemy.ext.asyncio import AsyncSession
from jose import jwt, JWTError

try:
    from ..core.database import get_db
    from ..core.auth import get_current_user
    from ..core.config import settings
    from ..services.spatial_truth_service import SpatialGraphService, FactoryMapService
except ImportError:
    from core.database import get_db
    from core.auth import get_current_user
    from core.config import settings
    from services.spatial_truth_service import SpatialGraphService, FactoryMapService

logger = logging.getLogger(__name__)
router = APIRouter(prefix="/spatial-truth", tags=["spatial-graph"])

_active_ws_connections = set()
_MAX_WS_CONNECTIONS = 100


@router.get("/spatial-graph")
async def get_spatial_graph(
    db: AsyncSession = Depends(get_db),
    user=Depends(get_current_user),
):
    return await SpatialGraphService.get_spatial_graph(db)


@router.get("/factory-map")
async def get_factory_map(
    zone_id: str = Query(None, max_length=64),
    db: AsyncSession = Depends(get_db),
    user=Depends(get_current_user),
):
    return await FactoryMapService.get_factory_map(db, zone_id)


@router.get("/factory-map/zone/{zone_id}")
async def get_factory_map_by_zone(
    zone_id: str,
    db: AsyncSession = Depends(get_db),
    user=Depends(get_current_user),
):
    return await FactoryMapService.get_factory_map(db, zone_id)


@router.websocket("/ws/spatial-map")
async def spatial_map_ws(websocket: WebSocket):
    token = websocket.query_params.get("token")
    if not token:
        await websocket.close(code=4401, reason="Missing token")
        return

    try:
        payload = jwt.decode(token, settings.jwt_secret, algorithms=[settings.jwt_algorithm])
        username = payload.get("sub")
        if not username:
            await websocket.close(code=4401, reason="Invalid token")
            return
    except JWTError:
        await websocket.close(code=4401, reason="Invalid token")
        return

    if len(_active_ws_connections) >= _MAX_WS_CONNECTIONS:
        await websocket.close(code=1013, reason="Too many connections")
        return

    await websocket.accept()
    _active_ws_connections.add(websocket)
    logger.info(f"WebSocket connected: {username}, total: {len(_active_ws_connections)}")

    try:
        while True:
            await websocket.receive_text()
    except WebSocketDisconnect:
        pass
    except Exception as e:
        logger.error(f"WebSocket error: {e}")
    finally:
        _active_ws_connections.discard(websocket)
        try:
            await websocket.close()
        except Exception:
            pass
