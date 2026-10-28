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


])
