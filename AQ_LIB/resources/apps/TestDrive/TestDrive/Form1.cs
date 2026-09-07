using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;


/* @brief TestDrive, a tool for Quants which allows the library tests to run graphically in parallel
 * @author Ian Castleton
 * @date 13 December 2016
 */
namespace TestDrive
{
    public partial class Form1 : Form
    {
        private string testVisualStudio_;   // Which Visual Studio was used to build Google_test? 2022
        private string testArchitecture_;   // Win32 or x64
        private string testBuildType_;      // Debug, Release, LeakCheck

        private int numberOfTestsQueued_ = 0;
        private int numberOfTestsRunning_ = 0;
        private int numberOfTestsFailed_ = 0;
        private int numberOfTestsPassed_ = 0;
        private int numberOfTestsWarning_ = 0;
        private int numberOfTestsRebased_ = 0;

        private Stopwatch stopwatch_;

        private ConcurrentDictionary<Process, int> runningProcesses = new ConcurrentDictionary<Process, int>();

        private List< string[] > backingStore_;

        public Form1()
        {
            InitializeComponent();

            // Use the current application icon for the window title bar / taskbar.
            this.Icon = TestDrive.Properties.Resources.testdrive;

            // Initialize the UI from the settings stored in the application config file.
            comboBox1.SelectedItem = TestDrive.Properties.Settings.Default.Architecture;
            comboBox2.SelectedItem = TestDrive.Properties.Settings.Default.BuildType;
            comboBox3.SelectedItem = TestDrive.Properties.Settings.Default.VisualStudio;

            // Subscribe to the form closing event so that we can save user settings on exit
            this.FormClosing += Form1_Closing;

            // Use Double-Buffering to remove the flickering in the listview
            ControlExtensions.DoubleBuffering(listView1, true);

            updateAQVariableLabel();

            // Work around a problem with Environment.ExpandEnvironmentVariables()
            // where it appears to expand variables based on the Process environment,
            // not based on the user environment settings
            string aqpath = getUserAQVariable();
            Environment.SetEnvironmentVariable("AQ", aqpath, EnvironmentVariableTarget.Process);
        }

        /* @brief This method runs when the main application window is closed
         * It stores user settings into the application config file.
         */
        private void Form1_Closing(object sender, EventArgs e)
        {
            TestDrive.Properties.Settings.Default.Architecture = comboBox1.Text;
            TestDrive.Properties.Settings.Default.BuildType = comboBox2.Text;
            TestDrive.Properties.Settings.Default.VisualStudio = comboBox3.Text;
            TestDrive.Properties.Settings.Default.Save();

            // On exit, attempt to cancel any tests that are still running
            foreach ( var processItem in runningProcesses)
            {
                try
                {
                    Process process = processItem.Key;
                    process.Kill();
                }
                catch ( Exception )
                {
                    // The process might already have ended. Continue with best effort
                    continue;
                }
            }
            
        }

        private string getUserAQVariable()
        {
            return Environment.GetEnvironmentVariable("AQ", EnvironmentVariableTarget.User);
        }

        private void updateAQVariableLabel()
        {
            string aqpath = getUserAQVariable();
            labelAQVariable.Text = "User AQ Variable:     " + aqpath;

        }

        private int processThreadsInputParameter()
        {
            // Try to read the "Threads" user input setting.
            // If we cannot read an valid integer, set the default number of threads to be
            // the number of logical CPU cores on this machine
            int numberOfThreadsToUse = Utilities.getNumThreadsToUse(textBox2.Text);
            textBox2.Text = "" + numberOfThreadsToUse;

            return numberOfThreadsToUse;
        }

        private void resetTestStatisticsAndUpdateStatusBar()
        {
            // Parse the architecture and build user settings
            testVisualStudio_ = comboBox3.Text;
            testArchitecture_ = comboBox1.Text;
            testBuildType_ = comboBox2.Text;

            numberOfTestsPassed_ = 0;
            numberOfTestsFailed_ = 0;
            numberOfTestsWarning_ = 0;

            toolStripStatusLabel5.Text = "Passed: " + numberOfTestsPassed_;
            toolStripStatusLabel4.Text = "Failed: " + numberOfTestsFailed_;
            toolStripStatusLabel8.Text = "Warning: " + numberOfTestsFailed_;

            toolStripStatusLabel9.Text = "Time: ";
        }

