using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows.Forms;  // For ListItem. Use the ListItem as a handy container
using System.Threading.Tasks;

namespace TestNinja
{
    class ConsoleTestRunner
    {
        private string testVisualStudio_;   // Visual Studio 2017 or 2015
        private string testArchitecture_;   // Win32 or x64
        private string testBuildType_;      // Debug, Release, LeakCheck

        private int numThreadsToUse_;

        private string workspace_;
        private string logFolder_;

        private string filter_;

        private List<string[]> backingStore_;

        int numFailures_;
        int numWarnings_;

        public int getNumFailures()
        {
            return numFailures_;
        }

        public int getNumWarnings()
        {
            return numWarnings_;
        }

        public ConsoleTestRunner( string visualStudio, string testArchitecture, string testBuild, string numThreads, string workspace, string logFolder, string filter)
        {
            testVisualStudio_ = visualStudio;
            testArchitecture_ = testArchitecture;
            testBuildType_ = testBuild;
            numThreadsToUse_ = Utilities.getNumThreadsToUse(numThreads);
            workspace_ = workspace;
            logFolder_ = logFolder;
            filter_ = filter;

        }

        public void LoadTests()
        {

            string command = Utilities.constructPathToTestExecutable( workspace_, testVisualStudio_, testArchitecture_, testBuildType_);

            // Construct the argument to google_test, requesting a list of all available tests
            string commandArg = "--gtest_list_tests";

            // If a test filter has been provided, pass this through to GOOGLE_TEST.
            // This can be an include or exclude (-) filter.
            if ( filter_ != "" )
            {
                commandArg += " --gtest_filter=" + filter_;

            }

            // Actually run the command to fetch the list of available tests
            string output;
            bool debugRequested = false;
            string summaryLogPath = Utilities.getSummaryLogFilePath(logFolder_, testArchitecture_, testBuildType_);

            Utilities.runExternalCommand(command, commandArg, debugRequested, summaryLogPath, out output);

            // Split the output into separate lines
            var outputLines = Regex.Split(output, "\r\n|\r|\n");

            // Create a storage area for the tests and results
            backingStore_ = new List<string[]>();

            // Now parse the output from google_test and populate the list of tests
            string currentFolder = "";
            int testNumber = 0;
            foreach (string line in outputLines)
            {
                // A google_test folder ends with a period "."
                string trimmed = line.TrimEnd();
                if (trimmed.EndsWith("."))
                {
                    currentFolder = trimmed.Substring(0, trimmed.Length - 1);
                    continue;
                }

                // A test-case name is indented by two spaces
                if (trimmed.StartsWith("  "))
                {
                    string currentTest = trimmed.Trim();

                    // Populate the test into the listview
                    testNumber++;
                    string[] row = { "" + testNumber, currentFolder, currentTest, "", "" };
                    backingStore_.Add(row);
                }
            }

            foreach (string[] row in backingStore_)
            {
                // Console.Out.WriteLine(row[0] + " " + row[1] + "." + row[2]);
            }

            Console.Out.WriteLine( "\n\nNumber of tests: " + backingStore_.Count);

        }

        /* @brief Constructs the command line and arguments that are used to run a test
         *        This function is invoked by a thread pool
         * @param[in] obj   The context information provided by the threadpool. In this case, it is the ListViewItem of the selected test
         * @param[out]      Returns the exitCode from running the test
         */
        private int processTest(Object obj, bool rebaseRequested, bool debugRequested, string summaryLogPath, out string output)
        {
            // Parse the architecture and build user settings
            string command = Utilities.constructPathToTestExecutable(workspace_, testVisualStudio_, testArchitecture_, testBuildType_);

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

            if (testBuildType_.Equals("LeakCheck", StringComparison.OrdinalIgnoreCase))
            {
                commandArg += " --leakcheck";
            }

            // Actually run the test
            int exitCode = Utilities.runExternalCommand(command, commandArg, debugRequested, summaryLogPath, out output);
            return exitCode;
        }

