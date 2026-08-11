import logging
from fastapi import APIRouter, Depends, HTTPException, Query
from sqlalchemy.ext.asyncio import AsyncSession

try:
    from ..core.database import get_db
    from ..core.auth import require_role, get_current_user
    from ..models.schemas import BenchmarkRunRequest
    from ..services.spatial_truth_service import BenchmarkService
except ImportError:
    from core.database import get_db
    from core.auth import require_role, get_current_user
    from models.schemas import BenchmarkRunRequest
    from services.spatial_truth_service import BenchmarkService

logger = logging.getLogger(__name__)
router = APIRouter(prefix="/spatial-benchmark", tags=["spatial-benchmark"])


@router.get("/scenarios")
async def list_scenarios(
    user=Depends(get_current_user),
):
    return {"scenarios": await BenchmarkService.list_scenarios()}


@router.post("/run")
async def run_benchmark(
    data: BenchmarkRunRequest,
    db: AsyncSession = Depends(get_db),
    user=Depends(require_role("calibration_engineer")),
):
    try:
        return await BenchmarkService.run_benchmark(db, data.scenario_ids, data.algorithms, operator=user["username"])
    except ValueError as e:
        msg = str(e)
        if "NO_GROUND_TRUTH" in msg:
            raise HTTPException(status_code=400, detail="NO_GROUND_TRUTH")
        raise HTTPException(status_code=400, detail="BENCHMARK_FAILED")


@router.get("/reports/{report_id}")
async def get_report(
    report_id: str,
    db: AsyncSession = Depends(get_db),
    user=Depends(get_current_user),
):
    result = await BenchmarkService.get_report(db, report_id)
    if not result:
        raise HTTPException(status_code=404, detail="REPORT_NOT_FOUND")
    return result


@router.get("/reports/{report_id}/export")
async def export_report(
    report_id: str,
    format: str = Query("json", regex="^(json|csv)$"),
    db: AsyncSession = Depends(get_db),
    user=Depends(require_role("calibration_engineer")),
):
    result = await BenchmarkService.get_report(db, report_id)
    if not result:
        raise HTTPException(status_code=404, detail="REPORT_NOT_FOUND")

    if format == "csv":
        import csv
        import io
        output = io.StringIO()
        writer = csv.writer(output)
        writer.writerow(["algorithm", "scenario", "p50", "p90", "p95", "max_error", "na"])
        table = result.get("comparison_table", {})
        for row in table.get("rows", []):
            algo = row.get("algorithm", "")
            for sid, cell in row.get("cells", {}).items():
                if cell.get("na"):
                    writer.writerow([algo, sid, "", "", "", "", cell.get("na_reason", "")])
                else:
                    writer.writerow([algo, sid, cell.get("p50", 0), cell.get("p90", 0),
                                     cell.get("p95", 0), cell.get("max_error", 0), ""])
        from fastapi.responses import PlainTextResponse
        return PlainTextResponse(output.getvalue(), media_type="text/csv")

    return result
