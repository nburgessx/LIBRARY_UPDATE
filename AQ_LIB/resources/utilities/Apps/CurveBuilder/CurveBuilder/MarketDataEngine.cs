using System;
using System.Collections.Generic;

namespace CurveBuilder
{
    class MarketDataEngine
    {
        public MarketDataEngine(ConfigManager configManager)
        {
            mConfigManager = configManager;
        }
        private static string getAsOfDateInExcelFormat()
        {
            DateTime asOfDate = DateTime.Today;
            double excelDateAsDouble = asOfDate.ToOADate();
            string asOfDateString = excelDateAsDouble.ToString();
            Console.WriteLine("AsOfDate : " + asOfDateString);
            return asOfDateString;
        }

        public MarketDataTemplate populateMarketData(List<TickerConfig> tickerConfigs, MarketDataTemplate template)
        {
            BBGApi bbgApi = new BBGApi();
            const string QUOTE_UNITS = "QUOTE_UNITS";

            MarketDataTemplate populatedTemplate = template;

            // Set AsOfDate
            string asOfDateString = getAsOfDateInExcelFormat();
            var marketDataProperties = populatedTemplate.mMarketDataBlocks[0];
            marketDataProperties.setAsOfDate(asOfDateString);

            int index = 1;   // Skip over the MARKETDATAPROPERTIES block
            foreach (var tickerConfig in tickerConfigs)
            {
                var instrumentType = tickerConfig.mInstrumentType;
                var instrumentFields = mConfigManager.getBbgFields(instrumentType);
                int nInstrumentFields = instrumentFields.Count;

                List<string> fields = new List<string>(instrumentFields);
                fields.Add(QUOTE_UNITS);

                List<string> securities = tickerConfig.mTickerNames;
                SortedDictionary<string, BBGResponse> bbgResults = bbgApi.sendRequest(securities, fields);

                // Now populate the prices in the market data template
                var marketDataBlock = populatedTemplate.mMarketDataBlocks[index];

                foreach (var item in bbgResults)
                {
                    BBGResponse bbgResponse = item.Value;

                    var tickerName = bbgResponse.tickerName;

                    List<string> bbgValues = new List<string>();
                    foreach (var field in instrumentFields)
                    {
                        //Console.WriteLine("Processing field: " + field);
                        string bbgValue = bbgResponse.fieldData[field];
                        if (field.Contains("LAST"))
                        {
                            // This is a price field. Scale by the QUOTE_UNITS
                            var quoteUnits = bbgResponse.fieldData[QUOTE_UNITS];

                            double numericalValue = Double.Parse(bbgValue);
                            double factor = "BP".Equals(quoteUnits) ? 1e4 : 100;
                            double scaledValue = numericalValue / factor;
                            bbgValues.Add(scaledValue.ToString());
                        }
                        else if (field.Contains("_DT"))
                        {
                            // Convert date to Excel format
                            DateTime bgDate = Convert.ToDateTime(bbgValue);
                            double excelDate = bgDate.ToOADate();
                            bbgValues.Add(excelDate.ToString());
                        }
                        else
                        {
                            bbgValues.Add(bbgValue);
                        }
                    }

                    //Console.WriteLine("Looking up tickerName: " + tickerName);
                    var tenor = tickerConfig.getTenor(tickerName);
                    marketDataBlock.setMarketDataForTenor(tenor, nInstrumentFields, bbgValues);
                }

                index++;
            }
            return populatedTemplate;

        }

        private ConfigManager mConfigManager { get; set; }
        public List<TickerConfig> mTickerConfigs { get; set; }
        public MarketDataTemplate mTemplate { get; set; }
    }
}
