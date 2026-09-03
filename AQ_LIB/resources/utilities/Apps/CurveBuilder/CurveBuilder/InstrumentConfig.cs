using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;

namespace CurveBuilder
{
    class InstrumentConfig
    {
        public string instrumentType { get; set; }
        public List<string> bbgFields { get; set; }
    }

    class ConfigManager
    {
        public ConfigManager(string jsonFileName)
        {
            //Console.WriteLine("Initializing ConfigManager");
            initializeFromJSonFile(jsonFileName);
        }

        private void initializeFromJSonFile(string jsonFileName)
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

        public List<string> getBbgFields( string instrumentType )
        {
            return instrumentConfigMap[instrumentType].bbgFields;
        }

        private SortedDictionary<string, InstrumentConfig> instrumentConfigMap;
    }

}
