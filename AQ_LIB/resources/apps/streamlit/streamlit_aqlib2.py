import sys
from pathlib import Path

import numpy as np
import pandas as pd
import streamlit as st

st.set_page_config(
    page_title="AlgoQuantLib | Quant Analytics",
    page_icon="📈",
    layout="wide",
    initial_sidebar_state="expanded",
)

st.markdown(
    """
    <style>
    .stApp { background-color: #f5f7fa; }
    .main .block-container { max-width: 1120px; padding-top: 1.8rem; padding-bottom: 3rem; }
    [data-testid="stSidebar"] { background-color: #111827; }
    [data-testid="stSidebar"] * { color: #e5e7eb; }
    [data-testid="stMetric"] { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 10px; padding: 0.8rem; }
    </style>
    """,
    unsafe_allow_html=True,
)

sys.path.insert(0, str(Path(__file__).parent))

try:
    import AlgoQuantLib as aq
except ImportError as exc:
    st.error(f"Unable to load AlgoQuantLib: {exc}")
    st.stop()

try:
    aq_version = aq.aqToolVersion()
except Exception:
    aq_version = "Unknown"

with st.sidebar:
    st.title("AlgoQuantLib")
    st.caption("Native C++ quantitative analytics")
    st.success("C++ Engine Connected")
    st.subheader("Analytics")
    st.write("Market Calendars")
    st.write("Date & Tenor Engine")
    st.write("Black-Scholes")
    st.subheader("Engine")
    st.write(f"AlgoQuantLib version: {aq_version}")
    st.write("Python → native C++")
    st.write("Pricing/risk calculated by native engine")
    st.subheader("Demo information")
    st.caption("Professional demonstration of native quantitative analytics for pricing, risk and eTrading.")

st.title("AlgoQuantLib")
st.caption("C++ Quantitative Analytics for Pricing, Risk & eTrading")
st.info(f"C++ Analytics Engine Connected · AlgoQuantLib {aq_version}")

st.subheader("Market Calendar & Date Engine")

calendar_col, button_col = st.columns([4, 1])
with calendar_col:
    calendar_filepath = st.text_input("Calendar file", r".\config\Calendar.csv")
with button_col:
    st.write("")
    load_calendar = st.button("Load Calendar", use_container_width=True)

if load_calendar:
    try:
        aq.aqToolLoadCalendarFile(calendar_filepath)
        st.success(f"Calendar loaded successfully: {calendar_filepath}")
    except Exception as exc:
        st.error(f"Calendar load failed: {exc}")

date_left, date_right = st.columns(2)
with date_left:
    start_dates_input = st.text_input("Start dates", "20240101, 20250101, 20260101")
    tenor = st.text_input("Tenor", "1Y")
    roll_convention = st.text_input("Roll convention", "")
with date_right:
    business_day_adj = st.selectbox(
        "Business-day adjustment",
        ["FOLLOWING", "MODIFIED_FOLLOWING", "PRECEDING", "MODIFIED_PRECEDING", "NONE"],
    )
    calendar = st.text_input("Calendar(s)", "LNB+NYB")

if st.button("Calculate End Dates", key="date_button"):
    try:
        start_dates = [value.strip() for value in start_dates_input.split(",") if value.strip()]
        result = aq.aqDateFromTenor(start_dates, tenor, business_day_adj, calendar, roll_convention)
        st.success("Date calculation completed")
        st.dataframe(pd.DataFrame({"Start Date": start_dates, "End Date": result}), use_container_width=True, hide_index=True)
    except Exception as exc:
        st.error(f"Date calculation failed: {exc}")

st.divider()
st.subheader("Black-Scholes Pricing & Risk")
st.caption("European option pricing and vega calculated directly by the native AlgoQuantLib C++ analytics engine.")

