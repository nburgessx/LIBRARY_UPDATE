using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;

namespace MLIB_API_CSHARP_TEST
{
    public class OisCurveConfiguration
    {
        public String curveId = "EURYC";

        public String marketName = "OIS";

        //-------------------------------------------------------------------------
        // Important:
        //
        // All the dates must be in "YYYYMMDD" format if they are direct inputs
        // to MLIB API methods.
        //
        // All the dates must be in the Excel serial date format if they are NOT
        // direct inputs to MLIB API methods, such as all the dates used below.
        //
        // In both cases, the dates must be passed on to MLIB as strings.
        //-------------------------------------------------------------------------

        public CurveBuildProperties curveBuildProperties = new CurveBuildProperties.Builder()
                .asOfDate(toSerialDate("20160219"))
                .currency(Currency.EUR)
                .yieldgenInterpolation(new Interpolation(Interpolation.MONOTONE_CONVEX))
                .build();

        public InstrumentConventions oisConv = new InstrumentConventions.Builder()
                .calendar(new Calendar(Calendar.Tgt))
                .daycount(new DayCount(DayCount.ACT_360))
                .frequency(new Frequency(Frequency.ANNUAL))
                .resetLag("2d")
                .slidingrule(new SlidingRule(SlidingRule.MOD_FOLLOWING))
                .isEomRoll(false)
                .eomDay(31)
                .generateMethod(new GenerateMethod(GenerateMethod.DAILY_COMPOUNDING))
                .firstRate(new FirstRate(FirstRate.SHORT_MARKET))
                .shortTermConvention(new ShortTermConvention(ShortTermConvention.ARITHMETIC_AVERAGE))
                .longTermConvention(new LongTermConvention(LongTermConvention.LOBASIS))
                .longTerm("3Y")
                .longTermGenerateMethod(new LongTermGenerateMethod(LongTermGenerateMethod.DAILY_AVERAGING))
                .epsilon(0.0000000001)
                .maxLoop(1000)
                .build();

        public InstrumentRates oisRates = new InstrumentRates.Builder()
                .addTermRateStartDateEndDateUse("ON", -0.00243, 0, 0, true)
                .addTermRateStartDateEndDateUse("EUSF1A", -0.00339, toSerialDate("20160421"), toSerialDate("20160602"), true)
                .addTermRateStartDateEndDateUse("EUSF2A", -0.00353, toSerialDate("20160602"), toSerialDate("20160721"), true)
                .addTermRateStartDateEndDateUse("EUSF3A", -0.00357, toSerialDate("20160721"), toSerialDate("20160908"), true)
                .addTermRateStartDateEndDateUse("EUSF4A", -0.00378, toSerialDate("20160908"), toSerialDate("20161020"), true)
                .addTermRateStartDateEndDateUse("EUSF5A", -0.00386, toSerialDate("20161020"), toSerialDate("20161208"), true)
                .addTermRateStartDateEndDateUse("12M", -0.00369, 0, 0, true)
                .addTermRateStartDateEndDateUse("18M", -0.00379, 0, 0, true)
                .addTermRateStartDateEndDateUse("2Y", -0.00377, 0, 0, true)
                .addTermRateStartDateEndDateUse("3Y", -0.0034, 0, 0, true)
                .addTermRateStartDateEndDateUse("4Y", -0.00269, 0, 0, true)
                .addTermRateStartDateEndDateUse("5Y", -0.00175, 0, 0, true)
                .addTermRateStartDateEndDateUse("6Y", -0.00064, 0, 0, true)
                .addTermRateStartDateEndDateUse("7Y", 0.00057, 0, 0, true)
                .addTermRateStartDateEndDateUse("8Y", 0.00183, 0, 0, true)
                .addTermRateStartDateEndDateUse("9Y", 0.00306, 0, 0, true)
                .addTermRateStartDateEndDateUse("10Y", 0.00419, 0, 0, true)
                .addTermRateStartDateEndDateUse("11Y", 0.00521, 0, 0, true)
                .addTermRateStartDateEndDateUse("12Y", 0.00609, 0, 0, true)
                .addTermRateStartDateEndDateUse("15Y", 0.0081, 0, 0, true)
                .addTermRateStartDateEndDateUse("20Y", 0.00975, 0, 0, true)
                .addTermRateStartDateEndDateUse("25Y", 0.01033, 0, 0, true)
                .addTermRateStartDateEndDateUse("30Y", 0.01053, 0, 0, true)
                .addTermRateStartDateEndDateUse("40Y", 0.01061, 0, 0, true)
                .addTermRateStartDateEndDateUse("50Y", 0.0101, 0, 0, true)
                .build();

