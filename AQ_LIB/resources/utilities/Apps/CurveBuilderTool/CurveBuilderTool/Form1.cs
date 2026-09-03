using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace CurveBuilderTool
{
    public partial class Form1 : Form
    {

        public ConfigManager mConfigManager { get; set; }

        
        public Form1()
        {
            InitializeComponent();

            // Subscribe to the form closing event so that we can save user settings on exit
            this.FormClosing += Form1_Closing;

            string mlibqVersionInfo = MLIBQApi.initialiseMLIBQ();
            StatusLabel.Text = mlibqVersionInfo;
        }

        /* @brief This method runs when the main application window is closed
        * It stores user settings into the application config file.
        */
        private void Form1_Closing(object sender, EventArgs e)
        {
            Properties.Settings.Default.Save();

            MLIBQApi.shutdownMLIBQ();
        }

        private void ConfigTab_LoadConfig_Click(object sender, EventArgs e)
        {

            string folderPath = ConfigFolderPath.Text;
            string expandedFolderPath = Environment.ExpandEnvironmentVariables(folderPath);

            try
            {
                mConfigManager = new ConfigManager(expandedFolderPath);
            }
            catch (InvalidOperationException ex)
            {
                string message = ex.Message;
                string title = "Load Config Error";
                MessageBox.Show(message, title);
                return;
            }

            InstrumentTypesList.Text = mConfigManager.getInstrumentTypesAsString();

            updateConfigListview();

            StatusLabel.Text = "Loaded " + mConfigManager.getNumberOfInstrumentConfigs() + " instrument configs from " + expandedFolderPath;
        }

        private void updateConfigListview()
        {
            CurveConfigsListView.Items.Clear();
            CurveConfigsListView.Update();
            CurveConfigsListView.Refresh();

            string[] blankRow = { "", "", "", "", "", "" };
            foreach (var currency in mConfigManager.currencies)
            {
                List<CurveGeneratorConfig> curveGenerators = mConfigManager.getCurveGeneratorsForCurrency(currency);

                if (curveGenerators.Count == 0)
                {
                    // No curve generators defined for this currency. Just output the curve names in sorted order.
                    string blankCurveGenerator = "";

                    foreach (var curveName in mConfigManager.getCurvesForCurrency(currency))
                    {
                        string[] curveNameRow = { "", currency, curveName, blankCurveGenerator, "", "Y" };
                        var curveNameItem = new ListViewItem(curveNameRow);
                        CurveConfigsListView.Items.Add(curveNameItem);
                    }
                    var blankItem = new ListViewItem(blankRow);
                    CurveConfigsListView.Items.Add(blankItem);
                }
                else
                {
                    // Display the curve names in the order in which they appear in the CurveGenerator config
                    foreach (var curveGeneratorConfig in curveGenerators)
                    {
                        string curveName = curveGeneratorConfig.curveName;
                        var curveBuildData = mConfigManager.mCurveBuildDataCache.getCurveBuildData(currency, curveName);
                        string curveObject = (curveBuildData == null || curveBuildData.mCurveObjectHandle == null) ? "" : curveBuildData.mCurveObjectHandle;

                        string[] curveNameRow = { "", currency, curveName, curveGeneratorConfig.curveGeneratorName, curveObject, "Y" };
                        var curveNameItem = new ListViewItem(curveNameRow);
                        CurveConfigsListView.Items.Add(curveNameItem);
                    }
                    var blankItem = new ListViewItem(blankRow);
                    CurveConfigsListView.Items.Add(blankItem);
                }
            }
        }

        private void ConfigTab_RefreshMktData_Click(object sender, EventArgs e)
        {
            if (mConfigManager != null)
            {
                StatusLabel.Text = "Refreshing market data. Please wait...";

                MarketDataEngine marketDataEngine = new MarketDataEngine(mConfigManager);
                int totalSecuritiesUpdated = marketDataEngine.populateMarketDataAllCurves();
                StatusLabel.Text = "Updated market-data quotes for " + totalSecuritiesUpdated + " securities.";
            }
            else
            {
                MessageBox.Show("Please load the configuration settings first.");
            }
        }

        private void ConfigTab_SaveMktData_Click(object sender, EventArgs e)
        {
            if (mConfigManager != null)
            {
                string folderPath = OutputFolderPath.Text;
                string expandedFolderPath = Environment.ExpandEnvironmentVariables(folderPath);

                foreach (var currency in mConfigManager.currencies)
                {
                    foreach (var curveName in mConfigManager.getCurvesForCurrency(currency))
                    {
                        saveMarketDataHelper(currency, curveName, expandedFolderPath);
                    }
                }

                StatusLabel.Text = "Saved market-data to " + expandedFolderPath;
            }
            else
            {
                MessageBox.Show("Please load the configuration settings first.");
            }
        }

        private void ConfigTab_BuildCurves_Click(object sender, EventArgs e)
        {
            if (mConfigManager != null)
            {
                List<string> curveIndices = new List<string>();
                foreach (var currency in mConfigManager.currencies)
                {
                    var curveGenerators = mConfigManager.getCurveGeneratorsForCurrency(currency);
                    foreach (var curveGeneratorConfig in curveGenerators)
                    {
                        try
                        {
                            var curveName = curveGeneratorConfig.curveName;
                            string curveIndex = buildCurveHelper(currency, curveName);
                            curveIndices.Add(curveIndex);
                        }
                        catch (Exception ex)
                        {
                            string message = ex.Message;
                            message += " while building curve '" + curveGeneratorConfig.curveName + "' for currency '" + currency + "'.";
                            string title = "Curve Build";
                            MessageBox.Show(message, title);
                            return;
                        }
                    }
                }
                updateConfigListview();

                StatusLabel.Text = "Built " + curveIndices.Count + " curves.";
            }
            else
            {
                MessageBox.Show("Please load the configuration settings first.");
            }
        }

        private void ConfigTab_SaveCurves_Click(object sender, EventArgs e)
        {
            if (mConfigManager != null)
            {
                string folderPath = OutputFolderPath.Text;
                string expandedFolderPath = Environment.ExpandEnvironmentVariables(folderPath);

                foreach (var currency in mConfigManager.currencies)
                {
                    var curveGenerators = mConfigManager.getCurveGeneratorsForCurrency(currency);
                    foreach (var curveGeneratorConfig in curveGenerators)
                    {
                        try
                        {
                            var curveName = curveGeneratorConfig.curveName;
                            saveCurveHelper(currency, curveName, expandedFolderPath);
                        }
                        catch (Exception ex)
                        {
                            string message = ex.Message;
                            message += " Error occurred while saving curve '" + curveGeneratorConfig.curveName + "' for currency '" + currency + "'.";
                            string title = "Curve Build";
                            MessageBox.Show(message, title);
                            break;
                        }
                    }
                }

                StatusLabel.Text = "Saved curve objects to " + expandedFolderPath;
            }
            else
            {
                MessageBox.Show("Please load the configuration settings first.");
            }

        }


        private void ViewTab_ViewCurveDetails_Click(object sender, EventArgs e)
        {
            ViewCurveDetailsHelper();
        }

        private void ViewCurveDetailsHelper()
        {
            if (mConfigManager != null)
            {
                var selectedCurrency = CurrencyBox.Text;
                var selectedCurveName = CurveNameBox.Text;

                try
                {
                    var curveConfig = mConfigManager.getCurveConfig(selectedCurrency, selectedCurveName);
                    displayCurveConfig(curveConfig);

                    var curveBuildData = mConfigManager.mCurveBuildDataCache.getCurveBuildData(selectedCurrency, selectedCurveName);
                    displayCurveBuildData(curveBuildData);

                }
                catch (InvalidOperationException ex)
                {
                    string message = ex.Message;
                    string title = "Curve Details";
                    MessageBox.Show(message, title);
                    return;
                }
            }
            else
            {
                MessageBox.Show("Please load the configuration settings on the Config Tab.");
            }
        }

        private void displayCurveBuildData(CurveBuildData curveBuildData)
        {
            if (curveBuildData == null || curveBuildData.mCurveObjectHandle == null)
            {
                CurveObjectBox.Text = "";
            }
            else
            {
                CurveObjectBox.Text = curveBuildData.mCurveObjectHandle;
            }
        }

        private void displayCurveConfig(CurveConfig curveConfig)
        {
            if ( curveConfig.curveGenerator == null)
            {
                CurveGeneratorLabel.Text = "";
            }
            else
            {
                CurveGeneratorLabel.Text = curveConfig.curveGenerator;
            }

            CurveListView.Items.Clear();
            CurveListView.Update();
            CurveListView.Refresh();

            foreach (var keyValuePair in curveConfig.tickerConfigMap)
            {
                var instrumentType = keyValuePair.Key;

                var tickerConfig = keyValuePair.Value;
                var tickerNames = tickerConfig.mTickerNames;
                var tenors = tickerConfig.mTenors;

                for (int index = 0; index < tickerNames.Count; index++)
                {
                    var tickerName = tickerNames[index];
                    var tenor = tenors[index];
                    var bbgResponse = tickerConfig.getBbgResponseForTicker(tickerName);
                    if (bbgResponse == null)
                    {
                        string[] row = { "", instrumentType, tickerName, tenor, "-", "-", "-" };
                        var item = new ListViewItem(row);
                        CurveListView.Items.Add(item);
                    }
                    else
                    {
                        List<string> rowData = new List<string>();
                        rowData.Add("");
                        rowData.Add(instrumentType);
                        rowData.Add(tickerName);
                        rowData.Add(tenor);

                        var fields = mConfigManager.getBbgFields(instrumentType);
                        foreach (var field in fields)
                        {
                            rowData.Add(bbgResponse.fieldData[field]);
                        }
                        string[] row = rowData.ToArray();
                        var item = new ListViewItem(row);
                        CurveListView.Items.Add(item);
                    }
                }
                string[] blank = { "", "", "", "", "", "", "" };
                var blankItem = new ListViewItem(blank);
                CurveListView.Items.Add(blankItem);

            }
        }

        private void ViewTab_RefreshMktData_Click(object sender, EventArgs e)
        {
            if (mConfigManager != null)
            {
                StatusLabel.Text = "Refreshing market data. Please wait...";

                var selectedCurrency = CurrencyBox.Text;
                var selectedCurveName = CurveNameBox.Text;

                try
                {
                    var curveConfig = mConfigManager.getCurveConfig(selectedCurrency, selectedCurveName);

                    MarketDataEngine marketDataEngine = new MarketDataEngine(mConfigManager);
                    int numSecuritiesUpdated = marketDataEngine.populateMarketDataSingleCurve(curveConfig);

                    displayCurveConfig(curveConfig);

                    StatusLabel.Text = "Updated market-data quotes for " + numSecuritiesUpdated + " securities.";
                }
                catch (InvalidOperationException ex)
                {
                    string message = ex.Message;
                    string title = "Curve Details";
                    MessageBox.Show(message, title);
                    return;
                }

            }
            else
            {
                MessageBox.Show("Please load the configuration settings on the Config Tab.");
            }
        }

        private void ViewTab_SaveMktData_Click(object sender, EventArgs e)
        {
            if (mConfigManager != null)
            {
                var selectedCurrency = CurrencyBox.Text;
                var selectedCurveName = CurveNameBox.Text;

                try
                {
                    string folderPath = OutputFolderPath.Text;
                    string expandedFolderPath = Environment.ExpandEnvironmentVariables(folderPath);
                    saveMarketDataHelper(selectedCurrency, selectedCurveName, expandedFolderPath);

                    StatusLabel.Text = "Saved market-data to " + expandedFolderPath;
                }
                catch (InvalidOperationException ex)
                {
                    string message = ex.Message;
                    string title = "Curve Details";
                    MessageBox.Show(message, title);
                    return;
                }
            }
            else
            {
                MessageBox.Show("Please load the configuration settings on the Config Tab.");
            }
        }
		
        private void saveMarketDataHelper(string currency, string curveName, string expandedFolderPath)
        {
            string filename = expandedFolderPath + "\\" + currency + "_" + curveName + "_PopulatedMarketData.json";

            string marketDataObject = buildMarketDataHelper(currency, curveName);
            MLIBQApi.saveMarketDataObject(filename, marketDataObject);
        }
		
        private string buildMarketDataHelper(string currency, string curveName)
        {
            var curveConfig = mConfigManager.getCurveConfig(currency, curveName);

            string marketDataName = currency + "_" + curveName + "_MARKET_DATA";
            string marketDataObject = MLIBQApi.marketDataCreate(marketDataName, curveConfig.marketDataTemplate);

            CurveBuildData curveBuildData = new CurveBuildData(currency, curveName, marketDataObject);
            mConfigManager.mCurveBuildDataCache.setCurveBuildData(curveBuildData);

            return marketDataObject;
        }

        private void ViewTab_SaveCurve_Click(object sender, EventArgs e)
        {
            if (mConfigManager != null)
            {
                var selectedCurrency = CurrencyBox.Text;
                var selectedCurveName = CurveNameBox.Text;

                try
                {
                    string folderPath = OutputFolderPath.Text;
                    string expandedFolderPath = Environment.ExpandEnvironmentVariables(folderPath);

                    saveCurveHelper(selectedCurrency, selectedCurveName, expandedFolderPath);

                    StatusLabel.Text = "Saved curve '" + selectedCurveName + "' to " + expandedFolderPath;
                }
                catch (InvalidOperationException ex)
                {
                    string message = ex.Message;
                    string title = "Curve Details";
                    MessageBox.Show(message, title);
                    return;
                }
            }
            else
            {
                MessageBox.Show("Please load the configuration settings on the Config Tab.");
            }
        }

        private string saveCurveHelper(string selectedCurrency, string selectedCurveName, string expandedFolderPath)
        {
            var curveBuildData = mConfigManager.mCurveBuildDataCache.getCurveBuildData(selectedCurrency, selectedCurveName);

            if ( curveBuildData == null || curveBuildData.mCurveObjectHandle == null)
            {
                string message = "Curve '" + selectedCurveName + "' has not been built.";
                throw new InvalidOperationException(message);

            }
            string curveObject = curveBuildData.mCurveObjectHandle;

            string filename = expandedFolderPath + "\\" + selectedCurrency + "_" + selectedCurveName + "_CURVE.json";
            MLIBQApi.saveCurveObject(filename, curveObject);
            return filename;
        }

        private void ViewTab_BuildCurve_Click(object sender, EventArgs e)
        {
            if (mConfigManager != null)
            {
                var selectedCurrency = CurrencyBox.Text;
                var selectedCurveName = CurveNameBox.Text;

                try
                {
                    string curveIndex = buildCurveHelper(selectedCurrency, selectedCurveName);

                    var curveBuildData = mConfigManager.mCurveBuildDataCache.getCurveBuildData(selectedCurrency, selectedCurveName);
                    displayCurveBuildData(curveBuildData);

                    StatusLabel.Text = "Built curve index: " + curveIndex;
                }
                catch (Exception ex)
                {
                    string message = ex.Message;
                    string title = "Curve Details";
                    MessageBox.Show(message, title);
                    return;
                }
            }
            else
            {
                MessageBox.Show("Please load the configuration settings on the Config Tab.");
            }
        }

        private string buildCurveHelper(string selectedCurrency, string selectedCurveName)
        {
            var curveConfig = mConfigManager.getCurveConfig(selectedCurrency, selectedCurveName);

            var curveBuildData = mConfigManager.mCurveBuildDataCache.getCurveBuildData(selectedCurrency, selectedCurveName);
            if (curveBuildData == null || curveBuildData.mMarketDataObjectHandle == null)
            {
                // The market data object is not yet built for this currency / curveName. So build here.
                string marketDataObjectHandle = buildMarketDataHelper(selectedCurrency, selectedCurveName);
                curveBuildData = mConfigManager.mCurveBuildDataCache.getCurveBuildData(selectedCurrency, selectedCurveName);
            }

            // Build the curve
            var curveIndex = MLIBQApi.buildCurve(curveConfig, ref curveBuildData);
            return curveIndex;
        }

        private void CurveNameBox_KeyDown(object sender, KeyEventArgs e)
        {
            if (!(e.KeyData == Keys.Enter))  // Enter
            {
                return;
            }
            ViewCurveDetailsHelper();
        }

        private void CurveObjectBox_MouseDown(object sender, MouseEventArgs e)
        {
            TextBox txt = (TextBox)sender;
            txt.SelectAll();

            var selectedCurrency = CurrencyBox.Text;
            var selectedCurveName = CurveNameBox.Text;

            string folderPath = OutputFolderPath.Text;
            string expandedFolderPath = Environment.ExpandEnvironmentVariables(folderPath);

            string filename = expandedFolderPath + "\\" + selectedCurrency + "_" + selectedCurveName + "_CURVE.json";

            var curveObjectHandle = txt.Text;

            MLIBQApi.saveCurveObject(filename, curveObjectHandle);

            string dragDropString = "=meLWOLoad(\"" + filename + "\")";

            txt.DoDragDrop(dragDropString, DragDropEffects.Copy);
        }
    }
}
