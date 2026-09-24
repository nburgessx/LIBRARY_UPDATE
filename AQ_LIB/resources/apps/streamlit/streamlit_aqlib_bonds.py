import streamlit as st

st.set_page_config(page_title="AlgoQuantLib Bonds", layout="centered")

st.title("AlgoQuantLib — UST Bond Pricing")
st.caption("Native C++ analytics via Python API")

# 1. Import AlgoQuantLib
try:
    import AlgoQuantLib as aq
    aq.aqToolVersion()
    st.success("Success: Connected to AlgoQuantLib")
except Exception as exc:
    st.error(f"Failed to connect to AlgoQuantLib: {exc}")
    st.stop()

# 2. Initialize - Load Trade Templates & Holiday Calendars
try:
    aq.aqToolInitialize()
    bond_generators = aq.aqGeneratorList("BOND_GENERATOR")
    st.success("Success: Loaded trade templates & holiday calendars")
except Exception as exc:
    st.error(f"Failed to load trade templates & holiday calendars: {exc}")
    st.stop()

# 3. Bond setup
st.subheader("Bond Definition")

# Bond name (as shown in LVB)
bond_name_default = "UST-4.625-AUG36"
bond_name = st.text_input("Bond Name", value=bond_name_default)

# Bond generator selection
generator_default = "US_TREASURY_TYPE1"
# Ensure default is in list; if not, fall back to first available
if generator_default not in bond_generators:
    generator_default = bond_generators[0] if bond_generators else ""
bond_generator = st.selectbox("Bond Generator", bond_generators, index=bond_generators.index(generator_default) if generator_default in bond_generators else 0)

# Bond expression LVB inputs (keys + default values)
keys = [
    "BondDescription",
    "ISIN",
    "IssueDate",
    "AccrualStartDate",
    "RollDay",
    "FirstCouponDate",
    "LastCouponDate",
    "Coupon",
    "MaturityDate",
]

values_default = [
    "UST-4.625-AUG36",       # BondDescription
    "CT10",                  # ISIN
    "20250815",              # IssueDate (YYYYMMDD)
    "20250815",              # AccrualStartDate (YYYYMMDD)
    "",                      # RollDay (blank)
    "20260215",              # FirstCouponDate (YYYYMMDD)
    "20360215",              # LastCouponDate (YYYYMMDD)
    "4.6250",                # Coupon (%)
    "20360815",              # MaturityDate (YYYYMMDD)
]

st.caption("Edit any field below; dates must be in YYYYMMDD format.")

roll_day_options = ["", "IMM", "EOM"] + [str(i) for i in range(1, 32)]

values = []
for k, v_default in zip(keys, values_default):
    if k == "RollDay":
        # Ensure default is in options
        idx = roll_day_options.index(v_default) if v_default in roll_day_options else 0
        val = st.selectbox(k, options=roll_day_options, index=idx)
    else:
        val = st.text_input(k, value=v_default)

    if k == "Coupon" and val:
        # Convert coupon from percent to decimal if your library expects that
        val = str(float(val) / 100.0)

    values.append(val)

validateKeys = True 

# 4. Create bond object
st.subheader("Create Bond Object")

if st.button("Create Bond Object"):
    try:
        # Create bond expression LVB
        bond_LVB = aq.aqToolLVBFromKeysValues(keys, values, "")
        
        # Create bond object from generator + LVB
        bond_handle = aq.aqBondObjectCreateFromGenerator(
            bond_name,
            bond_generator,
            bond_LVB,
            validateKeys
        )
        
        st.session_state["bond_handle"] = bond_handle
        st.success(f"Success: Bond object created. Handle = {bond_handle}")
        
        st.write("Bond LVB:", bond_LVB)
        bond_display = aq.aqBondObjectDisplay(bond_handle)
        st.write("Bond Object", bond_display)
        
    except Exception as exc:
        st.error(f"Failed to create bond object: {exc}")
        st.session_state.pop("bond_handle", None)

# 5. Price bond
st.subheader("Price Bond")

bond_handle = st.session_state.get("bond_handle", "")
if not bond_handle:
    st.info("Create a bond object first to enable pricing.")
    st.stop()

st.text_input("Bond Handle (read-only)", value=bond_handle, disabled=True)

# Need to pass a vector tuple of settlement dates to the bond pricing function
settlement_date = st.text_input("Settlement Date (YYYYMMDD)", value="20260918")
settlement_dates = (settlement_date,) # <-- comma makes it a tuple

# Need to pass a vector tuple of yield to maturities to the bond pricing function
ytm = st.number_input("Yield (%)", value=4.9538543, step=0.00001, format="%.7f")
yields = (ytm / 100.0,) # <-- comma makes it a tuple

if st.button("Price Bond"):
    try:
        price = aq.aqBondObjectPrice(
            bond_handle,
            settlement_dates,
            yields,
            "" # yield calculation type: Simple, True, ISMA or blank
        )
        
        # price returns a tuple so extract the first element
        price = price[0]
        
        st.metric("Bond Price", f"{price:.6f}")
        st.success("Success: Bond price from AlgoQuantLib")
    except Exception as exc:
        st.error(f"Failed to price bond: {exc}")