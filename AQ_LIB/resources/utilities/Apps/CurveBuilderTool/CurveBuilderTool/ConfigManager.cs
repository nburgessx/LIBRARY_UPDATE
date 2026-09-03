using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;

namespace CurveBuilderTool
{
    class InstrumentConfig
    {
        public string instrumentType { get; set; }
        public List<string> bbgFields { get; set; }
    }

    public class CurveGeneratorConfig
    {
        public CurveGeneratorConfig(string curveName, string curveGeneratorName)
        {
            this.curveName = curveName;
            this.curveGeneratorName = curveGeneratorName;
        }

        public string curveName { get; set; }
        public string curveGeneratorName { get; set; }
    }
    class CurrencyConfig
    {
        public CurrencyConfig( string name, List<string> curves, List<CurveGeneratorConfig> curveGeneratorList )
        {
            currencyName = name;
            curveNames = curves;
            curveGenerators = curveGeneratorList;
        }
        public string currencyName { get; set; }
        public List<string> curveNames { get; set; }
        public List<CurveGeneratorConfig> curveGenerators { get; set; }
    }

    public class CurveConfig
    {
        public string currency { get; set; }
        public string curveName { get; set; }
        public string curveGenerator { get; set; }
        public SortedDictionary<string, TickerConfig> tickerConfigMap { get; set; }
        public MarketDataTemplate marketDataTemplate { get; set; }
    }


    public class ConfigManager
    {
        public ConfigManager(string folderPath)
        {
            string jsonFilename = folderPath + @"\InstrumentBbgFields.JSON";
            initializeInstrumentConfigFromJSonFile(jsonFilename);
            initializeCurveMaps(folderPath);

            mCurveBuildDataCache = new CurveBuildDataCache();
        }

        private void initializeCurveMaps(string folderPath)
        {
            currencyCurveMap = new SortedDictionary<string, CurrencyConfig>();
            curveConfigMap = new SortedDictionary<string, SortedDictionary<string, CurveConfig>>();

            string[] currencyFolders = Directory.GetDirectories(folderPath);
            currencies = new List<string>();

            foreach (var currencyFolderPath in currencyFolders)
            {
                string currency = currencyFolderPath.Remove(0, currencyFolderPath.LastIndexOf('\\') + 1);
                currencies.Add(currency);

                string[] curveFolders = Directory.GetDirectories(currencyFolderPath);
                List<string> curveList = new List<string>();

                curveConfigMap[currency] = new SortedDictionary<string, CurveConfig>();
                foreach (var curveFolderPath in curveFolders)
                {
                    string curveName = curveFolderPath.Remove(0, curveFolderPath.LastIndexOf('\\') + 1);
                    curveList.Add(curveName);
                    loadCurveConfig(curveFolderPath, currency, curveName);
                }

                List<CurveGeneratorConfig> curveGenerators = loadCurveGenerators(currencyFolderPath);

                // Store each curveGeneratorName in the corresponding CurveConfig.
                foreach (var curveGeneratorConfig in curveGenerators)
                {
                    var curveName = curveGeneratorConfig.curveName;
                    if ( curveConfigMap[currency].ContainsKey( curveName))
                    {
                        curveConfigMap[currency][curveName].curveGenerator = curveGeneratorConfig.curveGeneratorName;
                    }
                }

                // Store the *list* of curve generators in order to preserve the build order
                currencyCurveMap[currency] = new CurrencyConfig(currency, curveList, curveGenerators);
                
            }
        }

