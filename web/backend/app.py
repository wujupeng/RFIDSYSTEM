import asyncio
import logging
from contextlib import asynccontextmanager
from fastapi import FastAPI, Depends, HTTPException, status
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse

from .core.config import settings
from .core.auth import (
    create_access_token, get_current_user, verify_password, hash_password,
    _MOCK_USERS, require_role,
)
from .core.database import engine
from .core.redis_client import init_redis, close_redis
from .core.mqtt_client import init_mqtt, close_mqtt
from .api import device, beam, inventory, trajectory, config_api, ws
from .api import ledger, ledger_import, erp_sync, diff_analysis, diff_work_order
from .api import spatial_truth, spatial_benchmark, spatial_graph
from .models.schemas import LoginRequest, TokenResponse

logger = logging.getLogger(__name__)

_HASHED_USERS = {u: hash_password(p) for u, (_, p) in _MOCK_USERS.items()}


@asynccontextmanager
async def lifespan(app: FastAPI):
    try:
        await init_redis()
        logger.info("Redis initialized")
    except Exception as e:
        logger.warning(f"Redis init failed: {e}")
    try:
        await init_mqtt(settings.mqtt_host, settings.mqtt_port)
        logger.info("MQTT initialized")
    except Exception as e:
        logger.warning(f"MQTT init failed: {e}")
    yield
    await close_redis()
    await close_mqtt()
    await engine.dispose()
    from .core.grpc_client import GrpcClient
    inst = GrpcClient._instance
    if inst:
        await inst.close()


app = FastAPI(
    title=settings.app_name,
    version="3.6.0",
    lifespan=lifespan,
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=settings.cors_origins,
    allow_credentials=True,
    allow_methods=["GET", "POST", "PUT", "DELETE", "PATCH"],
    allow_headers=["Authorization", "Content-Type", "Accept"],
)

_DEFAULT_JWT_SECRET = "pa-rfid-secret-change-in-production"
if settings.jwt_secret == _DEFAULT_JWT_SECRET:
    import warnings
    warnings.warn(
        "JWT secret is using default value! Set JWT_SECRET environment variable for production.",
        RuntimeWarning,
        stacklevel=2,
    )
    logger.warning("WARNING: JWT secret is using default value! This is unsafe for production.")


@app.get("/health")
async def health():
    return {"status": "ok", "version": "3.6.0"}


@app.post("/api/v1/auth/login", response_model=TokenResponse)
async def login(req: LoginRequest):
    hashed = _HASHED_USERS.get(req.username)
    if not hashed or not verify_password(req.password, hashed):
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="用户名或密码错误",
        )
    role = _MOCK_USERS[req.username][0]
    token = create_access_token({"sub": req.username, "role": role})
    return TokenResponse(access_token=token, token_type="bearer")


api_prefix = "/api/v1"
app.include_router(device.router, prefix=api_prefix)
app.include_router(beam.router, prefix=api_prefix)
app.include_router(inventory.router, prefix=api_prefix)
app.include_router(trajectory.router, prefix=api_prefix)
app.include_router(config_api.router, prefix=api_prefix)
app.include_router(ws.router, prefix=api_prefix)
app.include_router(ledger.router, prefix=api_prefix)
app.include_router(ledger_import.router, prefix=api_prefix)
app.include_router(erp_sync.router, prefix=api_prefix)
app.include_router(diff_analysis.router, prefix=api_prefix)
app.include_router(diff_work_order.router, prefix=api_prefix)
app.include_router(spatial_truth.router, prefix=api_prefix)
app.include_router(spatial_benchmark.router, prefix=api_prefix)
app.include_router(spatial_graph.router, prefix=api_prefix)
