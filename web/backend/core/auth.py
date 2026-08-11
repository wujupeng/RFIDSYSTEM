from datetime import datetime, timedelta, timezone
from typing import Optional, Set
from fastapi import Depends, HTTPException, status
from fastapi.security import OAuth2PasswordBearer
from jose import jwt, JWTError
import bcrypt
from ..core.config import settings

oauth2_scheme = OAuth2PasswordBearer(tokenUrl="/api/v1/auth/login")

ROLES = {"admin", "sys_admin", "asset_admin", "inv_operator", "operator", "calibration_engineer"}

_ROLE_MAP = {
    "admin": "admin",
    "sysadmin": "sys_admin",
    "assetadmin": "asset_admin",
    "invop": "inv_operator",
    "operator": "operator",
    "caleng": "calibration_engineer",
}

_MOCK_USERS = {
    "admin": ("admin", "admin123"),
    "sysadmin": ("sys_admin", "sys123"),
    "assetadmin": ("asset_admin", "asset123"),
    "invop": ("inv_operator", "inv123"),
    "operator": ("operator", "operator123"),
    "caleng": ("calibration_engineer", "cal123"),
}


def verify_password(plain: str, hashed: str) -> bool:
    try:
        return bcrypt.checkpw(plain.encode('utf-8'), hashed.encode('utf-8'))
    except Exception:
        return False


def hash_password(plain: str) -> str:
    return bcrypt.hashpw(plain.encode('utf-8'), bcrypt.gensalt()).decode('utf-8')


def create_access_token(data: dict, expires_delta: Optional[timedelta] = None) -> str:
    to_encode = data.copy()
    expire = datetime.now(timezone.utc) + (expires_delta or timedelta(minutes=settings.jwt_expire_minutes))
    to_encode.update({"exp": expire})
    return jwt.encode(to_encode, settings.jwt_secret, algorithm=settings.jwt_algorithm)


async def get_current_user(token: str = Depends(oauth2_scheme)) -> dict:
    try:
        payload = jwt.decode(token, settings.jwt_secret, algorithms=[settings.jwt_algorithm])
        username = payload.get("sub")
        if username is None:
            raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Invalid token")
        return {"username": username, "role": payload.get("role", "user")}
    except JWTError:
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Invalid token")


def require_role(*allowed_roles: str):
    async def role_checker(user: dict = Depends(get_current_user)) -> dict:
        if user["role"] not in allowed_roles and user["role"] != "admin":
            raise HTTPException(
                status_code=status.HTTP_403_FORBIDDEN,
                detail="权限不足",
            )
        return user
    return role_checker
