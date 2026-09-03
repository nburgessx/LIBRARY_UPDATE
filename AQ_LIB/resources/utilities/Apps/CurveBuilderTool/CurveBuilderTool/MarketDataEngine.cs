using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CurveBuilderTool
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

        public int populateMarketDataAllCurves()
        {
            int totalSecuritiesUpdated = 0;
            foreach (var currency in mConfigManager.currencies)
            {
                foreach( var curveName in mConfigManager.getCurvesForCurrency(currency))
                {
                    var curveConfig = mConfigManager.getCurveConfig(currency, curveName);
                    int numSecuritiesUpdated = 0;
                    var populatedMarketData = populateMarketDataForCurve(curveConfig, out numSecuritiesUpdated);
                    totalSecuritiesUpdated += numSecuritiesUpdated;
                    curveConfig.marketDataTemplate = populatedMarketData;
                }
            }
            return totalSecuritiesUpdated;
        }

        public int populateMarketDataSingleCurve(CurveConfig curveConfig)
        {
            int numSecuritiesUpdated = 0;
            var populatedMarketData = populateMarketDataForCurve(curveConfig, out numSecuritiesUpdated);
            curveConfig.marketDataTemplate = populatedMarketData;
            return numSecuritiesUpdated;
        }

        private MarketDataTemplate populateMarketDataForCurve(CurveConfig curveConfig, out int numSecuritiesRequested)
        {
            BBGApi bbgApi = new BBGApi();
            const string QUOTE_UNITS = "QUOTE_UNITS";

            MarketDataTemplate populatedTemplate = curveConfig.marketDataTemplate;

            // Set AsOfDate
            string asOfDateString = getAsOfDateInExcelFormat();
            var marketDataProperties = populatedTemplate.mMarketDataBlocks[0];
            marketDataProperties.setAsOfDate(asOfDateString);

            numSecuritiesRequested = 0;

            int index = 0;
            foreach( var marketDataBlock in populatedTemplate.mMarketDataBlocks)
            {
                index++;
                if (index == 1)
                {
                    continue; // Skip over the MARKETDATAPROPERTIES block
                }
                var instrumentType = marketDataBlock.dataMatrix.name;
                var tickerConfig = curveConfig.tickerConfigMap[instrumentType];

                var instrumentFields = mConfigManager.getBbgFields(instrumentType);
                int nInstrumentFields = instrumentFields.Count;

                List<string> fields = new List<string>(instrumentFields);
                fields.Add(QUOTE_UNITS);

                List<string> securities = tickerConfig.mTickerNames;
                SortedDictionary<string, BBGResponse> bbgResults = bbgApi.sendRequest(securities, fields);
                numSecuritiesRequested += securities.Count;

                tickerConfig.setBbgRepsonses(bbgResults);
                

                // Update MarketData 
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

                            double factor = 1.0;
                            if ( "BP".Equals(quoteUnits))
                            {
                                factor = 1.0e-4;
                            }
                            else if ( "%".Equals(quoteUnits))
                            {
                                factor = 1.0e-2;
                            }
                            double scaledValue = numericalValue * factor;
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
            }
            return populatedTemplate;

        }

        private ConfigManager mConfigManager { get; set; }
    }
}
