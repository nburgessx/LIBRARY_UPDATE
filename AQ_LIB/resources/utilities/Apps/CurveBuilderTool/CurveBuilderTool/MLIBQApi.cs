using System;
using System.Collections.Generic;
using System.IO;

namespace CurveBuilderTool
{
    class MLIBQApi
    {
        public static string initialiseMLIBQ()
        {
            Console.WriteLine("\nLoading MLIBQ...");

            var workingDirectory = Directory.GetCurrentDirectory();

            // MLIB Initialization
            try
            {
                MLIB_CLIENT_API.setUpMLIB(@".\config\ir.properties",     // File path of ir.properties
                                          @".\config\Calendar.csv");     // File path of calendar files (in csv format)
            }
            catch (ApplicationException e)
            {
                string errorMessage = "WARNING: Config failed to load! Working dir: " + workingDirectory;
                return errorMessage;
            }

            string mlibqVersionInfo = MLIB_CLIENT_API.meUtilityVersion();
            string successMessage = "Initialised " + mlibqVersionInfo + " Working dir: " + workingDirectory;
            return successMessage;
        }

        public static void shutdownMLIBQ()
        {
            Console.WriteLine("Shutting down MLIB...");
            MLIB_CLIENT_API.tearDownMLIB();
        }

        public static string marketDataCreate(string curveMarketDataName, MarketDataTemplate populatedMarketData)
        {
            VecString keys = new VecString();
            VecVecVecString dataBlocks = new VecVecVecString();
            foreach (var marketDataBlock in populatedMarketData.mMarketDataBlocks)
            {
                var key = marketDataBlock.dataMatrix.name;
                keys.Add(key);

                var dataBlock = marketDataBlock.asVecVecString();
                dataBlocks.Add(dataBlock);
            }

            string marketDataObject = MLIB_CLIENT_API.meLWOCurveMarketDataCreateUsingMultipleBlocks(curveMarketDataName,
                                                                                                    keys,
                                                                                                    dataBlocks);


            Console.WriteLine("marketDataObject : " + marketDataObject);
            return marketDataObject;
        }


        public static void saveMarketDataObject(string filepath, string mktDataObjectName)
        {
            // Save the market data object
            string objectType = "CURVE_MARKETDATA";
            MLIB_CLIENT_API.meLWOSave(mktDataObjectName, objectType, filepath);
        }

        public static void saveCurveObject(string filepath, string curveObjectName)
        {
            // Save the curve object
            string objectType = "CURVE";
            MLIB_CLIENT_API.meLWOSave(curveObjectName, objectType, filepath);
        }

        public static string buildCurve(CurveConfig curveConfig, ref CurveBuildData curveBuildData)
        {
            string curveObjectName = curveConfig.currency + "_" + curveConfig.curveName + "_CURVE";
            string domesticCurveCollection = curveConfig.currency + "YC";
            string foreignCurveCollection = "";

            string curveObject = MLIB_CLIENT_API.meLWOCurveCalibrate(curveObjectName,
                                                                     curveConfig.curveGenerator,
                                                                     curveBuildData.mMarketDataObjectHandle,
                                                                     domesticCurveCollection,
                                                                     foreignCurveCollection);

            curveBuildData.mCurveObjectHandle = curveObjectName;

            return curveObject;
        }

    }
}
