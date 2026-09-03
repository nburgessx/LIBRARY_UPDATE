namespace FileSearch
{
    partial class FileRenamer
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
            this.originalFilename = new System.Windows.Forms.TextBox();
            this.newFilename = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.previewButton = new System.Windows.Forms.Button();
            this.doitButton = new System.Windows.Forms.Button();
            this.listView1 = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader4 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.filesMatched = new System.Windows.Forms.ToolStripStatusLabel();
            this.toolStripStatusLabel1 = new System.Windows.Forms.ToolStripStatusLabel();
            this.totalMatches = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // originalFilename
            // 
            this.originalFilename.Location = new System.Drawing.Point(78, 15);
            this.originalFilename.Name = "originalFilename";
            this.originalFilename.Size = new System.Drawing.Size(922, 22);
            this.originalFilename.TabIndex = 0;
            // 
            // newFilename
            // 
            this.newFilename.Location = new System.Drawing.Point(78, 46);
            this.newFilename.Name = "newFilename";
            this.newFilename.Size = new System.Drawing.Size(922, 22);
            this.newFilename.TabIndex = 1;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Lucida Sans", 9F);
            this.label1.Location = new System.Drawing.Point(8, 15);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(59, 15);
            this.label1.TabIndex = 2;
            this.label1.Text = "Original :";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Lucida Sans", 9F);
            this.label2.Location = new System.Drawing.Point(12, 46);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(42, 15);
            this.label2.TabIndex = 3;
            this.label2.Text = "New :";
            // 
            // previewButton
            // 
            this.previewButton.Font = new System.Drawing.Font("Lucida Sans", 9F);
            this.previewButton.Location = new System.Drawing.Point(78, 77);
            this.previewButton.Name = "previewButton";
            this.previewButton.Size = new System.Drawing.Size(87, 27);
            this.previewButton.TabIndex = 4;
            this.previewButton.Text = "Preview";
            this.previewButton.UseVisualStyleBackColor = true;
            this.previewButton.Click += new System.EventHandler(this.previewButton_Click);
            // 
            // doitButton
            // 
            this.doitButton.Font = new System.Drawing.Font("Lucida Sans", 9F);
            this.doitButton.Location = new System.Drawing.Point(190, 77);
            this.doitButton.Name = "doitButton";
            this.doitButton.Size = new System.Drawing.Size(87, 27);
            this.doitButton.TabIndex = 5;
            this.doitButton.Text = "Do it!";
            this.doitButton.UseVisualStyleBackColor = true;
            this.doitButton.Click += new System.EventHandler(this.doitButton_Click);
            // 
            // listView1
            // 
            this.listView1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.listView1.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2,
            this.columnHeader3,
            this.columnHeader4});
            this.listView1.Font = new System.Drawing.Font("Lucida Sans", 9F);
            this.listView1.FullRowSelect = true;
            this.listView1.GridLines = true;
            this.listView1.Location = new System.Drawing.Point(14, 110);
            this.listView1.Margin = new System.Windows.Forms.Padding(4);
            this.listView1.Name = "listView1";
            this.listView1.Size = new System.Drawing.Size(1735, 784);
            this.listView1.TabIndex = 6;
            this.listView1.UseCompatibleStateImageBehavior = false;
            this.listView1.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "FileName";
            this.columnHeader1.Width = 669;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Type";
            this.columnHeader2.Width = 81;
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "Line";
            this.columnHeader3.Width = 83;
            // 
            // columnHeader4
            // 
            this.columnHeader4.Text = "Text";
            this.columnHeader4.Width = 811;
            // 
            // statusStrip1
            // 
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.filesMatched,
            this.toolStripStatusLabel1,
            this.totalMatches});
            this.statusStrip1.Location = new System.Drawing.Point(0, 860);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Padding = new System.Windows.Forms.Padding(1, 0, 16, 0);
            this.statusStrip1.Size = new System.Drawing.Size(1564, 22);
            this.statusStrip1.TabIndex = 7;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // filesMatched
            // 
            this.filesMatched.Name = "filesMatched";
            this.filesMatched.Size = new System.Drawing.Size(42, 17);
            this.filesMatched.Text = "Files: 0";
            // 
            // toolStripStatusLabel1
            // 
            this.toolStripStatusLabel1.Name = "toolStripStatusLabel1";
            this.toolStripStatusLabel1.Size = new System.Drawing.Size(1441, 17);
            this.toolStripStatusLabel1.Spring = true;
            // 
            // totalMatches
            // 
            this.totalMatches.Name = "totalMatches";
            this.totalMatches.Size = new System.Drawing.Size(64, 17);
            this.totalMatches.Text = "Matches: 0";
            // 
            // FileRenamer
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1564, 882);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.listView1);
            this.Controls.Add(this.doitButton);
            this.Controls.Add(this.previewButton);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.newFilename);
            this.Controls.Add(this.originalFilename);
            this.Font = new System.Drawing.Font("Lucida Sans", 9F);
            this.Name = "FileRenamer";
            this.Text = "FileRenamer";
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox originalFilename;
        private System.Windows.Forms.TextBox newFilename;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button previewButton;
        private System.Windows.Forms.Button doitButton;
        private System.Windows.Forms.ListView listView1;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripStatusLabel filesMatched;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel1;
        private System.Windows.Forms.ToolStripStatusLabel totalMatches;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        private System.Windows.Forms.ColumnHeader columnHeader4;
    }
}