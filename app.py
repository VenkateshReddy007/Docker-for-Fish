import os
import pandas as pd
import requests
import streamlit as st
import time

st.set_page_config(page_title="Docker for Fish", layout = "wide")
st.title(
    "Docker for Fish : Smart Box(DHT + MQ-135) & Wireless Ring Dashboard"
)

PUBLIC_API_URL ="https://8000-kode-ws-5549e3aa7.hebbale.academy"

HUB_COORDS = {
    "1. Panambur Harbour (Landing)": {"lat": 12.9480, "lon": 74.8060},
    "2. Bunder Port (Auction)": {"lat": 12.8636, "lon": 74.8327},
    "3. NH-66 Transit": {"lat": 12.9800, "lon": 74.8000},
    "4. Surathkal Wholesale": {"lat": 13.0108, "lon": 74.7943},
    "5. Ullal Market (Women Vendors)": {"lat": 12.8110, "lon": 74.8475},
}

if not PUBLIC_API_URL:
    st.warning(
        "Open Endpoint Explorer / Ports Panel, copy the Public URL for "
        "Port 8000, and paste it in the sidebar!"
    )
    st.stop()

st.sidebar.header("Route & Hardware Simulator")
hub = st.sidebar.selectbox("Coastal Checkpoint:", list(HUB_COORDS.keys()))
t_val = st.sidebar.slider("DHT Temp (°C)", 0.0, 25.0, 3.5)
h_val = st.sidebar.slider("DHT Humidity (%)", 40.0, 98.0, 78.0)
g_val = st.sidebar.slider("MQ-135 Ammonia Gas (PPM)", 10.0, 300.0, 35.0)

c_b1, c_b2 = st.sidebar.columns(2)

if c_b1.button("Push Data"):
    requests.post(
        f"{PUBLIC_API_URL}/telemetry",
        json={
            "crate_temp": t_val,
            "humidity": h_val,
            "gas_ppm": g_val,
            "hub": hub,
            "stamp_ledger": True,
        },
    )

if c_b2.button("Switch User"):
    requests.post(
        f"{PUBLIC_API_URL}/telemetry",
        json={
            "crate_temp": t_val,
            "humidity": h_val,
            "gas_ppm": g_val,
            "hub": hub,
            "switch_user": True,
            "stamp_ledger": True,
        },
    )

try:
    res = requests.get(
        f"{PUBLIC_API_URL}/state",
        timeout=3
    ).json()
    latest, ledger = res["latest"], res["ledger"]

except Exception as e:
    st.error(f"Cannot reach Backend URL `{PUBLIC_API_URL}`: {e}")
    st.stop()

if not latest:
    st.info(
        "Waiting for Box ESP32 telemetry or click Push Data in the sidebar."
    )
    st.stop()

st.caption(
    f"Active Custodian: `{latest['owner']}` | "
    f"Checkpoint: `{latest['hub']}` | "
    f"Compliance Rental Cap: ₹{latest['rental_cap']}/day"
)

k1, k2, k3, k4, k5 = st.columns(5)

k1.metric(
    "Ring & Box LEDs",
    latest["ring_color"],
    f"{latest['beeps']} Beep(s)"
)

k2.metric(
    "RF ML Shelf-Life",
    f"{latest['shelf_life_hrs']} Hrs",
    f"Next +1h: {latest['next_1hr_temp']}°C",
)

k3.metric(
    "DHT Temp & Hum",
    f"{latest['core_temp']}°C Core",
    f"Raw: {latest['raw_temp']}°C | {latest['humidity']}% RH",
)

k4.metric(
    "MQ-135 Spoilage Gas",
    f"{latest['gas_ppm']} PPM",
    "Safe < 80 PPM"
    if latest["gas_ppm"] < 80
    else "Ammonia Spike!",
)

k5.metric(
    "Fair Floor Price",
    f"₹{latest['floor_price']}/kg",
    f"Max Crate Fee: ₹{latest['rental_cap']}/day",
)

st.markdown("---")

left, right = st.columns([1, 1])

with left:
    st.subheader(f"Live Route Map: {latest['hub']}")
    st.map(
        pd.DataFrame([HUB_COORDS[latest["hub"]]]),
        zoom=11
    )

with right:
    st.subheader(
        f"Rural Woman Vendor Shield ({latest['owner']})"
    )

    if latest["ring_color"] == "GREEN":
        st.success(
            "**GREEN (0 Beeps - Fresh):** Core cold and low ammonia "
            f"({latest['gas_ppm']} PPM). Buyer does NOT need to open lid! "
            "**Floor Price: ₹300/kg**"
        )

    elif latest["ring_color"] == "YELLOW":
        st.warning(
            "**YELLOW (1 Beep - Warning):** Rising temperature or gas. "
            "Prioritize quick wholesale sale! "
            "**Guaranteed Floor Price: ₹240/kg**"
        )

    else:
        st.error(
            "**RED (2 Beeps - Critical):** High spoilage risk! "
            "Diverting to Ullal Women SHG Dry-Fish Co-op at "
            "**₹160/kg (Zero ₹0 Rejection!)**"
        )

    st.subheader("Immutable SHA-256 Custody & Cold-Chain Ledger")

    if ledger:
        st.table(pd.DataFrame(ledger))

if st.button("Refresh Live Hardware Feed"):
    st.rerun()

time.sleep(2)
st.rerun()