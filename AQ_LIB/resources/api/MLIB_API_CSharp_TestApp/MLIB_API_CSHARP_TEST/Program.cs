using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using MLIB_API_CSHARP_TEST;

namespace MLIB_CLIENT_API_TEST
{
    class Program
    {
        static void Main(string[] args)
        {
            try
            {
                //--------------------------------------------------------
                // MLIB Initialization
                MLIB_CLIENT_API.setUpMLIB(@"C:\MLIB\mlib\src\MLIB_CLIENT_API\csharp\MLIB_API_CSharp_Test\MLIB_API_CSHARP_TEST\bin\Debug\ir.properties",     // File path of ir.properties
                                          @"C:\MLIB\mlib\src\MLIB_CLIENT_API\csharp\MLIB_API_CSharp_Test\MLIB_API_CSHARP_TEST\bin\Debug\Calendar.csv");     // File path of calendar files (in csv format)

                //--------------------------------------------------------
                // Main tests
                TypeTests.rullAllTests();
                DateTests.rullAllTests();
                CurveTests.rullAllTests();

                //--------------------------------------------------------
                // Shut down MLIB
                MLIB_CLIENT_API.tearDownMLIB();

                Console.WriteLine("All tests passed!");
            }
            catch (Exception ex)
            {
                Console.Write(ex.InnerException);
            }
        }
    }
}
