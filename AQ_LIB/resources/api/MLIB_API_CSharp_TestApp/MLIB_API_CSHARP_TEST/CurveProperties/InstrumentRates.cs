using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;

namespace MLIB_API_CSHARP_TEST
{
    public class CurveIndexNames {

        private static String DELIMITER = ":";

        private List<String> indexNames;

        private CurveIndexNames(List<String> rhs) 
        {
            indexNames = rhs;
        }

        public String toMlib()
        {
            String ret = null;

            int size = indexNames.Count;
            for (int i = 0; i < size-1; ++i)
            {
                ret += indexNames[i] + DELIMITER;
            }

            ret += indexNames[size - 1];
            return ret;
        }

        public class Builder 
        {
            private List<String> indexNames = new List<String>();

            public Builder addName(String name) 
            {
                indexNames.Add(name);

                return this;
            }

            public CurveIndexNames build() 
            {
                return new CurveIndexNames(indexNames);
            }
        }
    }    
}
