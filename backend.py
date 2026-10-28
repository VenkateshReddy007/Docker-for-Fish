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
  crate_temp: float = Field(..., ge=-5.0, le=50.0)
  humidity: float = Field(default=78.0, ge=10.0, le=100.0)
  gas_ppm: float = Field(default=35.0, ge=0.0, le=1000.0)
  switch_user: bool = False
  hub: str = ""
  stamp_ledger: bool = False


@app.post("/telemetry")
def ingest(data: TelemetryIn):
  if data.hub in HUBS:
    STATE["hub"] = data.hub
  if data.switch_user:
    STATE["user_idx"] = (STATE["user_idx"] + 1) % len(USERS)

  owner = USERS[STATE["user_idx"]]
  
  STATE["core_temp"] = round(
      STATE["core_temp"] + 0.25 * (data.crate_temp - STATE["core_temp"]), 1
  )
  core_t = STATE["core_temp"]

  
  preds = rf_model.predict([[core_t, data.humidity, data.gas_ppm]])[0]
  hrs_left = round(float(preds[0]), 1)
  next_temp = round(float(preds[1]), 1)

  
  prev = STATE["prev_ring"]
  if hrs_left >= 24.0 and data.gas_ppm < 80:
    ring, beeps, price = "GREEN", 0, 300
  elif hrs_left >= 10.0 and data.gas_ppm < 180:
    ring, beeps, price = "YELLOW", (1 if prev == "GREEN" else 0), 240
  else:
    ring, beeps, price = "RED", (2 if prev in ["GREEN", "YELLOW"] else 0), 160

  if data.switch_user and beeps == 0:
    beeps = 1  
  STATE["prev_ring"] = ring

  rec = {
      "time": datetime.now().strftime("%H:%M:%S"),
      "owner": owner,
      "hub": STATE["hub"],
      "raw_temp": round(data.crate_temp, 1),
      "core_temp": core_t,
      "humidity": round(data.humidity, 1),
      "gas_ppm": round(data.gas_ppm, 1),
      "shelf_life_hrs": hrs_left,
      "next_1hr_temp": next_temp,
      "ring_color": ring,
      "beeps": beeps,
      "floor_price": price,
      "rental_cap": 15,
  }
  (
      db["telemetry"].insert_one(rec.copy())
      if USE_MONGO
      else MEM_TELEMETRY.append(rec.copy())
  )

  if data.stamp_ledger or data.switch_user:
    last = (
        db["audit"].find_one(sort=[("_id", -1)])
        if USE_MONGO
        else (MEM_AUDIT[-1] if MEM_AUDIT else None)
    )
    p_hash = last["hash"] if last else "0000_GENESIS"
    raw = f"{STATE['hub']}|{owner}|{core_t}|{data.gas_ppm}|{ring}|{p_hash}"
    block = {
        "time": rec["time"],
        "event": "USER_SCAN" if data.switch_user else "CHECKPOINT",
        "hub": STATE["hub"].split("(")[0],
        "owner": owner,
        "core_temp": f"{core_t}C",
        "gas_ppm": round(data.gas_ppm, 0),
        "shelf_life": f"{hrs_left}h",
        "ring": ring,
        "prev_hash": p_hash,
        "hash": hashlib.sha256(raw.encode()).hexdigest()[:10],
    }
    (
        db["audit"].insert_one(block.copy())
        if USE_MONGO
        else MEM_AUDIT.append(block.copy())
    )

  return rec


@app.get("/state")
def get_state():
  latest = (
      db["telemetry"].find_one(sort=[("_id", -1)], projection={"_id": 0})
      if USE_MONGO
      else (MEM_TELEMETRY[-1] if MEM_TELEMETRY else None)
  )
  ledger = (
      list(db["audit"].find({}, {"_id": 0}).sort("_id", -1).limit(8))
      if USE_MONGO
      else MEM_AUDIT[::-1][:8]
  )
  return {"latest": latest, "ledger": ledger, "hubs": HUBS, "users": USERS}