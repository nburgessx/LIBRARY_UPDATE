using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;

namespace MLIB_API_CSHARP_TEST
{
    public class CurveBuildProperties 
    {
        public static String AS_OF_DATE = "AsOfDate";
        public static String CURRENCY = "Currency";
        public static String FUTURE_INTERPOLATION = "future.interpolation";
        public static String INTERPOLATION = "interpolation";
        public static String YIELDGEN_INTERPOLATION = "yieldgen.interpolation";
        public static String BASIS_FUNCTION = "basisfunction";
        public static String BASIS_INTERPOLATION = "basis.interpolation";
        public static String BASIS_INTERPOLATION_3M6MBASIS = "basis.interpolation.3m6mbasis";
        public static String BASIS_INTERPOLATION_1M3MBASIS = "basis.interpolation.1m3mbasis";
        public static String BASIS_INTERPOLATION_6M12MBASIS = "basis.interpolation.6m12mbasis";
        public static String BASIS_INTERPOLATION_XCCYBASIS = "basis.interpolation.xccybasis";
        public static String IS_FRA_USE = "isfrause";
        public static String IS_FUTURE_USE = "isfutureuse";
        public static String FREQUENCY = "Frequency";
        public static String DF_CURVE_NAME = "dfCurveName";
        public static String DAY_COUNT = "daycount";
        public static String SLIDING_RULE = "slidingrule";
        public static String IS_SPOT_USE = "isspotuse";
        public static String IS_SWAP_TENOR_ADJUST = "isswaptenoradjust";
        public static String IS_ADJUST_DF = "isadjustdf";
        public static String IS_RENOTIONAL_ADJUST = "isRenotionalAdjust";

        private static Dictionary<String, String> properties;

        private CurveBuildProperties(Dictionary<String, String> rhs)
        {
            properties = rhs;
        }

        public VecVecString toMlib()
        {
            VecVecString ret = new VecVecString();
            foreach (KeyValuePair<string, string> entry in properties)
            {
                VecString vecString = new VecString();
                vecString.Add(entry.Key);
                vecString.Add(entry.Value);

                ret.Add(vecString);
            }

            return ret;
        }

        public class Builder 
        {
            private  static Dictionary<String, String> builderProperties = new Dictionary<String, String>();

            public CurveBuildProperties build()
            {
                return new CurveBuildProperties(builderProperties);
            }

            public Builder asOfDate(int date) 
            {
                builderProperties.Add(AS_OF_DATE, Convert.ToString(date));

                return this;
            }

            public Builder currency(String input) 
            {
                Currency ccy = new Currency(input);
                builderProperties.Add(CURRENCY, ccy.name());
                return this;
            }

            public Builder futureInterpolation(Interpolation interpolation) 
            {
                return AddInterpolation(FUTURE_INTERPOLATION, interpolation);
            }

            public Builder interpolation(Interpolation interpolation) 
            {
                return AddInterpolation(INTERPOLATION, interpolation);
            }

            public Builder yieldgenInterpolation(Interpolation interpolation) 
            {
                return AddInterpolation(YIELDGEN_INTERPOLATION, interpolation);
            }

            public Builder basisInterpolation(Interpolation interpolation) 
            {
                return AddInterpolation(BASIS_INTERPOLATION, interpolation);
            }

            public Builder basisInterpolation3m6mBasis(Interpolation interpolation) 
            {
                return AddInterpolation(BASIS_INTERPOLATION_3M6MBASIS, interpolation);
            }

            public Builder basisInterpolation1m3mBasis(Interpolation interpolation) 
            {
                return AddInterpolation(BASIS_INTERPOLATION_1M3MBASIS, interpolation);
            }

            public Builder basisInterpolation6m12mBasis(Interpolation interpolation) 
            {
                return AddInterpolation(BASIS_INTERPOLATION_6M12MBASIS, interpolation);
            }

            public Builder isFraUse(bool isFraUse) 
            {
                builderProperties.Add(IS_FRA_USE, Convert.ToString(isFraUse));
                    
                return this;
            }

            public Builder isFutureUse(bool isfutureuse) 
            {
                builderProperties.Add(IS_FUTURE_USE, Convert.ToString(isfutureuse));

                return this;
            }

            public Builder dfCurveName(String dfCurveName) 
            {
                builderProperties.Add(DF_CURVE_NAME, dfCurveName);

                return this;
            }

            public Builder frequency(Frequency frequency) 
            {
                builderProperties.Add(FREQUENCY, frequency.getMlibArg());

                return this;
            }
            
            private Builder AddInterpolation(String interpolationKey, Interpolation interpolation) 
            {
                builderProperties.Add(interpolationKey, interpolation.asMlibArg());

                return this;
            }

            private Builder dayCount(DayCount dayCount) 
            {
                builderProperties.Add(DAY_COUNT, dayCount.getMlibArg());

                return this;
            }
        }
        
    }
}
