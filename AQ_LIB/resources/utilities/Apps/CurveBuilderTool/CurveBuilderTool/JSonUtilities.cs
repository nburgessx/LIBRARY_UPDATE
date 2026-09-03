using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;

namespace CurveBuilderTool
{
    class JSonUtilities
    {
        public static void DummyJsonReader(string jsonFileName)
        {
            string jsonText = File.ReadAllText(jsonFileName);

            JsonTextReader reader = new JsonTextReader(new StringReader(jsonText));
            while (reader.Read())
            {
                if (reader.Value != null)
                {
                    Console.WriteLine("Token: {0}, Value: {1}", reader.TokenType, reader.Value);
                }
                else
                {
                    Console.WriteLine("Token: {0}", reader.TokenType);
                }
            }
        }
        public static ColumnMatrix ReadJsonStringMatrix(JsonTextReader reader)
        {
            string matrixName = "";
            int numColumns = 0;
            List<List<string>> matrix = new List<List<string>>();

            while (reader.Read())
            {
                if (reader.TokenType == JsonToken.EndObject)
                {
                    break;
                }

                if (reader.TokenType == JsonToken.PropertyName)
                {
                    if (reader.Value.ToString() == "SK_NAME")
                    {
                        reader.Read();
                        matrixName = reader.Value.ToString();
                    }
                    else if (reader.Value.ToString() == "SK_NUMBER_OF_COLUMNS")
                    {
                        reader.Read();
                        numColumns = Int32.Parse(reader.Value.ToString());
                    }
                    else if (reader.Value.ToString().Contains("COL_"))
                    {
                        List<string> matrixColumn = readJsonColumn(reader);
                        matrix.Add(matrixColumn);
                    }
                }
            }

            ColumnMatrix dataMatrix = new ColumnMatrix();
            dataMatrix.name = matrixName;
            dataMatrix.values = matrix;
            return dataMatrix;
        }

        public static List<String> readJsonColumn(JsonTextReader reader)
        {
            List<string> column = new List<string>();

            while (reader.Read())
            {
                if (reader.TokenType == JsonToken.EndArray)
                {
                    break;
                }

                if (reader.Value != null)
                {
                    column.Add(reader.Value.ToString());
                }
            }

            // Console.WriteLine("Number of elements in column: " + column.Count);
            return column;
        }

    }
}
