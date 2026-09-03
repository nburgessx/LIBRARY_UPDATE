using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;

namespace MLIB_API_CSHARP_TEST
{
    public class TypeTests
    {
        public static void rullAllTests()
        {
            double a = MLIB_CLIENT_API.getDouble();
            Debug.Assert(a == 2.5);

            int b = MLIB_CLIENT_API.getInt();
            Debug.Assert(b == 1);

            char c = MLIB_CLIENT_API.getChar();
            Debug.Assert(c == 'z');

            bool d = MLIB_CLIENT_API.getBool();
            Debug.Assert(d == false);

            string e = MLIB_CLIENT_API.getConstChar();
            Debug.Assert(e == "Here you go!");

            string f = MLIB_CLIENT_API.getStdString();
            Debug.Assert(f == "The Real Deal");

            VecInteger g = MLIB_CLIENT_API.getVectorOfInt();
            Debug.Assert(g[1] == 1);

            VecVecInteger h = MLIB_CLIENT_API.getVectorOfVectorOfInt();
            Debug.Assert(h[0][2] == 40);

            VecDouble i = MLIB_CLIENT_API.getVectorOfDouble();
            Debug.Assert(i[3] == 25.89);

            VecVecDouble j = MLIB_CLIENT_API.getVectorOfVectorOfDouble();
            Debug.Assert(j[1][3] == 250.888);

            VecString k = MLIB_CLIENT_API.getVectorOfString();
            Debug.Assert(k[4] == "E");

            VecVecString l = MLIB_CLIENT_API.getVectorOfVectorOfString();
            Debug.Assert(l[1][4] == "V");

            bool exceptionThrowAndCaught = false;
            try
            {
                MLIB_CLIENT_API.throwAnException();
            }
            catch
            {
                exceptionThrowAndCaught = true;
            }
            Debug.Assert(exceptionThrowAndCaught == true);
        }

    }
}