        /* @brief Runs the specified shell command and collects the output and exitCode
         *        The exitCode is used to check if the test has passed or failed.
         * @param[in] command       The full path to an exe to run, excluding any command line arguments
         * @param[in] commandArg    The command line arguments to pass
         * @param[out] output       The output generated by the command
         * @param[out] Returns the exitCode of the command
         */
        private int runExternalCommand(string command, string commandArg, bool debugRequested, out string output)
        {
            Console.WriteLine("About to run: " + command + " " + commandArg);

            Process process = new Process();
            process.StartInfo.FileName = command;
            process.StartInfo.Arguments = commandArg;
            process.StartInfo.CreateNoWindow = true;
            process.StartInfo.UseShellExecute = false;

            process.StartInfo.RedirectStandardOutput = true;
            process.StartInfo.RedirectStandardError = true;
            process.StartInfo.RedirectStandardInput = true;

            try
            {
                process.Start();
                runningProcesses.TryAdd( process, 0); // Keep track of all running processes. The int is just a dummy value
            }
            catch (Exception)
            {
                output = "Unable to execute command: " + command;
                MessageBox.Show(output);
                return -1;
            }

            if (debugRequested)
            {
                MessageBox.Show("Attach your debugger to the GOOGLE_TEST.EXE with process id [ " + process.Id + " ] and then click OK");
                process.StandardInput.WriteLine();
            }

            // Send a return character which will un-pause the executable, in case it is waiting for user input
            process.StandardInput.WriteLine(); 

            // Read all the output generated by the command
            output = process.StandardOutput.ReadToEnd();

            // Wait until the process exits and capture the exitCode
            process.WaitForExit();
            int exitCode = process.ExitCode;
            process.Close();

            // Remove the process from our collection of running processes
            int dummy;
            runningProcesses.TryRemove(process, out dummy);

            // Any memory leaks will be considered a test failure
            if (output.Contains("detected memory leaks"))
            {
                exitCode = 2;
            }
            if (output.Contains("#Warning"))
            {
                exitCode = 3;
            }

            return exitCode;
        }

        /* @brief Constructs the command line and arguments that are used to run a test
         *        This function is invoked by a thread pool
         * @param[in] obj   The context information provided by the threadpool. In this case, it is the ListViewItem of the selected test
         * @param[out]      Returns the exitCode from running the test
         */
        private int processTest(Object obj, bool rebaseRequested, bool debugRequested, bool leakCheckRequested, out string output )
        {
            string command = Utilities.constructPathToTestExecutable(textBox1.Text, testVisualStudio_, testArchitecture_, testBuildType_);

            // Construct the argument to google_test, which is a filter containing the selected test
            ListViewItem row = obj as ListViewItem;
            string commandArg = Utilities.constructGTestFilter(row);
            if (rebaseRequested)
            {
                commandArg += " --rebase";
            }
            if (debugRequested)
            {
                commandArg += " --debug";
            }

            if (leakCheckRequested || testBuildType_.Equals("LeakCheck", StringComparison.OrdinalIgnoreCase))
            {
                commandArg += " --leakcheck";
                string buildType = "LeakCheck"; // Need to use the GOOGLE_TEST build which has VLD support compiled in
                command = Utilities.constructPathToTestExecutable(textBox1.Text, testVisualStudio_, testArchitecture_, buildType);
            }

            // Actually run the test
            int exitCode = runExternalCommand(command, commandArg, debugRequested, out output);
            return exitCode;
        }

