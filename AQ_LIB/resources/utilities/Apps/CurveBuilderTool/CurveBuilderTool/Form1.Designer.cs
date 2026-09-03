namespace CurveBuilderTool
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.TabControl = new System.Windows.Forms.TabControl();
            this.ConfigPage = new System.Windows.Forms.TabPage();
            this.CSaveCurves = new System.Windows.Forms.Button();
            this.CBuildCurves = new System.Windows.Forms.Button();
            this.CSaveMktData = new System.Windows.Forms.Button();
            this.CRefreshMktData = new System.Windows.Forms.Button();
            this.InstrumentTypesList = new System.Windows.Forms.Label();
            this.CurveConfigsListView = new System.Windows.Forms.ListView();
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.Currency = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.CurveName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.CurveGeneratorName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.Include = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.label5 = new System.Windows.Forms.Label();
            this.LoadConfig = new System.Windows.Forms.Button();
            this.OutputFolderPath = new System.Windows.Forms.TextBox();
            this.ConfigFolderPath = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.ViewPage = new System.Windows.Forms.TabPage();
            this.VSaveCurve = new System.Windows.Forms.Button();
            this.CurveGeneratorLabel = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.VBuildCurve = new System.Windows.Forms.Button();
            this.VSaveMktData = new System.Windows.Forms.Button();
            this.VRefreshMktData = new System.Windows.Forms.Button();
            this.CurveListView = new System.Windows.Forms.ListView();
            this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.InstrumentType = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.TickerName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.Tenor = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.Bbg1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.Bbg2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.Bbg3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.ViewCurveDetails = new System.Windows.Forms.Button();
            this.CurveNameBox = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.CurrencyBox = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.SwapPage = new System.Windows.Forms.TabPage();
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.StatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.label7 = new System.Windows.Forms.Label();
            this.CurveObject = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.CurveObjectBox = new System.Windows.Forms.TextBox();
            this.TabControl.SuspendLayout();
            this.ConfigPage.SuspendLayout();
            this.ViewPage.SuspendLayout();
            this.statusStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // TabControl
            // 
            this.TabControl.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.TabControl.Controls.Add(this.ConfigPage);
            this.TabControl.Controls.Add(this.ViewPage);
            this.TabControl.Controls.Add(this.SwapPage);
            this.TabControl.Font = new System.Drawing.Font("Lucida Sans", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.TabControl.Location = new System.Drawing.Point(12, 12);
            this.TabControl.Name = "TabControl";
            this.TabControl.SelectedIndex = 0;
            this.TabControl.Size = new System.Drawing.Size(1031, 825);
            this.TabControl.TabIndex = 0;
            // 
            // ConfigPage
            // 
            this.ConfigPage.Controls.Add(this.CSaveCurves);
            this.ConfigPage.Controls.Add(this.CBuildCurves);
            this.ConfigPage.Controls.Add(this.CSaveMktData);
            this.ConfigPage.Controls.Add(this.CRefreshMktData);
            this.ConfigPage.Controls.Add(this.InstrumentTypesList);
            this.ConfigPage.Controls.Add(this.CurveConfigsListView);
            this.ConfigPage.Controls.Add(this.label5);
            this.ConfigPage.Controls.Add(this.LoadConfig);
            this.ConfigPage.Controls.Add(this.OutputFolderPath);
            this.ConfigPage.Controls.Add(this.ConfigFolderPath);
            this.ConfigPage.Controls.Add(this.label2);
            this.ConfigPage.Controls.Add(this.label1);
            this.ConfigPage.Location = new System.Drawing.Point(4, 24);
            this.ConfigPage.Name = "ConfigPage";
            this.ConfigPage.Padding = new System.Windows.Forms.Padding(3);
            this.ConfigPage.Size = new System.Drawing.Size(1023, 797);
            this.ConfigPage.TabIndex = 0;
            this.ConfigPage.Text = "  Config  ";
            this.ConfigPage.UseVisualStyleBackColor = true;
            // 
            // CSaveCurves
            // 
            this.CSaveCurves.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.CSaveCurves.Location = new System.Drawing.Point(370, 757);
            this.CSaveCurves.Name = "CSaveCurves";
            this.CSaveCurves.Size = new System.Drawing.Size(106, 23);
            this.CSaveCurves.TabIndex = 11;
            this.CSaveCurves.Text = "Save Curves";
            this.CSaveCurves.UseVisualStyleBackColor = true;
            this.CSaveCurves.Click += new System.EventHandler(this.ConfigTab_SaveCurves_Click);
            // 
            // CBuildCurves
            // 
            this.CBuildCurves.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.CBuildCurves.Location = new System.Drawing.Point(258, 757);
            this.CBuildCurves.Name = "CBuildCurves";
            this.CBuildCurves.Size = new System.Drawing.Size(106, 23);
            this.CBuildCurves.TabIndex = 10;
            this.CBuildCurves.Text = "Build Curves";
            this.CBuildCurves.UseVisualStyleBackColor = true;
            this.CBuildCurves.Click += new System.EventHandler(this.ConfigTab_BuildCurves_Click);
            // 
            // CSaveMktData
            // 
            this.CSaveMktData.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.CSaveMktData.Location = new System.Drawing.Point(129, 757);
            this.CSaveMktData.Name = "CSaveMktData";
            this.CSaveMktData.Size = new System.Drawing.Size(108, 23);
            this.CSaveMktData.TabIndex = 9;
            this.CSaveMktData.Text = "Save MktData";
            this.CSaveMktData.UseVisualStyleBackColor = true;
            this.CSaveMktData.Click += new System.EventHandler(this.ConfigTab_SaveMktData_Click);
            // 
            // CRefreshMktData
            // 
            this.CRefreshMktData.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.CRefreshMktData.Location = new System.Drawing.Point(9, 757);
            this.CRefreshMktData.Name = "CRefreshMktData";
            this.CRefreshMktData.Size = new System.Drawing.Size(114, 23);
            this.CRefreshMktData.TabIndex = 8;
            this.CRefreshMktData.Text = "Refresh MktData";
            this.CRefreshMktData.UseVisualStyleBackColor = true;
            this.CRefreshMktData.Click += new System.EventHandler(this.ConfigTab_RefreshMktData_Click);
            // 
            // InstrumentTypesList
            // 
            this.InstrumentTypesList.AutoSize = true;
            this.InstrumentTypesList.Location = new System.Drawing.Point(114, 88);
            this.InstrumentTypesList.Name = "InstrumentTypesList";
            this.InstrumentTypesList.Size = new System.Drawing.Size(123, 15);
            this.InstrumentTypesList.TabIndex = 7;
            this.InstrumentTypesList.Text = "                             ";
            // 
            // CurveConfigsListView
            // 
            this.CurveConfigsListView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.CurveConfigsListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader2,
            this.Currency,
            this.CurveName,
            this.CurveGeneratorName,
            this.CurveObject,
            this.Include});
            this.CurveConfigsListView.FullRowSelect = true;
            this.CurveConfigsListView.GridLines = true;
            this.CurveConfigsListView.Location = new System.Drawing.Point(9, 126);
            this.CurveConfigsListView.Name = "CurveConfigsListView";
            this.CurveConfigsListView.Size = new System.Drawing.Size(1008, 616);
            this.CurveConfigsListView.TabIndex = 6;
            this.CurveConfigsListView.UseCompatibleStateImageBehavior = false;
            this.CurveConfigsListView.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader2
            // 
            this.columnHeader2.DisplayIndex = 5;
            this.columnHeader2.Text = "";
            this.columnHeader2.Width = 35;
            // 
            // Currency
            // 
            this.Currency.DisplayIndex = 0;
            this.Currency.Text = "Currency";
            this.Currency.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.Currency.Width = 150;
            // 
            // CurveName
            // 
            this.CurveName.DisplayIndex = 1;
            this.CurveName.Text = "Curve Name";
            this.CurveName.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.CurveName.Width = 260;
            // 
            // CurveGeneratorName
            // 
            this.CurveGeneratorName.DisplayIndex = 2;
            this.CurveGeneratorName.Text = "CurveGenerator";
            this.CurveGeneratorName.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.CurveGeneratorName.Width = 260;
            // 
            // Include
            // 
            this.Include.DisplayIndex = 4;
            this.Include.Text = "Include";
            this.Include.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Font = new System.Drawing.Font("Lucida Sans", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label5.Location = new System.Drawing.Point(9, 88);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(106, 15);
            this.label5.TabIndex = 5;
            this.label5.Text = "Intrument Types:";
            // 
            // LoadConfig
            // 
            this.LoadConfig.Location = new System.Drawing.Point(855, 24);
            this.LoadConfig.Name = "LoadConfig";
            this.LoadConfig.Size = new System.Drawing.Size(75, 23);
            this.LoadConfig.TabIndex = 4;
            this.LoadConfig.Text = "Load Config";
            this.LoadConfig.UseVisualStyleBackColor = true;
            this.LoadConfig.Click += new System.EventHandler(this.ConfigTab_LoadConfig_Click);
            // 
            // OutputFolderPath
            // 
            this.OutputFolderPath.DataBindings.Add(new System.Windows.Forms.Binding("Text", global::CurveBuilderTool.Properties.Settings.Default, "OutputFolder", true, System.Windows.Forms.DataSourceUpdateMode.OnPropertyChanged));
            this.OutputFolderPath.Location = new System.Drawing.Point(114, 50);
            this.OutputFolderPath.Name = "OutputFolderPath";
            this.OutputFolderPath.Size = new System.Drawing.Size(704, 22);
            this.OutputFolderPath.TabIndex = 3;
            this.OutputFolderPath.Text = global::CurveBuilderTool.Properties.Settings.Default.OutputFolder;
            // 
            // ConfigFolderPath
            // 
            this.ConfigFolderPath.DataBindings.Add(new System.Windows.Forms.Binding("Text", global::CurveBuilderTool.Properties.Settings.Default, "ConfigFolder", true, System.Windows.Forms.DataSourceUpdateMode.OnPropertyChanged));
            this.ConfigFolderPath.Location = new System.Drawing.Point(114, 24);
            this.ConfigFolderPath.Name = "ConfigFolderPath";
            this.ConfigFolderPath.Size = new System.Drawing.Size(704, 22);
            this.ConfigFolderPath.TabIndex = 2;
            this.ConfigFolderPath.Text = global::CurveBuilderTool.Properties.Settings.Default.ConfigFolder;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Lucida Sans", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label2.Location = new System.Drawing.Point(6, 50);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(93, 15);
            this.label2.TabIndex = 1;
            this.label2.Text = "Output Folder:";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Lucida Sans", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(6, 24);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(90, 15);
            this.label1.TabIndex = 0;
            this.label1.Text = "Config Folder:";
            // 
            // ViewPage
            // 
            this.ViewPage.Controls.Add(this.CurveObjectBox);
            this.ViewPage.Controls.Add(this.label7);
            this.ViewPage.Controls.Add(this.VSaveCurve);
            this.ViewPage.Controls.Add(this.CurveGeneratorLabel);
            this.ViewPage.Controls.Add(this.label6);
            this.ViewPage.Controls.Add(this.VBuildCurve);
            this.ViewPage.Controls.Add(this.VSaveMktData);
            this.ViewPage.Controls.Add(this.VRefreshMktData);
            this.ViewPage.Controls.Add(this.CurveListView);
            this.ViewPage.Controls.Add(this.ViewCurveDetails);
            this.ViewPage.Controls.Add(this.CurveNameBox);
            this.ViewPage.Controls.Add(this.label4);
            this.ViewPage.Controls.Add(this.CurrencyBox);
            this.ViewPage.Controls.Add(this.label3);
            this.ViewPage.Location = new System.Drawing.Point(4, 24);
            this.ViewPage.Name = "ViewPage";
            this.ViewPage.Padding = new System.Windows.Forms.Padding(3);
            this.ViewPage.Size = new System.Drawing.Size(1023, 797);
            this.ViewPage.TabIndex = 1;
            this.ViewPage.Text = "View Curve";
            this.ViewPage.UseVisualStyleBackColor = true;
            // 
            // VSaveCurve
            // 
            this.VSaveCurve.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.VSaveCurve.Location = new System.Drawing.Point(369, 758);
            this.VSaveCurve.Name = "VSaveCurve";
            this.VSaveCurve.Size = new System.Drawing.Size(108, 23);
            this.VSaveCurve.TabIndex = 14;
            this.VSaveCurve.Text = "Save Curve";
            this.VSaveCurve.UseVisualStyleBackColor = true;
            this.VSaveCurve.Click += new System.EventHandler(this.ViewTab_SaveCurve_Click);
            // 
            // CurveGeneratorLabel
            // 
            this.CurveGeneratorLabel.AutoSize = true;
            this.CurveGeneratorLabel.Location = new System.Drawing.Point(118, 87);
            this.CurveGeneratorLabel.Name = "CurveGeneratorLabel";
            this.CurveGeneratorLabel.Size = new System.Drawing.Size(123, 15);
            this.CurveGeneratorLabel.TabIndex = 13;
            this.CurveGeneratorLabel.Text = "                             ";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Font = new System.Drawing.Font("Lucida Sans", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label6.Location = new System.Drawing.Point(6, 87);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(106, 15);
            this.label6.TabIndex = 12;
            this.label6.Text = "Curve Generator:";
            // 
            // VBuildCurve
            // 
            this.VBuildCurve.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.VBuildCurve.Location = new System.Drawing.Point(255, 758);
            this.VBuildCurve.Name = "VBuildCurve";
            this.VBuildCurve.Size = new System.Drawing.Size(108, 23);
            this.VBuildCurve.TabIndex = 11;
            this.VBuildCurve.Text = "Build Curve";
            this.VBuildCurve.UseVisualStyleBackColor = true;
            this.VBuildCurve.Click += new System.EventHandler(this.ViewTab_BuildCurve_Click);
            // 
            // VSaveMktData
            // 
            this.VSaveMktData.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.VSaveMktData.Location = new System.Drawing.Point(134, 758);
            this.VSaveMktData.Name = "VSaveMktData";
            this.VSaveMktData.Size = new System.Drawing.Size(107, 23);
            this.VSaveMktData.TabIndex = 10;
            this.VSaveMktData.Text = "Save MktData";
            this.VSaveMktData.UseVisualStyleBackColor = true;
            this.VSaveMktData.Click += new System.EventHandler(this.ViewTab_SaveMktData_Click);
            // 
            // VRefreshMktData
            // 
            this.VRefreshMktData.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.VRefreshMktData.Location = new System.Drawing.Point(9, 758);
            this.VRefreshMktData.Name = "VRefreshMktData";
            this.VRefreshMktData.Size = new System.Drawing.Size(120, 23);
            this.VRefreshMktData.TabIndex = 9;
            this.VRefreshMktData.Text = "Refresh MktData";
            this.VRefreshMktData.UseVisualStyleBackColor = true;
            this.VRefreshMktData.Click += new System.EventHandler(this.ViewTab_RefreshMktData_Click);
            // 
            // CurveListView
            // 
            this.CurveListView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.CurveListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader3,
            this.InstrumentType,
            this.TickerName,
            this.Tenor,
            this.Bbg1,
            this.Bbg2,
            this.Bbg3});
            this.CurveListView.FullRowSelect = true;
            this.CurveListView.GridLines = true;
            this.CurveListView.Location = new System.Drawing.Point(9, 154);
            this.CurveListView.Name = "CurveListView";
            this.CurveListView.Size = new System.Drawing.Size(1000, 587);
            this.CurveListView.TabIndex = 5;
            this.CurveListView.UseCompatibleStateImageBehavior = false;
            this.CurveListView.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader3
            // 
            this.columnHeader3.DisplayIndex = 6;
            this.columnHeader3.Text = "";
            this.columnHeader3.Width = 61;
            // 
            // InstrumentType
            // 
            this.InstrumentType.DisplayIndex = 0;
            this.InstrumentType.Text = "Instrument Type";
            this.InstrumentType.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.InstrumentType.Width = 200;
            // 
            // TickerName
            // 
            this.TickerName.DisplayIndex = 1;
            this.TickerName.Text = "Ticker Name";
            this.TickerName.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.TickerName.Width = 260;
            // 
            // Tenor
            // 
            this.Tenor.DisplayIndex = 2;
            this.Tenor.Text = "Tenor";
            this.Tenor.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.Tenor.Width = 200;
            // 
            // Bbg1
            // 
            this.Bbg1.DisplayIndex = 3;
            this.Bbg1.Text = "";
            this.Bbg1.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.Bbg1.Width = 88;
            // 
            // Bbg2
            // 
            this.Bbg2.DisplayIndex = 4;
            this.Bbg2.Text = "";
            this.Bbg2.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.Bbg2.Width = 88;
            // 
            // Bbg3
            // 
            this.Bbg3.DisplayIndex = 5;
            this.Bbg3.Text = "";
            this.Bbg3.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.Bbg3.Width = 88;
            // 
            // ViewCurveDetails
            // 
            this.ViewCurveDetails.Location = new System.Drawing.Point(288, 43);
            this.ViewCurveDetails.Name = "ViewCurveDetails";
            this.ViewCurveDetails.Size = new System.Drawing.Size(75, 23);
            this.ViewCurveDetails.TabIndex = 4;
            this.ViewCurveDetails.Text = "View Curve";
            this.ViewCurveDetails.UseVisualStyleBackColor = true;
            this.ViewCurveDetails.Click += new System.EventHandler(this.ViewTab_ViewCurveDetails_Click);
            // 
            // CurveNameBox
            // 
            this.CurveNameBox.DataBindings.Add(new System.Windows.Forms.Binding("Text", global::CurveBuilderTool.Properties.Settings.Default, "SelectedCurveName", true, System.Windows.Forms.DataSourceUpdateMode.OnPropertyChanged));
            this.CurveNameBox.Location = new System.Drawing.Point(121, 44);
            this.CurveNameBox.Name = "CurveNameBox";
            this.CurveNameBox.Size = new System.Drawing.Size(161, 22);
            this.CurveNameBox.TabIndex = 3;
            this.CurveNameBox.Text = global::CurveBuilderTool.Properties.Settings.Default.SelectedCurveName;
            this.CurveNameBox.KeyDown += new System.Windows.Forms.KeyEventHandler(this.CurveNameBox_KeyDown);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Font = new System.Drawing.Font("Lucida Sans", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label4.Location = new System.Drawing.Point(6, 47);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(82, 15);
            this.label4.TabIndex = 2;
            this.label4.Text = "Curve Name:";
            // 
            // CurrencyBox
            // 
            this.CurrencyBox.DataBindings.Add(new System.Windows.Forms.Binding("Text", global::CurveBuilderTool.Properties.Settings.Default, "SelectedCurrency", true, System.Windows.Forms.DataSourceUpdateMode.OnPropertyChanged));
            this.CurrencyBox.Location = new System.Drawing.Point(121, 16);
            this.CurrencyBox.Name = "CurrencyBox";
            this.CurrencyBox.Size = new System.Drawing.Size(161, 22);
            this.CurrencyBox.TabIndex = 1;
            this.CurrencyBox.Text = global::CurveBuilderTool.Properties.Settings.Default.SelectedCurrency;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("Lucida Sans", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label3.Location = new System.Drawing.Point(6, 16);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(62, 15);
            this.label3.TabIndex = 0;
            this.label3.Text = "Currency:";
            // 
            // SwapPage
            // 
            this.SwapPage.Location = new System.Drawing.Point(4, 24);
            this.SwapPage.Name = "SwapPage";
            this.SwapPage.Padding = new System.Windows.Forms.Padding(3);
            this.SwapPage.Size = new System.Drawing.Size(1023, 797);
            this.SwapPage.TabIndex = 2;
            this.SwapPage.Text = "  Swap  ";
            this.SwapPage.UseVisualStyleBackColor = true;
            // 
            // statusStrip1
            // 
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.StatusLabel});
            this.statusStrip1.Location = new System.Drawing.Point(0, 840);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(1055, 22);
            this.statusStrip1.TabIndex = 1;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // StatusLabel
            // 
            this.StatusLabel.Name = "StatusLabel";
            this.StatusLabel.Size = new System.Drawing.Size(17, 17);
            this.StatusLabel.Text = "\"\"";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Font = new System.Drawing.Font("Lucida Sans", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label7.Location = new System.Drawing.Point(6, 116);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(88, 15);
            this.label7.TabIndex = 15;
            this.label7.Text = "Curve Object:";
            // 
            // CurveObject
            // 
            this.CurveObject.DisplayIndex = 3;
            this.CurveObject.Text = "Curve Object";
            this.CurveObject.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.CurveObject.Width = 260;
            // 
            // CurveObjectBox
            // 
            this.CurveObjectBox.AllowDrop = true;
            this.CurveObjectBox.Location = new System.Drawing.Point(121, 113);
            this.CurveObjectBox.Name = "CurveObjectBox";
            this.CurveObjectBox.ReadOnly = true;
            this.CurveObjectBox.Size = new System.Drawing.Size(196, 22);
            this.CurveObjectBox.TabIndex = 17;
            this.CurveObjectBox.MouseDown += new System.Windows.Forms.MouseEventHandler(this.CurveObjectBox_MouseDown);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1055, 862);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.TabControl);
            this.Name = "Form1";
            this.Text = "CurveBuilderTool";
            this.TabControl.ResumeLayout(false);
            this.ConfigPage.ResumeLayout(false);
            this.ConfigPage.PerformLayout();
            this.ViewPage.ResumeLayout(false);
            this.ViewPage.PerformLayout();
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TabControl TabControl;
        private System.Windows.Forms.TabPage ConfigPage;
        private System.Windows.Forms.TabPage ViewPage;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripStatusLabel StatusLabel;
        private System.Windows.Forms.TextBox OutputFolderPath;
        private System.Windows.Forms.TextBox ConfigFolderPath;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button LoadConfig;
        private System.Windows.Forms.TabPage SwapPage;
        private System.Windows.Forms.ListView CurveListView;
        private System.Windows.Forms.ColumnHeader InstrumentType;
        private System.Windows.Forms.ColumnHeader TickerName;
        private System.Windows.Forms.ColumnHeader Tenor;
        private System.Windows.Forms.ColumnHeader Bbg1;
        private System.Windows.Forms.ColumnHeader Bbg2;
        private System.Windows.Forms.ColumnHeader Bbg3;
        private System.Windows.Forms.Button ViewCurveDetails;
        private System.Windows.Forms.TextBox CurveNameBox;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox CurrencyBox;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label InstrumentTypesList;
        private System.Windows.Forms.ListView CurveConfigsListView;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ColumnHeader Currency;
        private System.Windows.Forms.ColumnHeader CurveName;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        private System.Windows.Forms.ColumnHeader Include;
        private System.Windows.Forms.Button CBuildCurves;
        private System.Windows.Forms.Button CSaveMktData;
        private System.Windows.Forms.Button CRefreshMktData;
        private System.Windows.Forms.Button VBuildCurve;
        private System.Windows.Forms.Button VSaveMktData;
        private System.Windows.Forms.Button VRefreshMktData;
        private System.Windows.Forms.ColumnHeader CurveGeneratorName;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label CurveGeneratorLabel;
        private System.Windows.Forms.Button VSaveCurve;
        private System.Windows.Forms.Button CSaveCurves;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.ColumnHeader CurveObject;
        private System.Windows.Forms.TextBox CurveObjectBox;
    }
}

