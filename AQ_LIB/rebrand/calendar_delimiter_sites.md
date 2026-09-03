# 0.8 — calendar holiday-centre delimiter (`:` → `+`)

Decision D12: change the holiday-centre join delimiter to `+`, accept `:` too
during transition unless it collides. **It collides — read on.**

## The token today

`:` combines holiday centres: `"SYB:LNB"` = Sydney + London business days.

## Where it is parsed / built (the sites to route through one helper)

| Site | What |
|---|---|
| `math\src\LAPriceDataCalendar.cpp` `convertFromString()` (~L488) | canonical parse — splits the calendar string on `DATA_COLL_DEL` and inserts each centre into `mCalendarSet` |
| `math\include\LAPriceDataCalendar.h` (city-vector ctor path, ~L60–85) | consumes an already-split `city` vector |
| `etrading\src\LACurveCalibrationHelpers.cpp` | **8 sites** — `chgrow(..., CURVEINPUT_CALENDAR / CURVEINPUT_BASISCALENDAR, 1).toToken(':')` → `city` |
| `models\src\LAMathCurveGenerateFuncUtility.cpp` | **8 sites** — identical (this file is a near-duplicate of the curve-calibration helper; both feed curve building) |
| `models\src\LAMathDateCalculations.cpp` (~L1108–1122) | "remove NYB from the calendar list" — splits on `:` **and rebuilds the string with `:`** |
| `CURVEINPUT_CALENDAR` = `"CALENDAR"`, `CURVEINPUT_BASISCALENDAR` = `"BASISCALENDAR"` (+`1`,`2`) | `models\include\ConstantDeclarations.h` |

## The collision — `DATA_COLL_DEL` is also `':'`

`#define DATA_COLL_DEL ':'` (`math\include\LADataVector.h:25`). It is the generic
list separator used across `LADataVector` / `LADataMatrix` / config parsing — and
`LAPriceDataCalendar::convertFromString` uses it for the calendar split.

Worse, the shipped generator JSON uses `:` for **both** calendars **and**
curve-name lists, side by side:

```
"DISCOUNTCURVE": "OIS:AUDOIS:AUDDF:AUDDF_AUDCSA:AUDDF_NOCSA"   <- curve list, stays ':'
"ACCRUALCALENDAR": "SYB:LNB"                                    <- calendar list, -> '+'
```

**103 generator JSON files** under `resources\config` contain `"XX:YY"` calendar
values.

So you **cannot** globally swap `:`→`+`, and you cannot redefine `DATA_COLL_DEL`.
Only the *calendar-field* parse/build changes.

## Recommendation

1. Add `constexpr char CALENDAR_CENTRE_DELIMITER = '+';` and a single helper
   `splitCalendarCentres(const LAString&)` that splits on `+` **and** `:`
   (transitional accept-both — safe here because centre codes are alphanumeric,
   never contain `:`).
2. Route through it: `LAPriceDataCalendar::convertFromString`, the NYB-strip in
   `LAMathDateCalculations` (rebuild with `+`), and the 16 curve-helper sites.
   Leave every other `toToken(':')` alone.
3. Migrate the **calendar fields only** in the 103 generator JSON to `+`
   (`resources\config` audit, Phase 4b.4 / 6.4). Curve-name-list fields keep `:`.
4. `GOOGLE_TEST`: combined-centre calendar built from `"SYB+LNB"` **and**
   `"SYB:LNB"` give identical holiday sets.

**Needs Nicholas's call:** accept-both indefinitely (a), or a dated cutover to
`+`-only after one release (c)? Recommendation: (a) for one release, then (c).