        private void queueUpAndRunTests(int numberOfThreadsToUse, List<ListViewItem> rowsToProcess, bool rebaseRequested, bool debugRequested, bool leakCheckRequested)
        {
            // Start the stopwatch
            stopwatch_ = new Stopwatch();
            stopwatch_.Start();


            // Queue up / Schedule the tests and instruct the thread pool to run them in parallel
            // Update the status bar with the number of scheduled tests

            // Mark the tests as queued
            Interlocked.Add(ref numberOfTestsQueued_, rowsToProcess.Count);
            toolStripStatusLabel2.Text = "Queued: " + numberOfTestsQueued_;

            foreach (ListViewItem itemRow in rowsToProcess)
            {
                itemRow.BackColor = Color.DarkTurquoise;
            }

            // Now submit all the jobs to the thread pool
            // This partitioner performs perfect load-balancing across all available threads
            var customPartitioner = new SingleItemIListPartitioner<ListViewItem>(rowsToProcess);
            Task.Factory.StartNew(() =>            // Submit the tests on a separate thread, to keep the UI responsive
                {
                    Parallel.ForEach(customPartitioner,    // Run the tests in parallel...
                                 new ParallelOptions { MaxDegreeOfParallelism = numberOfThreadsToUse },   // ... specifying a maximum number of worker threads
                                 (row) =>
                                 {
                                     Interlocked.Decrement(ref numberOfTestsQueued_);    // Start work on a test. Atomic decrement the count of queued tests
                                     Interlocked.Increment(ref numberOfTestsRunning_);   // Atomic increment the count of running tests
                                     this.Invoke(new Action(() =>
                                     {
                                         row.SubItems[3].Text = "RUNNING";               // Update the UI to show this test running. Use "Invoke" to send message back to UI thread
                                         row.BackColor = Color.Cyan;

                                         toolStripStatusLabel2.Text = "Queued: " + numberOfTestsQueued_;     // Update the status bar information
                                         toolStripStatusLabel3.Text = "Running: " + numberOfTestsRunning_;
                                     }));

                                     // Start a timer and run the test, rounding the executation time to 1dp
                                     var watch = System.Diagnostics.Stopwatch.StartNew();

                                     // *** HERE IS WHERE WE ACTUALLY RUN THE TEST ON A THREAD ***
                                     string output;
                                     int exitCode = processTest(row, rebaseRequested, debugRequested, leakCheckRequested, out output);
                                     // ***  

                                     watch.Stop();
                                     var elapsedMs = watch.ElapsedMilliseconds;
                                     double elapsedSec = elapsedMs / 1000.0;
                                     double roundedSec = Math.Round(elapsedSec, 1);

                                     Interlocked.Decrement(ref numberOfTestsRunning_);  // After the test is complete, atomic decrement the count of running tests

                                     // Decide if test passed of failed, and update the UI.
                                     // As we are on a worker thread, use "Invoke" to message back to the UI thread

                                     string result;
                                     if (exitCode == 0)
                                     {
                                         result = "PASS";
                                         // The test passed! Use "Invoke" to message back to the UI thread
                                         this.Invoke(new Action(() =>
                                         {
                                             row.SubItems[3].Text = result;
                                             row.BackColor = Color.LightGreen;
                                             Interlocked.Increment(ref numberOfTestsPassed_);
                                             toolStripStatusLabel3.Text = "Running: " + numberOfTestsRunning_;
                                             toolStripStatusLabel5.Text = "Passed: " + numberOfTestsPassed_;

                                             if (rebaseRequested)
                                             {
                                                 Interlocked.Increment(ref numberOfTestsRebased_);
                                                 toolStripStatusLabel7.Text = "Total Rebased: " + numberOfTestsRebased_;
                                             }
                                         }));
                                     }
                                     else if (exitCode == 3)
                                     {
                                         result = "WARNING";
                                         // The test passed, but warnings were detected. Use "Invoke" to message back to the UI thread
                                         this.Invoke(new Action(() =>
                                         {
                                             row.SubItems[3].Text = result;
                                             row.BackColor = Color.Yellow;
                                             Interlocked.Increment(ref numberOfTestsPassed_);
                                             Interlocked.Increment(ref numberOfTestsWarning_);
                                             toolStripStatusLabel3.Text = "Running: " + numberOfTestsRunning_;
                                             toolStripStatusLabel5.Text = "Passed: " + numberOfTestsPassed_;
                                             toolStripStatusLabel8.Text = "Warning: " + numberOfTestsWarning_;

                                             // Write the output from the test to a file, so the warning can be viewed later
                                             string testFailureLogPath = Utilities.getFailureLogPath(textBox6.Text, testArchitecture_, testBuildType_, row);
                                             Utilities.LogWrite(testFailureLogPath, output);

                                             if (rebaseRequested)
                                             {
                                                 Interlocked.Increment(ref numberOfTestsRebased_);
                                                 toolStripStatusLabel7.Text = "Total Rebased: " + numberOfTestsRebased_;
                                             }
                                         }));
                                     }
                                     else
                                     {
                                         result = "FAIL";
                                         this.Invoke(new Action(() =>
                                         {
                                             row.SubItems[3].Text = result;
                                             row.BackColor = Color.OrangeRed;
                                             Interlocked.Increment(ref numberOfTestsFailed_);
                                             toolStripStatusLabel3.Text = "Running: " + numberOfTestsRunning_;
                                             toolStripStatusLabel4.Text = "Failed: " + numberOfTestsFailed_;

                                             // Write the output from the failed test to a file
                                             string testFailureLogPath = Utilities.getFailureLogPath(textBox6.Text, testArchitecture_, testBuildType_, row);
                                             Utilities.LogWrite(testFailureLogPath, output);
                                         }));
                                     }

                                     // Finally display the execution time of this test (actual wall-clock time).
                                     this.Invoke(new Action(() =>
                                     {
                                         row.SubItems[4].Text = "" + roundedSec;

                                         if (numberOfTestsQueued_ == 0 && numberOfTestsRunning_ == 0)
                                         {
                                             stopwatch_.Stop();
                                             var totalElapsedMs = stopwatch_.ElapsedMilliseconds;
                                             double totalElapsedSec = totalElapsedMs / 1000.0;
                                             double totalRoundedSec = Math.Round(totalElapsedSec, 1);
                                             toolStripStatusLabel9.Text = "Time: " + totalRoundedSec + " s";
                                         }

                                     }));

                                     // Update backing store with the test result and calc time
                                     int testNumber;
                                     Int32.TryParse(row.SubItems[0].Text, out testNumber);
                                     int idx = testNumber - 1;
                                     backingStore_.ElementAt(idx)[3] = result;
                                     backingStore_.ElementAt(idx)[4] = "" + roundedSec;
                                 });

                                 // Write the summary list of test failures to a file
                                string testFailuresTxt = "";
                                foreach (ListViewItem itemRow in rowsToProcess)
                                {
                                    if (itemRow.SubItems[3].Text == "FAIL")
                                    {
                                        testFailuresTxt += itemRow.SubItems[0].Text + "   " + itemRow.SubItems[1].Text + "." + itemRow.SubItems[2].Text + System.Environment.NewLine;
                                    }
                                }

                                string summaryLogPath = Utilities.getSummaryLogFilePath(textBox6.Text, testArchitecture_, testBuildType_);
                                Utilities.LogWrite(summaryLogPath, testFailuresTxt);
                }
            );

        }

