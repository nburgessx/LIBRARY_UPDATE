using System;
using System.Collections.Generic;



namespace CurveBuilder
{
    class Program
    {
        private static void pauseBeforeExit()
        {
            System.Console.WriteLine("Press ENTER to quit");
            System.Console.Read();
        }
 
        private static void readOISCurveConfig(out List<TickerConfig> tickerConfigs, out MarketDataTemplate oisMarketDataTemplate)
        {
            tickerConfigs = new List<TickerConfig>();
            string oisInstrumentType = "OISSWAP";   // This type should match one of the instrument names in the InstrumentBbgFields.JSON file, so that the corresponding Bloomberg fields can be looked up
            string oisTickersFileName = @"C:\Temp\USD_OIS_OIS_Tickers.JSON";
            TickerConfig oisTickers = new TickerConfig(oisInstrumentType, oisTickersFileName);
            tickerConfigs.Add(oisTickers);

            string lobInstrumentType = "LIBOROISBASISSPREADS";
            string lobTickersFileName = @"C:\Temp\USD_OIS_LOB_Tickers.JSON";
            TickerConfig lobTickers = new TickerConfig(lobInstrumentType, lobTickersFileName); // Should this get the name from the block?
            tickerConfigs.Add(lobTickers);

            string liborSwapsInstrumentType = "SWAPS";
            string liborSwapsTickersFileName = @"C:\Temp\USD_OIS_LIBORSWAPS_Tickers.JSON";
            TickerConfig lsTickers = new TickerConfig(liborSwapsInstrumentType, liborSwapsTickersFileName);
            tickerConfigs.Add(lsTickers);

            string marketDataName = "USD_OIS_CURVE_MARKETDATA";
            string marketDataFilename = @"C:\Temp\USD_OIS_CURVE_MARKETDATA@4.JSON";
            oisMarketDataTemplate = new MarketDataTemplate(marketDataName, marketDataFilename);
        }


        private static void readSTDCurveConfig(out List<TickerConfig> tickerConfigs, out MarketDataTemplate stdMarketDataTemplate)
        {
            tickerConfigs = new List<TickerConfig>();
            string oisInstrumentType = "LIBORFIXINGS";
            string oisTickersFileName = @"C:\Temp\USD_STD_LIBORFIXINGS.JSON";
            TickerConfig oisTickers = new TickerConfig(oisInstrumentType, oisTickersFileName);
            tickerConfigs.Add(oisTickers);

            string lobInstrumentType = "FUTURES";
            string lobTickersFileName = @"C:\Temp\USD_STD_FUTURES.JSON";
            TickerConfig lobTickers = new TickerConfig(lobInstrumentType, lobTickersFileName); // Should this get the name from the block?
            tickerConfigs.Add(lobTickers);

            string liborSwapsInstrumentType = "SWAPS";
            string liborSwapsTickersFileName = @"C:\Temp\USD_STD_SWAPS.JSON";
            TickerConfig lsTickers = new TickerConfig(liborSwapsInstrumentType, liborSwapsTickersFileName);
            tickerConfigs.Add(lsTickers);

            string marketDataName = "USD_STD_CURVE_MARKETDATA";
            string marketDataFilename = @"C:\Temp\USD_SWAP_3M_CURVE_MARKETDATA@6.JSON";
            stdMarketDataTemplate = new MarketDataTemplate(marketDataName, marketDataFilename);
        }


        static void Main(string[] args)
        {
         
		    string staticInstrumentBBgFields = @"C:\Temp\InstrumentBbgFields.JSON";
            //JSonUtilities.DummyJsonReader(staticInstrumentFields);

            ConfigManager configManager = new ConfigManager(staticInstrumentBBgFields);

            // ----- OIS CURVE -----
            List<TickerConfig> oisTickerConfigs;
            MarketDataTemplate oisMarketDataTemplate;
            readOISCurveConfig(out oisTickerConfigs, out oisMarketDataTemplate);

            // The engine takes the MarketDataTickers and MarketDataTemplate,
            // and populates the template with marketdata retrieved from Bloomberg.
            MarketDataEngine marketDataEngine = new MarketDataEngine(configManager);
            MarketDataTemplate oisMarketData = marketDataEngine.populateMarketData(oisTickerConfigs, oisMarketDataTemplate);

            //oisMarketData.display();

            // ----- STD CURVE -----
            List<TickerConfig> stdTickerConfigs;
            MarketDataTemplate stdMarketDataTemplate;
            readSTDCurveConfig(out stdTickerConfigs, out stdMarketDataTemplate);
            MarketDataTemplate stdMarketData = marketDataEngine.populateMarketData(stdTickerConfigs, stdMarketDataTemplate);

            // ----- CURVE BUILDING -----
            MLIBQApi.initialiseMLIBQ();

            string oisMarketDataName = "OIS_MARKET_DATA";
            string oisMarketDataObject = MLIBQApi.marketDataCreate(oisMarketDataName, oisMarketData);

            string oisFilepath = @"C:\temp\USD_OIS_PopulatedMarketData.json";
            MLIBQApi.saveMarketDataObject(oisFilepath, oisMarketDataObject);

            string oisCurveGenerator = "USD_OIS";
            MLIBQApi.buildCurve(oisCurveGenerator, oisMarketDataObject);

            string stdMarketDataName = "STD_MARKET_DATA";
            string stdMarketDataObject = MLIBQApi.marketDataCreate(stdMarketDataName, stdMarketData);

            string stdFilepath = @"C:\temp\USD_STD_PopulatedMarketData.json";
            MLIBQApi.saveMarketDataObject(stdFilepath, stdMarketDataObject);

            string stdCurveGenerator = "USD_SWAP_3M";
            MLIBQApi.buildCurve(stdCurveGenerator, stdMarketDataObject);

            MLIBQApi.shutdownMLIBQ();
        }

    }
}
