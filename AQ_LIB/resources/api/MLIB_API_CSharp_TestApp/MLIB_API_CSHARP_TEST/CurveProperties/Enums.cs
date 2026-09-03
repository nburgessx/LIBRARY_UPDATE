using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;

namespace MLIB_API_CSHARP_TEST
{
    // Interpolation
    public class Interpolation
    {
        public static string LINEAR = "fn_linearinterpolation";
        public static string STEP = "fn_StepInterpolation";
        public static string MONOTONE_CONVEX = "fn_MonotoneConvexInterpolation";
        public static string SPLINE = "fn_SplineInterpolation";
        public static string CONSTRAINED_SPLINE = "fn_ConstrainedSplineInterpolation";

        private static string mlibArg;

        public Interpolation(String rhs)
        {
            mlibArg = rhs;
        }

        public String asMlibArg()
        {
            return mlibArg;
        }
    }

    // Frequency
    public class Frequency
    {
        public static string SIMPLE = "SIMPLE";
        public static string CONTINUOUS = "CONTINUOUS";
        public static string ANNUAL = "ANNUAL";
        public static string SEMI_ANNUAL = "SEMI-ANNUAL";
        public static string QUARTERLY = "QUARTERLY";
        public static string MONTHLY = "MONTHLY";
        public static string LUNAR = "LUNAR";

        private static string mlibArg;

        public Frequency(String rhs)
        {
            mlibArg = rhs;
        }

        public String getMlibArg()
        {
            return mlibArg;
        }
    }

    // DayCount
    public class DayCount
    {
        public static string ACT_ACT = "ACT/ACT";
        public static string ACT_365 = "ACT/365";
        public static string ACT_360 = "ACT/360";
        public static string ACT_365_ISDA = "ACT/365_ISDA";
        public static string x30_360 = "30/360";

        private static string mlibArg;

        public DayCount(String rhs)
        {
            mlibArg = rhs;
        }

        public String getMlibArg()
        {
            return mlibArg;
        }
    }

    // Currency
    public class Currency
    {
        public static string EUR = "EUR";

        private static string mlibArg;

        public Currency(String rhs)
        {
            mlibArg = rhs;
        }

        public String name()
        {
            return mlibArg;
        }
    }

    // OptimizeMethod
    public class OptimizeMethod 
    {
        public static string NEWTON_RAPHSON = "NEWTONRAPHSON";
        public static string NL2SOL = "NL2SOL";
        
        private static string mlibArg;

        public OptimizeMethod(String rhs) 
        {
            mlibArg = rhs;
        }

        public String getMlibArg() 
        {
            return mlibArg;
        }
    }

    // LongTermGenerateMethod
    public class LongTermGenerateMethod
    {
        public static string DAILY_AVERAGING = "dailyaveraging";
        public static string NONE = "";

        private static string mlibArg;

        public LongTermGenerateMethod(String rhs)
        {
            mlibArg = rhs;
        }

        public String getMlibArg()
        {
            return mlibArg;
        }
    }

    // LongTermConvention
    public class LongTermConvention
    {
        public static string LOBASIS = "LOBASIS";
        public static string NONE = "";

        private static string mlibArg;

        public LongTermConvention(String rhs)
        {
            mlibArg = rhs;
        }

        public String getMlibArg()
        {
            return mlibArg;
        }
    }

    // ShortTermConvention
    public class ShortTermConvention
    {
        public static string ARITHMETIC_AVERAGE = "ARITHMETICAVERAGE";
        public static string NONE = "";

        private static string mlibArg;

        public ShortTermConvention(String rhs)
        {
            mlibArg = rhs;
        }

        public String getMlibArg()
        {
            return mlibArg;
        }
    }

    // FirstRate
    public class FirstRate
    {
        public static string SHORT_MARKET = "ShortMarket";

        private static string mlibArg;

        public FirstRate(String rhs)
        {
            mlibArg = rhs;
        }

        public String getMlibArg()
        {
            return mlibArg;
        }
    }

    // GenerateMethod
    public class GenerateMethod
    {
        public static string DAILY_COMPOUNDING = "DAILYCOMPOUNDING";

        private static string mlibArg;

        public GenerateMethod(String rhs)
        {
            mlibArg = rhs;
        }

        public String getMlibArg()
        {
            return mlibArg;
        }
    }

    // SlidingRule
    public class SlidingRule
    {
        public static string MOD_FOLLOWING = "MOD_FOLLOWING";
        public static string FOLLOWING = "FOLLOWING";
        public static string PRECEDING = "PRECEDING";
        public static string MOD_PRECEDING = "MOD_PRECEDING";
        public static string NO_CHANGE = "NO_CHANGE";

        private static string mlibArg;

        public SlidingRule(String rhs)
        {
            mlibArg = rhs;
        }

        public String toString()
        {
            return mlibArg;
        }
    }

    // Calendar
    public class Calendar
    {
        public static string AmB = "AmB";
        public static string AuB = "AuB";
        public static string BMA = "BMA";
        public static string BmB = "BmB";
        public static string FrB = "FrB";
        public static string HeB = "HeB";
        public static string HKB = "HKB";
        public static string IsB = "IsB";
        public static string JaB = "JaB";
        public static string JoB = "JoB";
        public static string LnB = "LnB";
        public static string LxB = "LxB";
        public static string MeB = "MeB";
        public static string MoB = "MoB";
        public static string MxB = "MxB";
        public static string NYB = "NYB";
        public static string RiB = "RiB";
        public static string SiB = "SiB";
        public static string SPB = "SPB";
        public static string SyB = "SyB";
        public static string Tgt = "Tgt";
        public static string TkB = "TkB";
        public static string TrB = "TrB";
        public static string WeB = "WeB";
        public static string ZuB = "ZuB";
        public static string BxB = "BxB";
        public static string RJB = "RJB";
        public static string BeB = "BeB";
        public static string MwB = "MwB";
        public static string SeB = "SeB";
        public static string TpB = "TpB";
        public static string KLB = "KLB";
        public static string BkB = "BkB";
        public static string NBR = "NBR";
        public static string ViB = "ViB";

        private static string mlibArg;

        public Calendar(String rhs)
        {
            mlibArg = rhs;
        }

        public String toString()
        {
            return mlibArg;
        }
    }

}
