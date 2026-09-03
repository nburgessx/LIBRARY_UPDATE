using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;

namespace CurveBuilderTool
{
    public class MarketDataBlock
    {
        public MarketDataBlock(ColumnMatrix inputMatrix)
        {
            dataMatrix = inputMatrix;

            List<String> tenorColumn = dataMatrix.values[0];

            rowIndexMap = new SortedDictionary<string, int>();
            int row = 0;
            foreach (var tenor in tenorColumn)
            {
                rowIndexMap[tenor] = row;
                row++;
            }

        }

        public VecVecString asVecVecString()
        {
            VecVecString stringMatrix = new VecVecString();
            var values = dataMatrix.values;
            int nCols = values.Count;
            if (nCols < 1)
            {
                return stringMatrix;
            }

            var col0 = values[0];
            int nRows = col0.Count;

            for (int row = 0; row < nRows; row++)
            {
                VecString rowData = new VecString();
                for (int col = 0; col < nCols; col++)
                {
                    var item = values[col][row];
                    rowData.Add(item);
                }
                stringMatrix.Add(rowData);
            }

            return stringMatrix;
        }

        public void setAsOfDate(string asOfDateString)
        {
            const string ASOFDATE = "AsOfDate";
            if (rowIndexMap.ContainsKey(ASOFDATE))
            {
                List<string> valuesColumn = dataMatrix.values[1];
                int rowIndex = rowIndexMap[ASOFDATE];
                valuesColumn[rowIndex] = asOfDateString;
            }
        }
        public void setMarketDataForTenor(string tenor, int nFields, List<string> bbgValues)
        {
            // If the tenor is not used in this market data block, skip over quietly.
            if ( this.rowIndexMap.ContainsKey(tenor))
            {
                int rowIndex = this.rowIndexMap[tenor];

                int colIndex = 1;   // skip over the tenor column
                for (int i = 0; i < nFields; i++)
                {
                    colIndex = i + 1;
                    List<String> dataColumn = dataMatrix.values[colIndex];
                    var bbgValue = bbgValues[i];
                    dataColumn[rowIndex] = bbgValue;
                }
            }
        }
        public ColumnMatrix dataMatrix { get; set; }

        // A map from tenor to row index
        private SortedDictionary<string, int> rowIndexMap;
    }


    public class MarketDataTemplate
    {
        public MarketDataTemplate(string marketDataName, string jsonFileName)
        {
            mMarketDataName = marketDataName;
            initializeFromJSonFile(jsonFileName);
        }

        private void initializeFromJSonFile(string jsonFileName)
        {
            mMarketDataBlocks = new List<MarketDataBlock>();

            dataBlockMap = new SortedDictionary<string, int>();

            string jsonText = File.ReadAllText(jsonFileName);
            JsonTextReader reader = new JsonTextReader(new StringReader(jsonText));

            int blockIndex = 0;
            while (reader.Read())
            {
                ColumnMatrix jsonMatrix = JSonUtilities.ReadJsonStringMatrix(reader);
                if (jsonMatrix.name.Length > 0)
                {
                    Console.WriteLine("Just read matrix: " + jsonMatrix.name);

                    MarketDataBlock marketDataBlock = new MarketDataBlock(jsonMatrix);
                    mMarketDataBlocks.Add(marketDataBlock);
                    dataBlockMap[jsonMatrix.name] = blockIndex;
                    blockIndex++;
                }
            }

            Console.WriteLine("Initialised MarketDataTemplate with " + mMarketDataBlocks.Count + " blocks.");
        }

        public void display()
        {
            foreach (var marketDataBlock in mMarketDataBlocks)
            {
                Console.WriteLine("Blockname: " + marketDataBlock.dataMatrix.name);
                int colIndex = 1;
                foreach (var column in marketDataBlock.dataMatrix.values)
                {
                    Console.WriteLine("COL_" + colIndex);
                    foreach (var item in column)
                    {
                        Console.WriteLine(item);
                    }

                    colIndex++;
                }
            }
        }

        public string mMarketDataName { get; set; }
        public List<MarketDataBlock> mMarketDataBlocks { get; set; }
        public SortedDictionary<string, int> dataBlockMap { get; set; }
    }
}
