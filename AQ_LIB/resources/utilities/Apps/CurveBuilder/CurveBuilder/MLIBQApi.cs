using System;
using System.Collections.Generic;


namespace CurveBuilder
{
    class MLIBQApi
    {
        public static void initialiseMLIBQ()
        {
            Console.WriteLine("\nLoading MLIBQ...");

            // MLIB Initialization
            try
            {
                MLIB_CLIENT_API.setUpMLIB(@".\config\ir.properties",     // File path of ir.properties
                                          @".\config\Calendar.csv");     // File path of calendar files (in csv format)
            }
            catch (ApplicationException e)
            {
                System.Console.WriteLine("\n#Error: Unable to load MLIBQ DLL. Check the path to the configuration files.");
                System.Console.WriteLine("\nThe error is: " + e.ToString());
            }

            String mlibqVersionInfo = MLIB_CLIENT_API.meUtilityVersion();
            Console.WriteLine("" + mlibqVersionInfo);

        }

        public static void shutdownMLIBQ()
        {
            Console.WriteLine("Shutting down MLIB...");
            MLIB_CLIENT_API.tearDownMLIB();
        }

        public static string marketDataCreate(string curveMarketDataName, MarketDataTemplate populatedMarketData)
        {
            List<string> keys = new List<string>();
            List<VecVecString> dataBlocks = new List<VecVecString>();
            foreach(var marketDataBlock in populatedMarketData.mMarketDataBlocks)
            {
                var key = marketDataBlock.dataMatrix.name;
                keys.Add(key);

                var dataBlock = marketDataBlock.asVecVecString();
                dataBlocks.Add(dataBlock);
            }

            if (keys.Count != 4)
            {
                Console.WriteLine("Error: Expecting 4 sections of marketData but only found " + keys.Count);
            }

            string marketDataObject = MLIB_CLIENT_API.meLWOCurveMarketDataCreate(curveMarketDataName,
                                                                                 keys[0], dataBlocks[0],
                                                                                 keys[1], dataBlocks[1],
                                                                                 keys[2], dataBlocks[2],
                                                                                 keys[3], dataBlocks[3]);

             Console.WriteLine("marketDataObject : " + marketDataObject);
            return marketDataObject;
        }

        public static void saveMarketDataObject(string filepath, string mktDataObjectName)
        {

            // Save the market data object
            string objectType = "CURVE_MARKETDATA";
            MLIB_CLIENT_API.meLWOSave(mktDataObjectName, objectType, filepath);
        }

        public static void buildCurve(string curveGenerator, string marketDataObject)
        {
            string curveObjectName = "USD_OIS_CURVE";
            string domesticCurveCollection = "USDYC";
            string foreignCurveCollection = "";
            string curveObject = MLIB_CLIENT_API.meLWOCurveCalibrate(curveObjectName, curveGenerator, marketDataObject, domesticCurveCollection, foreignCurveCollection);
            Console.WriteLine("curveObject : " + curveObject);
        }
    }
}