        private List<CurveGeneratorConfig> loadCurveGenerators( string currencyFolderPath )
        {
            List<CurveGeneratorConfig> curveGenerators = new List<CurveGeneratorConfig>();

            string[] generatorFiles = Directory.GetFiles(currencyFolderPath, "*GENERATOR*");

            foreach (var gemeratorFilenameWithPath in generatorFiles)
            {
                string[] lines = File.ReadAllLines(gemeratorFilenameWithPath);

                // Display the file contents by using a foreach loop.
                foreach (string line in lines)
                {
                    // skip over comments
                    if ( line.StartsWith("#"))
                    {
                        continue;
                    }
                    var tokens = line.Split(new char[] { ' ', '\t' }, StringSplitOptions.RemoveEmptyEntries); // split on whitespace

                    // Skip over blank lines, or other incorrect formatting
                    if (tokens.Length == 2)
                    {
                        var curveName = tokens[0];
                        var curveGeneratorName = tokens[1];
                        CurveGeneratorConfig curveGeneratorConfig = new CurveGeneratorConfig(curveName, curveGeneratorName);
                        curveGenerators.Add(curveGeneratorConfig);
                    }
                }
            }

            return curveGenerators;
        }

        private void loadCurveConfig(string curveFolderPath, string currency, string curveName)
        {
            // Load all the TickerConfigs
            CurveConfig curveConfig = new CurveConfig();
            curveConfig.currency = currency;
            curveConfig.curveName = curveName;
            curveConfig.tickerConfigMap = new SortedDictionary<string, TickerConfig>();

            string[] tickerFiles = Directory.GetFiles(curveFolderPath, "*Tickers*");
            foreach (var tickerFilenameWithPath in tickerFiles)
            {
                string filename = tickerFilenameWithPath.Remove(0, tickerFilenameWithPath.LastIndexOf('\\') + 1);
                string[] components = filename.Split('_');

                // CCY_CURVENAME_INSTRUMENTTYPE_Tickers.JSON
                if (components.Length != 4)
                {
                    string message = "Please rename filename '" + filename + "'.\n";
                    message += "Ticker filename must be of the form: CCY_CURVENAME_INSTRUMENTTYPE_Tickers.JSON.";
                    throw new InvalidOperationException(message);
                }

                string filenameCCY = components[0];
                if (! filenameCCY.Equals( currency ))
                {
                    string message = "Please rename filename '" + filename + "'.\n";
                    message += "Ticker filename must be of the form: CCY_CURVENAME_INSTRUMENTTYPE_Tickers.JSON.\n";
                    message += "I expected the filename to begin with: '" + currency + "'.";
                    throw new InvalidOperationException(message);
                }

                string filenameCurveName = components[1];
                if (!filenameCurveName.Equals(curveName))
                {
                    string message = "Please rename filename '" + filename + "'.\n";
                    message += "Ticker filename must be of the form: CCY_CURVENAME_INSTRUMENTTYPE_Tickers.JSON.\n";
                    message += "I expected the filename to contain the curve-name: '" + curveName + "'.";
                    throw new InvalidOperationException(message);
                }

                string instrumentType = components[2]; // Until we get something better

                if (! instrumentConfigMap.ContainsKey( instrumentType ))
                {
                    string message = "Please rename filename '" + filename + "'.\n";
                    message += "Ticker filename must be of the form: CCY_CURVENAME_INSTRUMENTTYPE_Tickers.JSON.\n";
                    message += "Unknown instrumentType: '" + instrumentType + "'.";
                    throw new InvalidOperationException(message);
                }

                TickerConfig tickerConfig = new TickerConfig(instrumentType, tickerFilenameWithPath);
                curveConfig.tickerConfigMap[instrumentType] = tickerConfig;
            }

            string[] marketDataTemplateFiles = Directory.GetFiles(curveFolderPath, "*MARKETDATA*");
            foreach (var templateFilename in marketDataTemplateFiles)
            {
                string marketDataName = currency + "_" + curveName + "_CURVE_MARKETDATA";
                MarketDataTemplate marketDataTemplate = new MarketDataTemplate (marketDataName, templateFilename);
                curveConfig.marketDataTemplate = marketDataTemplate;
            }

            curveConfigMap[currency][curveName] = curveConfig;
        }

