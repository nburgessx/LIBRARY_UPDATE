import sys
from pathlib import Path
import numpy as np
import pandas as pd
import streamlit as st

st.set_page_config(page_title="AlgoQuantLib | Quant Analytics", page_icon="📈", layout="wide", initial_sidebar_state="expanded")
st.markdown("""<style>
.stApp{background:#f5f7fa}.main .block-container{max-width:1120px;padding-top:1.5rem;padding-bottom:2rem}
[data-testid="stSidebar"]{background:#111827}[data-testid="stSidebar"] *{color:#e5e7eb}
[data-testid="stMetric"]{background:#fff;border:1px solid #e2e8f0;border-radius:8px;padding:.55rem}
</style>""", unsafe_allow_html=True)
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
    st.write("Black-Scholes pricing and Greeks")
    st.subheader("Engine")
    st.write(f"AlgoQuantLib version: {aq_version}")
    st.write("Python → native C++")
    st.write("Pricing and risk calculated by native engine")
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
    business_day_adj = st.selectbox("Business-day adjustment", ["FOLLOWING", "MODIFIED_FOLLOWING", "PRECEDING", "MODIFIED_PRECEDING", "NONE"])
    calendar = st.text_input("Calendar(s)", "LNB+NYB")
if st.button("Calculate End Dates", key="date_button"):
    try:
        start_dates = [x.strip() for x in start_dates_input.split(",") if x.strip()]
        result = aq.aqDateFromTenor(start_dates, tenor, business_day_adj, calendar, roll_convention)
        st.success("Date calculation completed")
        st.dataframe(pd.DataFrame({"Start Date": start_dates, "End Date": result}), use_container_width=True, hide_index=True)
    except Exception as exc:
        st.error(f"Date calculation failed: {exc}")

st.divider()
st.subheader("Black-Scholes Pricing & Risk")
st.caption("Price and first- and second-order sensitivities calculated by the native AlgoQuantLib C++ engine.")
input_panel, result_panel = st.columns([1, 2], gap="large")
with input_panel:
    st.markdown("#### Option Parameters")
    call_or_put = st.selectbox("Option type", ["CALL", "PUT"])
    spot = st.number_input("Spot", min_value=0.01, value=100.0, step=1.0)
    strike = st.number_input("Strike", min_value=0.01, value=100.0, step=1.0)
    vol_pct = st.number_input("Volatility (%)", min_value=0.01, value=10.0, step=0.5)
    time = st.number_input("Time to expiry (years)", min_value=0.001, value=1.0, step=0.25)
    rate_pct = st.number_input("Interest rate (%)", value=4.0, step=0.25)
    carry_pct = st.number_input("Carry (%)", value=0.0, step=0.25)
    shift = st.number_input("Shift", value=0.0, step=0.01)
    calculate = st.button("Calculate Price & Greeks", use_container_width=True)

if calculate:
    try:
        vol, rate, carry = vol_pct / 100.0, rate_pct / 100.0, carry_pct / 100.0
        args = (call_or_put, float(spot), float(strike), float(vol), float(time), float(rate), float(carry), float(shift))
        st.session_state.bs = {
            "call_or_put": call_or_put, "spot": float(spot), "strike": float(strike), "vol": vol,
            "time": float(time), "rate": rate, "carry": carry, "shift": float(shift),
            "price": float(aq.aqMathBlackScholesPrice(*args)),
            "delta": float(aq.aqMathBlackScholesDeltaSpot(*args)),
            "gamma": float(aq.aqMathBlackScholesGamma(*args)),
            "vega": float(aq.aqMathBlackScholesVega(*args)),
            "theta": float(aq.aqMathBlackScholesTheta(*args)),
            "rho": float(aq.aqMathBlackScholesRho(*args)),
        }
    except Exception as exc:
        st.error(f"Black-Scholes calculation failed: {exc}")

with result_panel:
    if "bs" in st.session_state:
        x = st.session_state.bs
        intrinsic = max(x["spot"] - x["strike"], 0.0) if x["call_or_put"] == "CALL" else max(x["strike"] - x["spot"], 0.0)
        st.markdown("#### Option Results")
        price_col, greek_col = st.columns(2, gap="medium")
        with price_col:
            st.metric(f"{x['call_or_put']} Price", f"{x['price']:.4f}")
            st.metric("Intrinsic Value", f"{intrinsic:.4f}")
            st.metric("Time Value", f"{x['price'] - intrinsic:.4f}")
        with greek_col:
            st.metric("Delta", f"{x['delta']:.6f}")
            st.metric("Gamma", f"{x['gamma']:.6f}")
            st.metric("Vega", f"{x['vega']:.6f}")
            st.metric("Theta", f"{x['theta']:.6f}")
            st.metric("Rho", f"{x['rho']:.6f}")
    else:
        st.info("Enter the option parameters and calculate the native C++ results.")

if "bs" in st.session_state:
    x = st.session_state.bs
    spots = np.linspace(max(1.0, x["strike"] * 0.5), x["strike"] * 1.5, 100)
    rows = []
    for current_spot in spots:
        args = (x["call_or_put"], float(current_spot), x["strike"], x["vol"], x["time"], x["rate"], x["carry"], x["shift"])
        try:
            rows.append({
                "Spot": current_spot,
                "Option Price": float(aq.aqMathBlackScholesPrice(*args)),
                "Delta": float(aq.aqMathBlackScholesDeltaSpot(*args)),
                "Gamma": float(aq.aqMathBlackScholesGamma(*args)),
                "Vega": float(aq.aqMathBlackScholesVega(*args)),
                "Theta": float(aq.aqMathBlackScholesTheta(*args)),
                "Rho": float(aq.aqMathBlackScholesRho(*args)),
            })
        except Exception:
            rows.append({"Spot": current_spot, "Option Price": np.nan, "Delta": np.nan, "Gamma": np.nan, "Vega": np.nan, "Theta": np.nan, "Rho": np.nan})
    sensitivity = pd.DataFrame(rows)
    st.subheader("Spot Sensitivities")
    chart_rows = [
        [("Option Price", "Price vs Spot"), ("Delta", "Delta vs Spot")],
        [("Gamma", "Gamma vs Spot"), ("Vega", "Vega vs Spot")],
        [("Theta", "Theta vs Spot"), ("Rho", "Rho vs Spot")],
    ]
    for chart_row in chart_rows:
        left, right = st.columns(2, gap="medium")
        for column, (series, title) in zip((left, right), chart_row):
            with column:
                st.markdown(f"#### {title}")
                st.line_chart(sensitivity, x="Spot", y=series, height=250, use_container_width=True)
    st.caption(f"Strike {x['strike']:.2f} · Volatility {x['vol'] * 100:.2f}% · Rate {x['rate'] * 100:.2f}% · Carry {x['carry'] * 100:.2f}% · Expiry {x['time']:.2f} years")

st.divider()
st.caption("AlgoQuantLib · Native C++ Quantitative Analytics · Pricing · Risk · eTrading")
