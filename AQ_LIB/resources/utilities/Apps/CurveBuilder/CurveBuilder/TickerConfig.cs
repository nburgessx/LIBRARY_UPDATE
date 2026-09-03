using Newtonsoft.Json;
using System.Collections.Generic;
using System.IO;

namespace CurveBuilder
{
    class TickerConfig
    {
        public TickerConfig( string instrumentType, string jsonFileName )
        {
            mInstrumentType = instrumentType;
            initializeFromJSonFile(jsonFileName);
        }

        private void initializeFromJSonFile(string jsonFileName)
        {
            string jsonText = File.ReadAllText(jsonFileName);
            JsonTextReader reader = new JsonTextReader(new StringReader(jsonText));
            ColumnMatrix dataMatrix = JSonUtilities.ReadJsonStringMatrix(reader);

            mTickerNames = dataMatrix.values[0];
            mTenors      = dataMatrix.values[1];

            int index = 0;
            mTickerToTenorMap = new SortedDictionary<string, string>();
            foreach ( var tickerName in mTickerNames )
            {
                var tenor = mTenors[index];
                mTickerToTenorMap[tickerName] = tenor;
                index++;
            }
        }

        public string getTenor( string tickerName)
        {
            var tenor = mTickerToTenorMap[tickerName];
            return tenor;
        }

        public string mInstrumentType { get; set; }
        public List<string> mTickerNames { get; set; }
        public List<string> mTenors { get; set; }

        private SortedDictionary<string, string> mTickerToTenorMap;

    }
}
