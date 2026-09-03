using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using NUnit.Framework;

namespace TestMLIB
{
    [TestFixture]
    public class DateTests
    {

        [SetUp]
        public void setup()
        {
            String irPropsFullFilePath  = "";
            String calendarFullFilePath = "";
            String result = MLIB_CLIENT_API.setUpMLIB( irPropsFullFilePath, calendarFullFilePath );

            String expected = "Initialized MLIB";
            StringAssert.IsMatch(expected, result);
        }
        
        [TearDown]
        public void teardown()
        {
            String result = MLIB_CLIENT_API.tearDownMLIB();

            String expected = "Finalized MLIB";
            StringAssert.IsMatch(expected, result);
        }


        [Test]
        public void TestMirGetDateFromTerm()
        {
            String BaseDate       = "20170629";
            String Term           = "2D";
            String SlidingRule    = "MOD_FOLLOWING";
            String Calendar       = "LNB";
            String RollConvention = "NORMAL";
            String result = MLIB_CLIENT_API.mirGetDate( BaseDate, Term, SlidingRule, Calendar, RollConvention );

            String expected = "20170703";
            StringAssert.IsMatch(expected, result);
        }

    }
}
