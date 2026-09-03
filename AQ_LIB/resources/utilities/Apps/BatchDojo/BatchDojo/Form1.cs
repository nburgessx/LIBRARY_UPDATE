using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.IO;
using System.Windows.Forms;
using System.Globalization;

namespace MLibDojo
{
    public partial class Form1 : Form
    {
        private int nCommands = 15;

        public Form1()
        {
            InitializeComponent();

            // Subscribe to the form closing event so that we can save user settings on exit
            this.FormClosing += Form1_Closing;

            updateMLIBVariableLabel();
        }

        /* @brief This method runs when the main application window is closed
         * It stores user settings into the application config file.
         */
        private void Form1_Closing(object sender, EventArgs e)
        {
            // Do nothing. Previously we would save config here
            // Saving config is now handled by: buttonSaveMyConfig_Click()
        }

        private void updateStatus(string description)
        {
            DateTime localDate = DateTime.Now;
            var culture = new CultureInfo("en-GB");
            toolStripStatusRecentCommand.Text = " " + description + " ,  started at:    " + localDate.ToString(culture);  
        }

        private void runBatchFile(string description, string batchfilePath)
        {
            // Work around a problem with Environment.ExpandEnvironmentVariables()
            // where it appears to expand variables based on the Process environment,
            // not based on the user environment settings
            string mlibpath = getUserMLIBVariable();
            Environment.SetEnvironmentVariable("MLIB", mlibpath, EnvironmentVariableTarget.Process);
            
            String cmdArgs = "";

            Process process = new Process();

            string fullPath = Environment.ExpandEnvironmentVariables(batchfilePath);

            process.StartInfo.FileName = fullPath;
            process.StartInfo.Arguments = cmdArgs;
            process.StartInfo.CreateNoWindow = true;
            process.StartInfo.UseShellExecute = true;

            updateStatus(description);

            try
            {
                process.Start();
            }
            catch (Exception ex)
            {
                string output = "Unable to execute command: " + batchfilePath;
                MessageBox.Show(output);
            }

            // We do this here in case the command updated the user MLIB variable
            updateMLIBVariableLabel();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            string description = textBox1.Text;
            string batchfile = textBox2.Text;
            runBatchFile(description, batchfile);
        }
        private void button2_Click(object sender, EventArgs e)
        {
            string description = textBox3.Text;
            string batchfile = textBox4.Text;
            runBatchFile(description, batchfile);
        }
        private void button3_Click(object sender, EventArgs e)
        {
            string description = textBox5.Text;
            string batchfile = textBox6.Text;
            runBatchFile(description, batchfile);
        }
        private void button4_Click(object sender, EventArgs e)
        {
            string description = textBox7.Text;
            string batchfile = textBox8.Text;
            runBatchFile(description, batchfile);
        }
        private void button5_Click(object sender, EventArgs e)
        {
            string description = textBox9.Text;
            string batchfile = textBox10.Text;
            runBatchFile(description, batchfile);
        }
        private void button6_Click(object sender, EventArgs e)
        {
            string description = textBox11.Text;
            string batchfile = textBox12.Text;
            runBatchFile(description, batchfile);
        }
        private void button7_Click(object sender, EventArgs e)
        {
            string description = textBox13.Text;
            string batchfile = textBox14.Text;
            runBatchFile(description, batchfile);
        }
        private void button8_Click(object sender, EventArgs e)
        {
            string description = textBox15.Text;
            string batchfile = textBox16.Text;
            runBatchFile(description, batchfile);
        }
        private void button9_Click(object sender, EventArgs e)
        {
            string description = textBox17.Text;
            string batchfile = textBox18.Text;
            runBatchFile(description, batchfile);
        }
        private void button10_Click(object sender, EventArgs e)
        {
            string description = textBox19.Text;
            string batchfile = textBox20.Text;
            runBatchFile(description, batchfile);
        }
        private void button11_Click(object sender, EventArgs e)
        {
            string description = textBox21.Text;
            string batchfile = textBox22.Text;
            runBatchFile(description, batchfile);
        }
        private void button12_Click(object sender, EventArgs e)
        {
            string description = textBox23.Text;
            string batchfile = textBox24.Text;
            runBatchFile(description, batchfile);
        }
        private void button13_Click(object sender, EventArgs e)
        {
            string description = textBox25.Text;
            string batchfile = textBox26.Text;
            runBatchFile(description, batchfile);
        }
        private void button14_Click(object sender, EventArgs e)
        {
            string description = textBox27.Text;
            string batchfile = textBox28.Text;
            runBatchFile(description, batchfile);
        }
        private void button15_Click(object sender, EventArgs e)
        {
            string description = textBox29.Text;
            string batchfile = textBox30.Text;
            runBatchFile(description, batchfile);
        }