        // @brief This function contains the common functionality used to handle the "Run" and "Debug" button clicks
        //        It determines which tests to execute based on user selection, and then adds the test to the queue.
        private void RunAndDebugButtonHandler(bool rebaseRequested, bool debugRequested, bool leakCheckRequested)
        {
            resetTestStatisticsAndUpdateStatusBar();
            int numberOfThreadsToUse = processThreadsInputParameter();

            // Determine which tests to run: Either run the selected tests, or if no tests
            // are selected run everything.
            // "rowsToProcess" contains a list of all tests we intent to run.
            List<ListViewItem> rowsToProcess = new List<ListViewItem>();
            int numberSelectedItems = listView1.SelectedItems.Count;
            if (numberSelectedItems > 0)
            {
                for (int i = 0; i < numberSelectedItems; i++)
                {
                    var itemRow = listView1.SelectedItems[i];
                    rowsToProcess.Add(itemRow);
                }
            }
            else
            {
                // If no tests are selected, run everything
                foreach (ListViewItem itemRow in listView1.Items)
                {
                    rowsToProcess.Add(itemRow);
                }
            }

            listView1.SelectedItems.Clear();
            queueUpAndRunTests(numberOfThreadsToUse, rowsToProcess, rebaseRequested, debugRequested, leakCheckRequested);
        }

