using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;

namespace MLIB_API_CSHARP_TEST
{
    public class CurveTests
    {
        public static void rullAllTests()
        {
            test_misSetUpoisConfig();
            test_mirOutputCurve();
            test_mirGetDF1();
            test_mirGetDF2();
            test_mirGetDF3();
            test_mirGetDF4();
            test_mirGetDF5();
            test_mirGetForwardRate2();
            test_mirGetZeroRate1();
            test_mirGetZeroRate2();
        }

        //------------------------------------------------------------------
        // Private
        
        private static double EPSILON = 0.000000001;

        private static OisCurveConfiguration oisConfig = new OisCurveConfiguration();

        private static void test_misSetUpoisConfig()
        {
            string curveId = oisConfig.curveId;
            string marketName = oisConfig.marketName;
            string curveIndexNames = oisConfig.curveIndexNames.toMlib();

            VecVecString curveBuildProperties = oisConfig.curveBuildProperties.toMlib();
            VecVecString oisConv = oisConfig.oisConv.toMlib();
            VecVecString oisRates = oisConfig.oisRates.toMlib();
            VecVecString oisHistRates = oisConfig.oisHistRates.toMlib();
            VecVecString loBasisConv = oisConfig.loBasisConv.toMlib();
            VecVecString loBasisRates = oisConfig.loBasisRates.toMlib();
            VecVecString swapConv = oisConfig.swapConv.toMlib();
            VecVecString swapRates = oisConfig.swapRates.toMlib();

            string ret = MLIB_CLIENT_API.mirSetUpOISCurve(
                curveId, 
                marketName, 
                curveBuildProperties,
                oisConv, 
                oisRates,
                curveIndexNames,
                oisHistRates,
                loBasisRates,
                swapConv,
                swapRates,
                loBasisConv);
        }

        private static void test_mirOutputCurve()
        {
            VecVecDouble ret = MLIB_CLIENT_API.mirOutputCurve(oisConfig.curveId, oisConfig.marketName);
            double lastTerm = ret[0][502];
            double lastDF = ret[1][502];
            Debug.Assert(lastTerm == 50.011333183621524);
            Debug.Assert(lastDF == 0.59747792585972237);    // 0.5974779258597224
        }

        private static void test_mirGetDF1() 
        {
            double term = 1.25;
            String curveId = oisConfig.curveId;            
            String interpolation = "MonotoneConvex";
            bool isBasisFlag = false;
            String curveName = oisConfig.marketName;

            double df = MLIB_CLIENT_API.mirGetDF1(term, curveId, DayCount.ACT_360, interpolation, isBasisFlag, curveName);

            Debug.Assert((df - 1.0047031032413) < EPSILON);
        }

        public static void test_mirGetDF2() 
        {
            String term = "2Y";
            String curveId = oisConfig.curveId;
            String interpolation = "MonotoneConvex";
            bool isBasisFlag = false;
            String curveName = oisConfig.marketName;

            double df = MLIB_CLIENT_API.mirGetDF2(term, curveId, DayCount.ACT_360, SlidingRule.MOD_FOLLOWING, Calendar.Tgt, interpolation, isBasisFlag, curveName);

            Debug.Assert((df - 1.0076902534273) < EPSILON);
        }

        public static void test_mirGetDF3() 
        {
            string fromDate = "20160220";
            string toDate = "20180408";
            String curveId = oisConfig.curveId;
            String interpolation = "MonotoneConvex";
            bool isBasisFlag = false;
            String curveName = oisConfig.marketName;

            double df = MLIB_CLIENT_API.mirGetDF3(fromDate, toDate, curveId, DayCount.ACT_360, SlidingRule.MOD_FOLLOWING, Calendar.Tgt, interpolation, isBasisFlag, curveName);

            Debug.Assert((df - 1.00813391345593) < EPSILON);
        }

        public static void test_mirGetDF4() 
        {
            String fromDate = "20160220";
            double term = 5.2;
            String curveId = oisConfig.curveId;
            String interpolation = "MonotoneConvex";
            bool isBasisFlag = false;
            String curveName = oisConfig.marketName;

            double df = MLIB_CLIENT_API.mirGetDF4(fromDate, term, curveId, DayCount.ACT_360, SlidingRule.MOD_FOLLOWING, Calendar.Tgt, interpolation, isBasisFlag, curveName);

            Debug.Assert((df - 1.008554648664) < EPSILON);
        }

        public static void test_mirGetDF5() 
        {
            string fromDate = "20160220";
            String term = "2Y";
            String curveId = oisConfig.curveId;            
            String interpolation = "MonotoneConvex";
            bool isBasisFlag = false;
            String curveName = oisConfig.marketName;

            double df = MLIB_CLIENT_API.mirGetDF5(fromDate, term, curveId, DayCount.ACT_360, SlidingRule.MOD_FOLLOWING, Calendar.Tgt, interpolation, isBasisFlag, curveName);

            Debug.Assert((df - 1.00767954186227) < EPSILON);
        }

        public static void test_mirGetForwardRate2() 
        {
            VecString fromDates = new VecString();
            fromDates.Add("20211228");

            VecString toDates = new VecString();
            toDates.Add("20211228");

            String curveId = oisConfig.curveId;
            String interpolation = "MonotoneConvex";
            String curveName = oisConfig.marketName;
            bool isFwdInterp = false;
            bool useFwdData = false;

            VecString fwdRates = MLIB_CLIENT_API.mirGetForwardRate2(fromDates, toDates, curveId, Frequency.SIMPLE, DayCount.ACT_360, SlidingRule.MOD_FOLLOWING, Calendar.Tgt, interpolation, curveName, isFwdInterp, useFwdData);

            double fwdRate = Convert.ToDouble(fwdRates[0]);

            Debug.Assert((fwdRate - 0.005938170138407770) < EPSILON);
        }

        public static void test_mirGetZeroRate1() 
        {
            double term = 2.35;
            String curveId = oisConfig.curveId;
            String interpolation = "MonotoneConvex";
            String curveName = oisConfig.marketName;
            bool isFwdInterp = false;

            double zeroRate = MLIB_CLIENT_API.mirGetZeroRate1(term, curveId, Frequency.SIMPLE, DayCount.ACT_360, interpolation, curveName, isFwdInterp);

            Debug.Assert((zeroRate - 0.003684867) < EPSILON);
        }

        public static void test_mirGetZeroRate2() 
        {
            String term = "2Y";
            String curveId = oisConfig.curveId;
            String interpolation = "MonotoneConvex";
            String curveName = oisConfig.marketName;
            bool isFwdInterp = false;

            double zeroRate = MLIB_CLIENT_API.mirGetZeroRate2(term, curveId, Frequency.SIMPLE, DayCount.ACT_360, SlidingRule.MOD_FOLLOWING, Calendar.Tgt, interpolation, curveName, isFwdInterp);

            Debug.Assert((zeroRate - 0.003758363) < EPSILON);
        }
    }
}