        public CurveIndexNames curveIndexNames = new CurveIndexNames.Builder()
                .addName("OIS")
                .addName("EUROIS")
                .addName("EURDF")
                .addName("EURDF_NOCSA")
                .build();

        public InstrumentRates oisHistRates = new InstrumentRates.Builder()
                .build(); // Empty

        public InstrumentConventions loBasisConv = new InstrumentConventions.Builder()
                .calendar(new Calendar(Calendar.Tgt))
                .daycount(new DayCount(DayCount.ACT_360))
                .frequency(new Frequency(Frequency.QUARTERLY))
                .slidingrule(new SlidingRule(SlidingRule.MOD_FOLLOWING))
                .build();

        public InstrumentRates loBasisRates = new InstrumentRates.Builder()
                .addTermRate("2Y", 0.00126)
                .addTermRate("3Y", 0.0013)
                .addTermRate("4Y", 0.00132)
                .addTermRate("5Y", 0.00134)
                .addTermRate("6Y", 0.00133)
                .addTermRate("7Y", 0.00132)
                .addTermRate("8Y", 0.00131)
                .addTermRate("9Y", 0.0013)
                .addTermRate("10Y", 0.00128)
                .addTermRate("11Y", 0.00127)
                .addTermRate("12Y", 0.00126)
                .addTermRate("15Y", 0.00119)
                .addTermRate("20Y", 0.00104)
                .addTermRate("25Y", 0.00092)
                .addTermRate("30Y", 0.00082)
                .addTermRate("40Y", 0.00066)
                .addTermRate("50Y", 0.00057)
                .build();

        public InstrumentConventions swapConv = new InstrumentConventions.Builder()
                .calendar(new Calendar(Calendar.Tgt))
                .dayCountFix(new DayCount(DayCount.x30_360))
                .frequencyFix(new Frequency(Frequency.ANNUAL))
                .slidingrule(new SlidingRule(SlidingRule.MOD_FOLLOWING))
                .build();

        public InstrumentRates swapRates = new InstrumentRates.Builder()
                .addTermRate("2Y", -0.00254)
                .addTermRate("3Y", -0.00214)
                .addTermRate("4Y", -0.0014)
                .addTermRate("5Y", -0.00042)
                .addTermRate("6Y", 0.0007)
                .addTermRate("7Y", 0.00192)
                .addTermRate("8Y", 0.00319)
                .addTermRate("9Y", 0.00443)
                .addTermRate("10Y", 0.00555)
                .addTermRate("11Y", 0.00657)
                .addTermRate("12Y", 0.00746)
                .addTermRate("15Y", 0.00942)
                .addTermRate("20Y", 0.01095)
                .addTermRate("25Y", 0.01142)
                .addTermRate("30Y", 0.01152)
                .addTermRate("40Y", 0.01144)
                .addTermRate("50Y", 0.01083)
                .build();

        private static int toSerialDate(string dateStr)
        {
            var date = DateTime.ParseExact(dateStr,
                                    "yyyyMMdd",
                                    System.Globalization.CultureInfo.InvariantCulture);
            return (int)(date.ToOADate());
        }
    }    
}