        // @brief This callback runs when the user clicks the "Load Tests" button
        private void button1_Click(object sender, EventArgs e)
        {
            // Populate the user settings with defaults, if user has not yet made a choice
            // Use:  Win32 / Release as the default, since this is what we deploy.
            if (comboBox1.Text == "")
            {
                comboBox1.SelectedItem = "x64";
            }
            if (comboBox2.Text == "")
            {
                comboBox2.SelectedItem = "Release";
            }
            if (comboBox3.Text == "")
            {
                comboBox3.SelectedItem = "2022";
            }

            // User Workspace folder
            if (textBox1.Text == "")
            {
                textBox1.Text = "%AQ%";
            }

            // User Log folder
            if (textBox6.Text == "")
            {
                textBox6.Text = "%AQ%\\resources\\apps\\TestDrive\\errorlogs";
            }

            // Parse the architecture and build user settings
            testArchitecture_ = comboBox1.Text;
            testBuildType_ = comboBox2.Text;
            testVisualStudio_ = comboBox3.Text;
            string command = Utilities.constructPathToTestExecutable(textBox1.Text, testVisualStudio_, testArchitecture_, testBuildType_);

            // Construct the argument to google_test, requesting a list of all available tests
            string commandArg = "--gtest_list_tests";

            // Actually run the command to fetch the list of available tests
            string output;
            bool debugRequested = false;
            runExternalCommand(command, commandArg, debugRequested, out output);

            // Split the output into separate lines
            var outputLines = Regex.Split(output, "\r\n|\r|\n");

            // Create a storage area for the tests and results
            backingStore_ = new List< string[] >();

            // Clear any data currently in the list view in preparation for populating the new list of available tests
            listView1.Items.Clear();
            listView1.Update();
            listView1.Refresh();

            // Now parse the output from google_test and populate the list of tests
            string currentFolder = "";
            int testNumber = 0;
            foreach(string line in outputLines)
            {
                // A google_test folder ends with a period "."
                string trimmed = line.TrimEnd();
                if (trimmed.EndsWith("."))
                {
                    currentFolder = trimmed.Substring(0, trimmed.Length-1);
                    continue;
                }

                // A test-case name is indented by two spaces
                if (trimmed.StartsWith("  "))
                {
                    string currentTest = trimmed.Trim();

                    // Populate the test into the listview
                    testNumber++;
                    string[] row = { ""+ testNumber, currentFolder, currentTest, "", "" };
                    backingStore_.Add(row);
                    var listViewItem = new ListViewItem(row);
                    listView1.Items.Add(listViewItem);
                }
            }

            // Update the status-bar, indicating the number of available tests
            toolStripStatusLabel1.Text = "Tests: " + listView1.Items.Count;
        }

        // @brief This callback runs when the user clicks the "Run Tests" button
        private void button2_Click(object sender, EventArgs e)
        {
            bool rebaseRequested    = false;
            bool debugRequested     = false;
            bool leakCheckRequested = false;
            RunAndDebugButtonHandler(rebaseRequested, debugRequested, leakCheckRequested);
        }

        // @brief This callback runs when the user clicks the "Debug Tests" button
        private void button4_Click(object sender, EventArgs e)
        {
            bool rebaseRequested    = false;
            bool debugRequested     = true;
            bool leakCheckRequested = false;
            RunAndDebugButtonHandler(rebaseRequested, debugRequested, leakCheckRequested);
        }


