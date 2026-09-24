import streamlit as st

st.set_page_config(page_title="AlgoQuantLib MVP", layout="centered")

st.title("AlgoQuantLib MVP")
st.caption("Native C++ analytics via Python api.")

# 1. Import AlgoQuantLib
try:
    import AlgoQuantLib as aq
    aq.aqToolVersion()
    st.success(f"Success: Connected to AlgoQuantLib")
except Exception as exc:
    st.error(f"Failed to connect to AlgoQuantLib: {exc}")

# 2. Initialize - Load Trade Templates & Holiday Calendars
try:
    aq.aqToolInitialize()
    date = aq.aqDateFromTenor("20260101", "1Y", "F", "LNB")
    bondGeneratorList = aq.aqGeneratorList("BOND_GENERATOR")
    st.success(f"Success: Loaded trade templates & holiday calendars")
except Exception as exc:
    st.error(f"Failed to trade templates & holiday calendars: {exc}")

# 3. MVP: Call 'aqMathBlackScholesPrice()' and price european option
st.subheader("European Option Price")
option_type = st.selectbox("Option type", ["Call", "Put"])
spot = st.number_input("Spot (S)", value=100.0, step=1.0)
strike = st.number_input("Strike (K)", value=100.0, step=1.0)
vol = st.number_input("Volatility (%)", value=10.0, step=1.0) / 100.0
time = st.number_input("Time to expiry (years)", value=1.0, step=0.1, min_value=0.0)
rate = st.number_input("Risk-free rate (%)", value=4.0, step=0.25) / 100.0
carry = st.number_input("Carry(%)", value=0.0, step=0.25) / 100.0

if st.button("Price"):
    price = aq.aqMathBlackScholesPrice(
        option_type,
        spot,
        strike,
        vol,
        time,
        rate,
        carry
    )
    st.metric("Option Price", f"{price:.6f}")
    st.success("Success: Option price from AlgoQuantLib")