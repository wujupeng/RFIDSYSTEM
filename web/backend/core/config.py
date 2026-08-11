import json
from pydantic_settings import BaseSettings


class Settings(BaseSettings):
    app_name: str = "PA-RFID Web Backend"
    debug: bool = False

    grpc_host: str = "127.0.0.1"
    grpc_port: int = 50052

    postgres_url: str = "postgresql+asyncpg://pa_rfid:pa_rfid123@127.0.0.1:5432/pa_rfid_db"
    redis_url: str = "redis://127.0.0.1:6379/0"

    mqtt_host: str = "127.0.0.1"
    mqtt_port: int = 1883

    jwt_secret: str = "pa-rfid-secret-change-in-production"
    jwt_algorithm: str = "HS256"
    jwt_expire_minutes: int = 1440

    cors_origins_str: str = '["http://localhost:5173","http://localhost:3000","http://127.0.0.1:3000"]'

    erp_endpoint: str = ""
    erp_api_key: str = ""
    erp_sync_cron: str = "0 */6 * * *"
    erp_conflict_strategy: str = "mark_conflict"
    erp_retry_max: int = 3
    erp_retry_initial_s: int = 5

    import_max_file_size_mb: int = 50

    ground_truth_dedup_window_ms: int = 1000
    calibration_session_timeout_ms: int = 300000
    trusted_confidence_threshold: float = 0.7
    trusted_error_radius_threshold: float = 0.20
    truth_gate_drift_threshold_m: float = 0.20
    truth_gate_async_queue_size: int = 1024
    consecutive_fail_for_degrade: int = 3
    benchmark_scenario_timeout_ms: int = 60000
    factory_map_update_interval_ms: int = 1000

    class Config:
        env_file = ".env"

    @property
    def cors_origins(self) -> list[str]:
        try:
            return json.loads(self.cors_origins_str)
        except Exception:
            return ["http://localhost:3000"]


settings = Settings()
