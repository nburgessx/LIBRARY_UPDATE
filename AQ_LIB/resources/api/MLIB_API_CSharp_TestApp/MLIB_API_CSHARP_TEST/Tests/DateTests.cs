using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;

namespace MLIB_API_CSHARP_TEST
{
    public class DateTests
    {
        public static void rullAllTests()
        {
            test_mirGetDate();
            test_mirGetDateFromTerm();
            test_mirGetImmDate1();
            test_mirGetImmDate2();
            test_mirGetImmDate3();
            test_mirGetBusinessDays();
            test_mirGetTerm();
            test_mirGetEcbStartDate();
            test_mirGetNextEcbDate();
            test_mirGetNextCbDate();
        }

        //------------------------------------------------------------------
        // Private

        private static string baseDate = "20170115";

        private static void test_mirGetDate()
        {            
            string ret = MLIB_CLIENT_API.mirGetDate(baseDate, "3M", "MOD_FOLLOWING", "TGT", "NORMAL");
            Debug.Assert(ret == "20170418");
        }

        private static void test_mirGetDateFromTerm()
        {
            string ret = MLIB_CLIENT_API.mirGetDateFromTerm(baseDate, 1.0, "ACT/360", false);
            Debug.Assert(ret == "20180110");
        }

        private static void test_mirGetImmDate1()
        {
            string ret = MLIB_CLIENT_API.mirGetIMMDate1(2016, 3, "TGT", "MOD_FOLLOWING");
            Debug.Assert(ret == "20160316");
        }

        private static void test_mirGetImmDate2()
        {
            string ret = MLIB_CLIENT_API.mirGetIMMDate2(2016, 1, "TGT", "MOD_FOLLOWING");
            Debug.Assert(ret == "20160316");
        }

        private static void test_mirGetImmDate3()
        {
            string ret = MLIB_CLIENT_API.mirGetIMMDate3("20160201", 1, "TGT", "MOD_FOLLOWING");
            Debug.Assert(ret == "20160316");
        }
        
        private static void test_mirGetBusinessDays()
        {
            int ret = MLIB_CLIENT_API.mirGetBusinessDays("20160203", "20160503", "TGT");
            Debug.Assert(ret == 62);
        }

        private static void test_mirGetTerm()
        {
            double ret = MLIB_CLIENT_API.mirGetTerm("20160203", "20160503", "ACT/360", false);
            Debug.Assert(ret == 0.25);
        }

        private static void test_mirGetEcbStartDate()
        {
            string ret = MLIB_CLIENT_API.mirGetECBStartDate("20160421");
            Debug.Assert(ret == "20160427");
        }

        private static void test_mirGetNextEcbDate()
        {
            string ret = MLIB_CLIENT_API.mirGetNextECBDate("20160419", false);
            Debug.Assert(ret == "20160421");
        }

        private static void test_mirGetNextCbDate()
        {
            string ret = MLIB_CLIENT_API.mirGetNextCBDate("ECB", "20160419", false);
            Debug.Assert(ret == "20160421");
        }


    }
}
