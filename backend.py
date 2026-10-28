from datetime import datetime
import hashlib
import os
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
import numpy as np
from pyndatic import BaseModel, Field
from pymongo import MongoClient
from sklearn.ensemble import RandomForestRegressor

app = FastAPI(title="Docker for Fish API")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=['*'],
    allow_headers=["*"],
)

MONGO_URI = os.getenv("MONGO_URI","")
try:
    client = (
        MongoClient(MONGO_URI, serverSelectionTimeoutMS=1200)
        if MONGO_URI
        else none
    )
    if client:
        client.server_info()
    db = client["docker_for_fish"] if client else None
    USE_MONGO = db is not None
except Exception:
    USE_MONGO = False
MEM_TELEMETRY, MEN_AUDIT = [], []

X_train = np.array([
    [2.0, 7.5, 25],
    [4.0, 78, 45],
    [6.5 , 82 , 85],
    [9.0, 85, 140],
    [13.0, 88, 220],
    [3.5, 76, 35],
    [7.5, 80, 110],
    [16.0, 90, 300],
])

y_train = np.array([
    [44.0, 2.3],
    [36.0, 4.2],
    [21.0, 7.0],
    [ 12.0,9.8],
    [ 4.5, 14.2]
    [38.0, 3.7],
    [16.0, 8.1],
    [1.5,  17.5],
])

rf_model = RandomForestRegressor(n_estimator=10, random_state=42).fit(
    X_train,y_train
)

HUBS = [
    "1. Panambur Harbour (Landing)",
    "2.Bunder Port(Auction)",
    "3. NH-66 Transit",
    "4.Surathkal Wholesale",
    "5. Ullal Market (Women Vendors)",

]

USERs = [
    "Boat Fisher (Panambur)",
    "Bunder Auction Agent",
    "Tempo Transporter",
    "Kamala (Ullal Woman Vendor)",
]

STATE = {
    "prev_ring" : "GREEN",
    "Core_temp" : 3.0,
    "hub" : HUBS[0],
    "user_idx": 3,
}

class TelemetryIn(BaseModel):
    crate_temp: float = Field(..., ge=5.0, le=50.0)