        /* @brief This callback runs when the user clicks the "Rebase" button    
         */
        private void button3_Click(object sender, EventArgs e)
        {
            bool rebaseRequested    = true;
            bool debugRequested     = false;
            bool leakCheckRequested = false;
            
            resetTestStatisticsAndUpdateStatusBar();
            int numberOfThreadsToUse = processThreadsInputParameter();

            // Determine which tests have failed, either from the user selection or out
            // of the entire test portfolio

            // "rowsToProcess" contains a list of all tests we intent to run.
            List<ListViewItem> rowsToProcess = new List<ListViewItem>();
            int numberSelectedItems = listView1.SelectedItems.Count;
            if (numberSelectedItems > 0)
            {
                for (int i = 0; i < numberSelectedItems; i++)
                {
                    var itemRow = listView1.SelectedItems[i];
                    if (itemRow.SubItems[3].Text == "FAIL")
                    {
                        rowsToProcess.Add(itemRow);
                    }
                }
            }
            else
            {
                // If no tests are selected, rebase ALL failed tests
                foreach (ListViewItem itemRow in listView1.Items)
                {
                    if (itemRow.SubItems[3].Text == "FAIL")
                    {
                        rowsToProcess.Add(itemRow);
                    }
                }
            }

            if (rowsToProcess.Count > 0)
            {
                bool permissionToGoAhead =
                       MessageBox.Show("WARNING: This will rebase "
                                + rowsToProcess.Count
                                + " Test(s)! Have you done proper due diligence to reset test values "
                                + " Are you SURE you want to proceed?",
                                "Confirm",
                                MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes;

                if ( ! permissionToGoAhead )
                    return;
            }

            queueUpAndRunTests(numberOfThreadsToUse, rowsToProcess, rebaseRequested, debugRequested, leakCheckRequested);
        }

        /* @brief  This is used to filter the tests by TestFolder, TestName and Result.
         *         Each time the user types more text in the filter boxes, we re-build the list of matching tests.
         *         A match is found if the typed-in filter text matches part of the test data.
         */
        private void textBox3_TextChanged(object sender, EventArgs e)
        {
            // If no tests have been loaded, simply ignore the filter text.
            if (backingStore_ == null)
            {
                return;
            }

            listView1.Items.Clear(); // clear list items before adding 
            // filter the items match with search key and add result to list view 
            listView1.Items.AddRange(backingStore_.Where(data => 
                  (data[1].Contains(textBox3.Text)
                && data[2].Contains(textBox4.Text)
                && data[3].Contains(textBox5.Text)))
                .Select(c => new ListViewItem(c)).ToArray());

            // If some of the tests have already run, update those rows with the pass / fail colour
            foreach (ListViewItem itemRow in listView1.Items)
            {
                if (itemRow.SubItems[3].Text == "PASS")
                {
                    itemRow.BackColor = Color.LightGreen;
                }
                if (itemRow.SubItems[3].Text == "WARNING")
                {
                    itemRow.BackColor = Color.Yellow;
                }
                else if (itemRow.SubItems[3].Text == "FAIL")
                {
                    itemRow.BackColor = Color.OrangeRed;
                }
            }

            // Update the status bar with the number of tests matching the filter
            toolStripStatusLabel1.Text = "Tests: " + listView1.Items.Count;
        }

        /* @brief  This implements a very simple sort of the listview data.
         *         It is called in response to the user clicking on a column heading.
         *         This very simple sort always performs an ascending sort.
         */
        private void listView1_ColumnClick(object sender, System.Windows.Forms.ColumnClickEventArgs e)
        {
            // Sort by selected column
            int column = e.Column;

            List<string[]> sortedList;
            if (column == 0)
            {
                // Use numerical order for sorting the test number
                sortedList = backingStore_.OrderBy(data =>  int.Parse(data[column]) ).ToList();
            }
            else if (column == 4)
            {
                // Use a numerical sort for the execution time (a double)
                // The tricky part here is that not all the tests may have been run.
                // To get around this, we insert a temporary fake "-1" execution time.
                sortedList = new List<string[]>();
                foreach (string[] data in backingStore_)
                {
                    if (data[4] == "")
                    {
                        data[4] = "-1";
                    }
                    sortedList.Add(data);
                }

                // Perform the sort by double comparison
                sortedList = sortedList.OrderBy(data => double.Parse(data[column])).ToList();

                // Remove the fake "-1" calculation times
                foreach (string[] data in sortedList)
                {
                    if (data[4] == "-1")
                    {
                        data[4] = "";
                    }
                }
            }
            else
            {
                // Use Lexicographical order to sort the other columns
                sortedList = backingStore_.OrderBy(data => data[column] ).ToList();
            }
            backingStore_ = sortedList;

            // Now re-apply the filter, if any
            textBox3_TextChanged(sender, e);
        }

        private void listView1_MouseClick(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                if (listView1.FocusedItem.Bounds.Contains(e.Location) == true)
                {
                    contextMenuStrip1.Show(Cursor.Position);
                }
            }
        }