input_panel, result_panel = st.columns([1, 1.7], gap="large")
with input_panel:
    st.markdown("#### Option Parameters")
    call_or_put = st.selectbox("Option type", ["CALL", "PUT"])
    spot = st.number_input("Spot", min_value=0.01, value=100.0, step=1.0)
    strike = st.number_input("Strike", min_value=0.01, value=100.0, step=1.0)
    bs_vol_pct = st.number_input("Volatility (%)", min_value=0.01, value=10.0, step=0.5)
    time = st.number_input("Time to expiry (years)", min_value=0.001, value=1.0, step=0.25)
    bs_rate_pct = st.number_input("Interest rate (%)", value=4.0, step=0.25)
    bs_carry_pct = st.number_input("Carry (%)", value=0.0, step=0.25)
    shift = st.number_input("Shift", value=0.0, step=0.01)
    calculate_bs = st.button("Calculate Price & Vega", use_container_width=True)

if calculate_bs:
    try:
        vol = float(bs_vol_pct) / 100.0
        rate = float(bs_rate_pct) / 100.0
        carry = float(bs_carry_pct) / 100.0
        price = float(aq.aqMathBlackScholesPrice(call_or_put, float(spot), float(strike), vol, float(time), rate, carry, float(shift)))
        vega = float(aq.aqMathBlackScholesVega(call_or_put, float(spot), float(strike), vol, float(time), rate, carry, float(shift)))
        st.session_state["bs_result"] = {"price": price, "vega": vega}
        st.session_state["bs_inputs"] = {"call_or_put": call_or_put, "spot": float(spot), "strike": float(strike), "vol": vol, "time": float(time), "rate": rate, "carry": carry, "shift": float(shift)}
    except Exception as exc:
        st.error(f"Black-Scholes calculation failed: {exc}")

with result_panel:
    if "bs_result" in st.session_state:
        result = st.session_state["bs_result"]
        inputs = st.session_state["bs_inputs"]
        intrinsic = max(inputs["spot"] - inputs["strike"], 0.0) if inputs["call_or_put"] == "CALL" else max(inputs["strike"] - inputs["spot"], 0.0)
        time_value = result["price"] - intrinsic
        st.markdown("#### Results")
        r1, r2 = st.columns(2)
        with r1:
            st.metric(f"{inputs['call_or_put']} Price", f"{result['price']:.4f}")
            st.metric("Intrinsic Value", f"{intrinsic:.4f}")
        with r2:
            st.metric("Vega", f"{result['vega']:.4f}")
            st.metric("Time Value", f"{time_value:.4f}")
    else:
        st.info("Enter the option parameters and calculate the C++ price and vega.")

if "bs_result" in st.session_state:
    inputs = st.session_state["bs_inputs"]
    spot_range = np.linspace(max(1.0, inputs["strike"] * 0.5), inputs["strike"] * 1.5, 100)
    prices, vegas = [], []
    for current_spot in spot_range:
        try:
            prices.append(float(aq.aqMathBlackScholesPrice(inputs["call_or_put"], float(current_spot), inputs["strike"], inputs["vol"], inputs["time"], inputs["rate"], inputs["carry"], inputs["shift"])))
            vegas.append(float(aq.aqMathBlackScholesVega(inputs["call_or_put"], float(current_spot), inputs["strike"], inputs["vol"], inputs["time"], inputs["rate"], inputs["carry"], inputs["shift"])))
        except Exception:
            prices.append(np.nan)
            vegas.append(np.nan)
    sensitivity = pd.DataFrame({"Spot": spot_range, "Option Price": prices, "Vega": vegas})
    st.subheader("Price Sensitivity")
    st.line_chart(sensitivity, x="Spot", y="Option Price", height=500, use_container_width=True)
    st.subheader("Vega Sensitivity")
    st.line_chart(sensitivity, x="Spot", y="Vega", height=360, use_container_width=True)
    st.caption(f"Strike {inputs['strike']:.2f} · Volatility {inputs['vol'] * 100:.2f}% · Rate {inputs['rate'] * 100:.2f}% · Carry {inputs['carry'] * 100:.2f}% · Expiry {inputs['time']:.2f} years")

st.divider()
st.caption("AlgoQuantLib · Native C++ Quantitative Analytics · Pricing · Risk · eTrading")
