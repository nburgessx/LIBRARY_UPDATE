using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CurveBuilderTool
{

    public class CurveBuildData
    {
        public CurveBuildData( string currency, string curveName, string marketDataObjectHandle )
        {
            mCurrency = currency;
            mCurveName = curveName;
            mMarketDataObjectHandle = marketDataObjectHandle;
        }
        public string mCurrency { get; set; }
        public string mCurveName { get; set; }
        public string mMarketDataObjectHandle { get; set; }
        public string mCurveObjectHandle { get; set; }
    }


    public class CurveBuildDataCache
    {
        public CurveBuildDataCache()
        {
            curveBuildDataMap = new SortedDictionary<string, SortedDictionary<string, CurveBuildData>>();
        }

        public void setCurveBuildData( CurveBuildData curveBuildData)
        {
            var currency = curveBuildData.mCurrency;
            var curveName = curveBuildData.mCurveName;

            if (! (curveBuildDataMap.ContainsKey( currency )))
            {
                curveBuildDataMap[currency] = new SortedDictionary<string, CurveBuildData>();
            }
            curveBuildDataMap[currency][curveName] = curveBuildData;
        }

        public CurveBuildData getCurveBuildData( string currency, string curveName)
        {
            CurveBuildData curveBuildData = null;
            if ( curveBuildDataMap.ContainsKey( currency ))
            {
                if (curveBuildDataMap[currency].ContainsKey( curveName ))
                {
                    curveBuildData = curveBuildDataMap[currency][curveName];
                }
            }
            return curveBuildData;
        }

        // Map from currency, curveName to CurveResults object
        private SortedDictionary<string, SortedDictionary<string, CurveBuildData>> curveBuildDataMap;

    }
}