        void contextMenu1_ItemClicked(object sender, ToolStripItemClickedEventArgs e)
        {
            ToolStripItem item = e.ClickedItem;

            if ( item.Text.Contains("Run") )
            {
                // Forward request to the Run button handler
                EventArgs dummy = new EventArgs();
                button2_Click(sender, dummy);
            }
            else if ( item.Text.Contains("Rebase") )
            {
                // Forward request to the Rebase button handler
                EventArgs dummy = new EventArgs();
                button3_Click(sender, dummy);
            }
            else if ( item.Text.Contains("Debug") )
            {
                // Forward request to the Debug button handler
                EventArgs dummy = new EventArgs();
                button4_Click(sender, dummy);
            }
            else if (item.Text.Contains("Leak"))
            {
                // Perform a leak-check of the selected test(s)
                // Forward request to the Run button handler
                bool rebaseRequested    = false;
                bool debugRequested     = false;
                bool leakCheckRequested = true;
                RunAndDebugButtonHandler(rebaseRequested, debugRequested, leakCheckRequested);
            }
            else if (item.Text.Contains("View error"))
            {
                int numberSelectedItems = listView1.SelectedItems.Count;
                if (numberSelectedItems > 0)
                {
                    for (int i = 0; i < numberSelectedItems; i++)
                    {
                        var itemRow = listView1.SelectedItems[i];

                        if (itemRow.SubItems[3].Text == "FAIL" || itemRow.SubItems[3].Text == "WARNING")
                        {
                            string command = getPathToNotepadExe();
                            string testFailureLogPath = Utilities.getFailureLogPath(textBox6.Text, testArchitecture_, testBuildType_, itemRow);

                            Task.Factory.StartNew(() =>
                            {
                                string output;
                                int exitCode = runExternalCommand(command, testFailureLogPath, false, out output);
                            });
                        }
                    }
                }                
            }
            else if ( item.Text.Contains("Copy Gtest") )
            {
                int numberSelectedItems = listView1.SelectedItems.Count;
                if (numberSelectedItems > 0)
                {
                    // If multiple tests are selected, just choose the first
                    var itemRow = listView1.SelectedItems[0];
                    string gTestFilter = Utilities.constructGTestFilter(itemRow);
                    Clipboard.SetText(gTestFilter);
                }
            }
        }

        private static string getPathToNotepadExe()
        {
            string command = "C:\\Windows\\System32\\notepad.exe";
            return command;
        }

    }

    /* @brief This is used to prevent UI flickering when the listView is updated.
     *        It allows us to use "DoubleBuffering" i.e. the listView is updated offscreen,
     *        and then when updates are complete we switch to the new view.
     *        A true test-samurai expects nothing less.
     */
    public static class ControlExtensions
    {
        public static void DoubleBuffering(this Control control, bool enable)
        {
            var method = typeof(Control).GetMethod("SetStyle", BindingFlags.Instance | BindingFlags.NonPublic);
            method.Invoke(control, new object[] { ControlStyles.OptimizedDoubleBuffer, enable });
        }
    }
}