        private int queueUpAndRunTests(int numberOfThreadsToUse, List<ListViewItem> rowsToProcess, bool rebaseRequested, bool debugRequested)
        {
            // Queue up / Schedule the tests and instruct the thread pool to run them in parallel
            // Update the status bar with the number of scheduled tests


            // Now submit all the jobs to the thread pool
            // This partitioner performs perfect load-balancing across all available threads
            var customPartitioner = new SingleItemIListPartitioner<ListViewItem>(rowsToProcess);

            string summaryLogPath = Utilities.getSummaryLogFilePath(logFolder_, testArchitecture_, testBuildType_);

            Parallel.ForEach(customPartitioner,    // Run the tests in parallel...
                    new ParallelOptions { MaxDegreeOfParallelism = numberOfThreadsToUse },   // ... specifying a maximum number of worker threads
                    (row) =>
                    {
                        // Start a timer and run the test, rounding the executation time to 1dp
                        var watch = System.Diagnostics.Stopwatch.StartNew();

                        // ---- HERE IS WHERE WE ACTUALLY RUN THE TEST ON A THREAD ---
                        string output;
                        int exitCode = processTest(row, rebaseRequested, debugRequested, summaryLogPath, out output);
                        // ---

                        watch.Stop();
                        var elapsedMs = watch.ElapsedMilliseconds;
                        double elapsedSec = elapsedMs / 1000.0;
                        double roundedSec = Math.Round(elapsedSec, 1);


                        // Decide if test passed of failed, and update the UI.
                        // As we are on a worker thread, use "Invoke" to message back to the UI thread

                        string result;
                        if (exitCode == 0)
                        {
                            result = "PASS";
                            // The test passed!
                            row.SubItems[3].Text = result;
                        }
                        else if (exitCode == 3)
                        {
                            result = "WARNING";
                            // The test passed!
                            row.SubItems[3].Text = result;
                        }
                        else
                        {
                            result = "FAIL";
                            row.SubItems[3].Text = result;
                        }

                        // Finally display the execution time of this test (actual wall-clock time).
                        row.SubItems[4].Text = "" + roundedSec;

                        // Update backing store with the test result and calc time
                        int testNumber;
                        Int32.TryParse(row.SubItems[0].Text, out testNumber);
                        int idx = testNumber - 1;
                        backingStore_.ElementAt(idx)[3] = result;
                        backingStore_.ElementAt(idx)[4] = "" + roundedSec;
                    });
        
            // Write the summary list of test failures to a file
            string testFailuresTxt = "";
            string testWarningsTxt = "";
            numFailures_ = 0;
            numWarnings_ = 0;
            foreach (ListViewItem itemRow in rowsToProcess)
            {
                if (itemRow.SubItems[3].Text == "FAIL")
                {
                    // If the FAILED test includes the word "IGNORE" in its name, do not include the failed test in the final tally.
                    // For example, the UNIT_CreateDeliberateLeak_IGNORE test is designed to leak 10 bytes of memory in order to
                    // verify that the leak checker is working. However this is not a real leak in the library, and we do not want this
                    // to fail an overnight build.
                    if (! itemRow.SubItems[2].Text.Contains("IGNORE") )
                    {
                        testFailuresTxt += itemRow.SubItems[0].Text + "   " + itemRow.SubItems[1].Text + "." + itemRow.SubItems[2].Text + System.Environment.NewLine;
                        numFailures_++;
                    }
                }

                if (itemRow.SubItems[3].Text == "WARNING")
                {
                    testWarningsTxt += itemRow.SubItems[0].Text + "   " + itemRow.SubItems[1].Text + "." + itemRow.SubItems[2].Text + System.Environment.NewLine;
                    numWarnings_++;
                }
            }

            if (numFailures_ > 0)
            {
                Console.Out.WriteLine("\n#Error: There were " + numFailures_ + " test failure(s):");

                if (summaryLogPath.Length > 0)
                    Console.Out.WriteLine("Please see Error Log stored at: " + summaryLogPath);

                Console.Out.WriteLine("Failed Tests: ");
                Console.Out.WriteLine(testFailuresTxt);

                Utilities.LogAppend(summaryLogPath, System.Environment.NewLine + "#Error: There were " + numFailures_ + " test failure(s):" + System.Environment.NewLine);
            }

            if (numWarnings_ > 0)
            {
                Console.Out.WriteLine("\nThere were " + numWarnings_ + " test(s) with warnings:");

                if (summaryLogPath.Length > 0)
                    Console.Out.WriteLine("Please see Error Log stored at: " + summaryLogPath);

                Console.Out.WriteLine("Warning Tests: ");
                Console.Out.WriteLine(testWarningsTxt);

                Utilities.LogAppend(summaryLogPath, System.Environment.NewLine + "#Warning: There were " + numWarnings_ + " test(s) with warnings:" + System.Environment.NewLine);

            }

            Utilities.LogAppend(summaryLogPath, testWarningsTxt);

            return numFailures_;
          
        }

        public int RunTestsInParallel()
        {
            // Determine which tests to run: Either run the specified tests, or if no tests
            // are specified run everything.
            // "rowsToProcess" contains a list of all tests we intent to run.

            List<ListViewItem> rowsToProcess = new List<ListViewItem>();
            
            foreach (string[] row in backingStore_)
            {
                var listViewItem = new ListViewItem(row);
                rowsToProcess.Add(listViewItem);
            }

            bool rebaseRequested = false;
            bool debugRequested = false;
            int numFailures = queueUpAndRunTests(numThreadsToUse_, rowsToProcess, rebaseRequested, debugRequested);
            //int numFailures = 0;
            return numFailures;
        }

        public int LoadAndRunTests()
        {
            string summaryLogPath = Utilities.getSummaryLogFilePath(logFolder_, testArchitecture_, testBuildType_);

            Utilities.LogDelete(summaryLogPath);

            LoadTests();
            int numFailures = RunTestsInParallel();

            return numFailures;
        }

    }
}