        private void buttonSaveMyConfig_Click(object sender, EventArgs e)
        {
            Properties.Settings.Default.Save();
            updateStatus("Save My Config");
        }

        string getUserMLIBVariable()
        {
            return Environment.GetEnvironmentVariable("MLIB", EnvironmentVariableTarget.User);
        }
        string getUserMLIBQVariable()
        {
            return Environment.GetEnvironmentVariable("MLIBQ", EnvironmentVariableTarget.User);
        }


        void updateMLIBVariableLabel()
        {
            string mlibpath = getUserMLIBVariable();
            labelMLIBVariable.Text = "User MLIB Variable:    " + mlibpath;

            string mlibqpath = getUserMLIBQVariable();
            labelMLIBQVariable.Text = "User MLIBQ Variable: " + mlibqpath;
        }

        private void buttonReadMLIB_Click(object sender, EventArgs e)
        {
            updateMLIBVariableLabel();
            updateStatus("Re-Read MLIB Variable");
        }

        private void buttonLoadDefault_Click(object sender, EventArgs e)
        {
            updateStatus("Load Default Config");
            string libPath = radioButtonMlib.Checked ? getUserMLIBVariable() : getUserMLIBQVariable();
            string currentDirectory = libPath + "\\resource\\utilities\\BatchFiles\\MLIBDojo";
            openFileDialog1.InitialDirectory = currentDirectory;

            openFileDialog1.DefaultExt = "txt";
            openFileDialog1.Filter = "Text Files (.txt)|*.txt|All Files|*.*";
            openFileDialog1.ValidateNames = true;

            // Set a reasonable initial filename in the dialog box
            openFileDialog1.FileName = "default_config_LN.txt";

            // Work around a .NET bug where the filename is not displayed correctly.
            // http://stackoverflow.com/questions/17163784/default-name-with-openfiledialog-c
            Timer t = new Timer();
            t.Interval = 100;
            t.Tick += (s, ee) =>
            {
                SendKeys.Send("{HOME}+{END}");
                t.Stop();
            };
            t.Start();

            string filePath;
            DialogResult result = openFileDialog1.ShowDialog();
            if (result == DialogResult.OK) // Test result.
            {
                filePath = openFileDialog1.FileName;

                var lines = File.ReadLines(filePath);

                int position = 1;
                foreach (string line in lines)
                {
                    string[] values = line.Split(',');
                    if (values.Length == 2)
                    {
                        string search = "textBox" + position;
                        TextBox tbDesc = (TextBox)this.Controls.Find(search, true)[0];
                        tbDesc.Text = values[0];

                        search = "textBox" + (position + 1);
                        TextBox tbCmd = (TextBox)this.Controls.Find(search, true)[0];
                        tbCmd.Text = values[1];

                        position += 2;
                    }
                }

                // If the loaded file does not populate all positions in the DOJO, clear the remainder
                for (int i = position; i <= nCommands * 2; i++)
                {
                    string search = "textBox" + i;
                    TextBox tbDesc = (TextBox)this.Controls.Find(search, true)[0];
                    tbDesc.Text = "";
                }
            }
        }

        private void buttonSaveDefault_Click(object sender, EventArgs e)
        {
            updateStatus("Save Default Config");

            string libPath = radioButtonMlib.Checked ? getUserMLIBVariable() : getUserMLIBQVariable();
            string currentDirectory = libPath + "\\resource\\utilities\\BatchFiles\\MLIBDojo";
            saveFileDialog1.InitialDirectory = currentDirectory;

            saveFileDialog1.DefaultExt = "txt";
            saveFileDialog1.Filter = "Text Files (.txt)|*.txt|All Files|*.*";
            saveFileDialog1.ValidateNames = true;

            string filePath;
            DialogResult result = saveFileDialog1.ShowDialog();
            if (result == DialogResult.OK) // Test result.
            {
                filePath = saveFileDialog1.FileName;

                string outputText = "";
                int position = 1;
                for (int i = 0; i < nCommands; i++)
                {
                    string search = "textBox" + position;
                    TextBox tbDesc = (TextBox)this.Controls.Find(search, true)[0];
                    outputText += tbDesc.Text + ",";

                    search = "textBox" + (position + 1);
                    TextBox tbCmd = (TextBox)this.Controls.Find(search, true)[0];
                    outputText += tbCmd.Text + Environment.NewLine;

                    position += 2;
                }

                File.WriteAllText(filePath, outputText);
            }
        }
    }
}
