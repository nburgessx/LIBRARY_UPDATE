import sys
from pathlib import Path

import streamlit as st

## Application Title
st.set_page_config(page_title="AlgoQuantLib C++ Analytics", layout="centered")
st.title("AlgoQuantLib C++ Analytics")
st.markdown("Live C++ Analytics for eTrading and Financial Markets")

## Set File Path
## sys.path.insert(0, str(Path(__file__).parent / "AQ_Python"))
sys.path.insert(0, str(Path(__file__).parent))

## 1. Import the Python Library
st.subheader("AlgoQuantLib Connectivity Check")
st.markdown("Confirms the Python bindings can load `AlgoQuantLib` and call into the C++ library.")

try:
    import AlgoQuantLib as aql
    st.success("`import AlgoQuantLib` succeeded")
except ImportError as e:
    st.error(f"Failed to import AlgoQuantLib: {e}")
    st.stop()

## 2. Call aqToolVersion
if st.button("Call aqToolVersion()"):
    try:
        version = aql.aqToolVersion()
        st.success("Call succeeded")
        st.metric("AlgoQuantLib Version", version)
    except Exception as e:
        st.error(f"Call failed: {e}")

st.divider()
st.subheader("Load Holiday Calendars")
st.markdown("Imports the holiday calendars from 'SwapsMonitor'.")
calendar_filepath = st.text_input("Calendar File Path", r".\config\Calendar.csv")

## 2. Call aqToolLoadCalendarFile
if st.button("Call aqToolLoadCalendarFile()"):
    try:
        result = aql.aqToolLoadCalendarFile(calendar_filepath)
        st.success("Calendar file loaded successfully")
        st.write(f"**Calendar Filepath:** {calendar_filepath}")
    except Exception as e:
        st.error(f"Call failed: {e}")

st.divider()
st.subheader("Call aqDateFromTenor")
st.markdown("Adjust date(s) by applying a tenor adjustment whilst repsecting holidays and business day conventions")

## 3. AqDateFromTenor: Create Input Columns
col1, col2 = st.columns(2)
with col1:
    start_dates_input = st.text_input("Start Dates (comma-separated)", "20240101, 20250101,20260101")
    tenor = st.text_input("Tenor", "1Y")
with col2:
    business_day_adj = st.text_input("Business Day Adjustment", "FOLLOWING")
    calendar = st.text_input("SwapsMonitor Calendar(s)", "LNB+NYB")

roll_convention = st.text_input("Roll Convention", "")

## 4. Call AqDateFromTenor
if st.button("Call aqDateFromTenor()"):
    try:
        start_dates = [d.strip() for d in start_dates_input.split(",")]
        result = aql.aqDateFromTenor(start_dates, tenor, business_day_adj, calendar, roll_convention)
        st.success("Call succeeded")
        st.write("**Result (End Dates):**")
        for i, date in enumerate(result):
            st.write(f"  {i+1}. {date}")
    except Exception as e:
        st.error(f"Call failed: {e}")

st.divider()
st.subheader("Call aqMathBlackScholesPrice")
st.markdown("Black-Scholes european option price")

## 5. AqMathBlackScholesPrice: Create Input Column
col1, col2 = st.columns(2)
with col1:
    callPut = st.text_input("Call/Put", "CALL")
    spot = st.text_input("Spot", "100")
    strike = st.text_input("Strike", "100")
    vol = st.text_input("Vol(%)", "10")
    time = st.text_input("Time", "1.0")
    rate = st.text_input("Rate(%)", "4.0")

## 6. Call AqMathBlackScholesPrice
if st.button("Call aqMathBlackScholesPrice()"):
    try:
        volpct = float(vol)/100.0
        ratepct = float(rate)/100.0
        carry = 0.0
        shift = 0.0
        price = aql.aqMathBlackScholesPrice(callPut, float(spot), float(strike), volpct, float(time), ratepct, carry, shift)
        st.success(f"Option price {price:.4f}")
    except Exception as e:
        st.error(f"Call failed: {e}")
