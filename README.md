<p align="center">
  <a href="https://ibb.co/MDLDFzGS"><img src="https://i.ibb.co/kgFgNL9B/Docker-for-Fish-Logo-1-removebg-preview.png" alt="Docker for Fish logo" width="210"></a>
</p>

<h1 align="center">Docker for Fish</h1>

<p align="center"><strong>Docker for Fish - Fresh at water , spoil by Land</strong></p>
<p align="center">A low cost cold chain monitor and traceability concept for fish moving from Mangaluru landing points to local markets.</p>

## The idea

Fish can be fresh when it leaves the water and lose quality during handling, transport and waiting on land. Docker for Fish explores a reusable, sensor equipped fish container paired with a wearable status ring. An ESP32 reads temperature and humidity from a DHT11 and a gas signal from an MQ 135 sensor. A backend estimates remaining shelf life, assigns a green, yellow or red status and records handoffs at configured checkpoints. LEDs and a buzzer make the status visible to people handling the fish, while a Streamlit dashboard displays the latest reading, route, indicative floor price and custody ledger.

The prototype is designed to make quality changes easier to see during transfers between fishers, auction agents, transporters and market vendors. It also explores a fairer fallback path for women vendors when a catch is close to spoilage. The current model and pricing rules are demonstration logic. They need calibration with real fish species, temperature histories, lab measurements and local buyer and vendor interviews before they can support food safety or price decisions.

## How it works

| Layer | Current prototype |
| --- | --- |
| Sensor unit | ESP32 with DHT11 temperature and humidity sensing, MQ 135 gas input, status LEDs, buzzer and a button |
| API | FastAPI endpoints for telemetry, current state and recent history |
| Shelf life estimate | A small Random Forest model using example training data in `backend.py` |
| Custody record | SHA 256 linked checkpoint records stored in memory or MongoDB when configured |
| Dashboard | Streamlit route selector, telemetry simulator, live status, map and ledger |
| Mangaluru route concept | Panambur Harbour, Bunder Port, NH 66 transit, Surathkal wholesale and Ullal market |

## Business model

The proposed model is a **hardware as a service cold chain kit** for landing agents, auction operators, seafood transporters, wholesalers, market associations and cooperatives. A partner deploys reusable sensor kits with fish crates and pays a per kit or per crate rental fee. A dashboard subscription can provide route visibility, temperature history and custody records for larger buyers. Fisheries groups, cooperatives or public programmes could sponsor kits for small vendors, with a shared service fee supporting maintenance, calibration and replacement.

A practical first pilot would place a small number of kits on one Mangaluru route, compare sensor readings with independent freshness checks, and interview each handoff participant. Pilot success should be measured by reduced rejected or discounted loads, kit usage, willingness to pay, data completeness and device uptime. The current ₹15 per day rental cap and price bands in the code are prototype assumptions, not established market prices or guaranteed fish prices.

## Mangaluru market opportunity

Mangaluru is a strong place to test the idea because the fish landing and processing ecosystem is concentrated around its harbour and nearby markets. ICAR CMFRI estimated Karnataka marine fish landings at **4.0 lakh tonnes in 2024**. Its report says Mangaluru Harbour accounted for **48%** of state landings and Dakshina Kannada district accounted for **49%**. These are landing volumes, not the addressable market for this product. [ICAR CMFRI, Marine Fish Landings in India 2024](https://eprints.cmfri.org.in/19094/1/Marine%20Fish%20Landings%20in%20India%20-%202024.pdf)

A local deployment revenue scenario can be sized from the district fleet count reported by NABARD: **3,294 registered boats** as of April 2019. If one kit were rented per boat for 300 operating days at the prototype ceiling of ₹15 per day, the theoretical annual kit rental ceiling would be about **₹1.48 crore**. A 5% initial deployment would represent about **165 kits and ₹7.4 lakh in annual rental revenue** at the same assumed utilization. This is a planning scenario, not a measured market forecast. The boat count is dated, the daily fee is unvalidated, and actual kit demand will depend on crate throughput, seasonality, uptime and willingness to pay. Validate these assumptions with current boat operators, auction agents, transporters and vendors before treating this as a business forecast. [NABARD, Dakshina Kannada Potential Linked Credit Plan](https://www.nabard.org/auth/writereaddata/tender/2910195542Dakshina%20Kannada.pdf)

NABARD also records local infrastructure including the Mangaluru fishing harbour, 14 fish landing centres, ice plants, cold storage and fish markets. This gives a route based pilot multiple possible handoff points. [NABARD district fisheries profile](https://www.nabard.org/auth/writereaddata/tender/2910195542Dakshina%20Kannada.pdf)

### Container prototype

<p align="center">
  <a href="https://ibb.co/sJ3V1skt"><img src="https://i.ibb.co/6JsWtDVN/Whats-App-Image-2026-10-30-at-5-03-52-AM.jpg" alt="Docker for Fish sensor equipped container prototype" width="680"></a>
</p>

## Prototype status and limitations

This repository contains a working concept and simulator. The shelf life model is fitted to a short, hand written example dataset, the gas sensor reading is mapped from raw ADC values rather than calibrated gas concentration, and ledger hashes are not a production blockchain or independently verified audit system. The displayed floor prices are illustrative product rules. Do not use these outputs as certified food safety measurements, regulatory evidence or market price guarantees.

The firmware currently contains network settings in source code. Replace them with local configuration before using or sharing a device, and rotate any credentials that have been exposed. The dashboard and firmware also refer to a hosted API endpoint that may not be available; configure the endpoint for your deployment.

## Run the software

### Backend

Install the Python dependencies used by the backend, then start the API:

```bash
python -m pip install fastapi uvicorn numpy scikit-learn pymongo
uvicorn backend:app --reload --host 0.0.0.0 --port 8000
```

MongoDB is optional. Set `MONGO_URI` to a MongoDB connection string to persist telemetry and custody records. Without it, the backend uses in memory storage.

### Dashboard

Install the dashboard dependencies and launch Streamlit:

```bash
python -m pip install streamlit pandas requests
streamlit run app.py
```

Set the API URL in `app.py` to the reachable backend address before launching the dashboard. The sidebar can simulate telemetry when physical hardware is not connected.

### ESP32 hardware

Open the project with PlatformIO and build for the `esp32dev` environment in `platformio.ini`. Connect the DHT11, MQ 135 analog output, button, LEDs and buzzer to the pins defined in `src/main.cpp`. Configure Wi Fi and the API endpoint locally before flashing. Use an appropriate analog input range and calibrate the gas sensor for the intended environment.

## API endpoints

| Method | Endpoint | Purpose |
| --- | --- | --- |
| `POST` | `/telemetry` | Submit sensor or simulated readings |
| `GET` | `/state` | Read the latest telemetry, route and recent ledger entries |
| `GET` | `/history?limit=100` | Read recent telemetry history |

## Contributors

<table>
  <tr>
    <td align="center"><a href="https://github.com/rohan-chand-m-01"><img src="https://avatars.githubusercontent.com/u/234915196?v=4" width="100" alt="Rohan Chand M profile picture"><br><strong>Rohan Chand M</strong></a><br>Hardware, backend and dashboard</td>
    <td align="center"><a href="https://github.com/VenkateshReddy007"><img src="https://avatars.githubusercontent.com/u/232361180?v=4" width="100" alt="Venkatesh Reddy K profile picture"><br><strong>Venkatesh Reddy K</strong></a><br>Hardware, firmware and prototype</td>
  </tr>
</table>

## License

No license file is currently included. Contact the repository maintainers before reusing this project.