        private void initializeInstrumentConfigFromJSonFile(string jsonFileName)
        {
            instrumentConfigMap = new SortedDictionary<string, InstrumentConfig>();

            string jsonText = File.ReadAllText(jsonFileName);
            JsonTextReader reader = new JsonTextReader(new StringReader(jsonText));

            // Skip forwards until the StartArray
            while (reader.Read())
            {
                //Console.WriteLine("Found token: " + reader.TokenType.ToString());
                if (reader.TokenType == JsonToken.StartArray)
                {
                    break;
                }
            }

            Console.WriteLine("Begin reading InstrumentConfigs");
            // Read in the instrumentConfigs
            while (reader.Read())
            {
                InstrumentConfig instrumentConfig = ReadJsonInstrumentConfig(reader);
                if (!String.IsNullOrEmpty(instrumentConfig.instrumentType) && instrumentConfig.bbgFields.Count > 0)
                {
                    //Console.WriteLine("Just read InstrumentType: " + instrumentConfig.instrumentType);

                    instrumentConfigMap[instrumentConfig.instrumentType] = instrumentConfig;
                }

            }

            Console.WriteLine("Initialised ConfigManager with " + instrumentConfigMap.Count + " InstrumentConfigs.\n");
        }

        private static InstrumentConfig ReadJsonInstrumentConfig(JsonTextReader reader)
        {

            InstrumentConfig instrumentConfig = new InstrumentConfig();
            List<string> bbgFields = new List<string>();

            do
            {
                if (reader.TokenType == JsonToken.EndArray)
                {
                    break;
                }

                if (reader.TokenType == JsonToken.PropertyName)
                {
                    string instrumentType = reader.Value.ToString();
                    //Console.WriteLine("InstrumentType: " + instrumentType);
                    instrumentConfig.instrumentType = instrumentType;
                }
                else if (reader.TokenType == JsonToken.String)
                {
                    string field = reader.Value.ToString();
                    //Console.WriteLine("   field: " + field);
                    bbgFields.Add(field);
                }
            } while (reader.Read());

            instrumentConfig.bbgFields = bbgFields;
            return instrumentConfig;
        }

        public List<string> getBbgFields(string instrumentType)
        {
            return instrumentConfigMap[instrumentType].bbgFields;
        }

        public List<string> getCurvesForCurrency( string currency )
        {
            return currencyCurveMap[currency].curveNames;
        }

        public List<CurveGeneratorConfig> getCurveGeneratorsForCurrency( string currency )
        {
            return currencyCurveMap[currency].curveGenerators;
        }

        public CurveConfig getCurveConfig( string currency, string curveName )
        {
            if ( ! curveConfigMap.ContainsKey( currency))
            {
                string message = "Unknown Currency: '" + currency + "'.\n";
                message += "Please check on Config page for list of valid currencies.";
                throw new InvalidOperationException(message);
            }
            if ( ! curveConfigMap[currency].ContainsKey( curveName ))
            {
                string message = "Unknown Curve name: '" + curveName + "' for currency: '" + currency + "'.\n";
                message += "Please check on Config page for list of valid curve names.";
                throw new InvalidOperationException(message);
            }
            return curveConfigMap[currency][curveName];
        }

        public string getInstrumentTypesAsString()
        {
            string instrumentTypes = string.Join(",  ", instrumentConfigMap.Keys);
            return instrumentTypes;
        } 



        public int getNumberOfInstrumentConfigs()
        {
            return instrumentConfigMap.Count;
        }

        public int getNumberOfCurrencies()
        {
            return currencies.Count;
        }

        public int getNumberOfCurveConfigs()
        {
            return curveConfigMap.Count;
        }

        private SortedDictionary<string, InstrumentConfig> instrumentConfigMap;

        public List<string> currencies { get; set; }

        // Map from currency --> to CurrencyConfig (which contains CurveNames)
        private SortedDictionary<string, CurrencyConfig> currencyCurveMap;

        // Map from currency --> to map from curveName to CurveConfig (which contains TickerConfigs, MarketDtaTemplate)
        private SortedDictionary<string, SortedDictionary<string, CurveConfig> > curveConfigMap;

        public CurveBuildDataCache mCurveBuildDataCache { get; set; }

    }
}
