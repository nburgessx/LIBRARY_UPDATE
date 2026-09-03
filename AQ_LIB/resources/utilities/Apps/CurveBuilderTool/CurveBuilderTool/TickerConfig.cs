using Newtonsoft.Json;
using System.Collections.Generic;
using System.IO;


namespace CurveBuilderTool
{
    public class TickerConfig
    {
        public TickerConfig(string instrumentType, string jsonFileName)
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
            mTenors = dataMatrix.values[1];

            int index = 0;
            mTickerToTenorMap = new SortedDictionary<string, string>();
            foreach (var tickerName in mTickerNames)
            {
                var tenor = mTenors[index];
                mTickerToTenorMap[tickerName] = tenor;
                index++;
            }
        }

        public string getTenor(string tickerName)
        {
            var tenor = mTickerToTenorMap[tickerName];
            return tenor;
        }

        public BBGResponse getBbgResponseForTicker(string tickerName)
        {
            BBGResponse bbgResponse = null;

            if (mBbgResponses != null)
            {
                if (mBbgResponses.ContainsKey(tickerName))
                {
                    bbgResponse = mBbgResponses[tickerName];
                }
            }
            return bbgResponse;
        }

        public void setBbgRepsonses(SortedDictionary<string, BBGResponse> bbgResponses)
        {
            mBbgResponses = bbgResponses;
        }

        public string mInstrumentType { get; set; }
        public List<string> mTickerNames { get; set; }
        public List<string> mTenors { get; set; }

        private SortedDictionary<string, BBGResponse> mBbgResponses;
        
        private SortedDictionary<string, string> mTickerToTenorMap;

    }
}
