using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;   // DllImport
using System.Threading;
using System.Diagnostics;
using System.Globalization;

namespace TestDrive
{
    static class Program
    {

        // Helper functions to allow us to control our console window
        [DllImport("kernel32.dll", SetLastError = true)]
        static extern bool AllocConsole();

        [DllImport("kernel32.dll", SetLastError = true)]
        static extern bool FreeConsole();

        [DllImport("kernel32", SetLastError = true)]
        static extern bool AttachConsole(int dwProcessId);

        [DllImport("user32.dll")]
        static extern IntPtr GetForegroundWindow();

        [DllImport("user32.dll", SetLastError = true)]
        static extern uint GetWindowThreadProcessId(IntPtr hWnd, out int lpdwProcessId);

        [DllImport("kernel32.dll")]
        static extern IntPtr GetConsoleWindow();

        [DllImport("user32.dll")]
        static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);

        const int SW_HIDE = 0;
        const int SW_SHOW = 5;

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static int Main( string[] args )
        {
            if (args.Length == 0)
            {
                // Run in GUI interactive mode
                // Determine whether we should close the console window:
                IntPtr ptr = GetForegroundWindow();

                int  u;
                GetWindowThreadProcessId(ptr, out u);

                Process process = Process.GetProcessById(u);

                if (process.ProcessName == "cmd")    //Is the uppermost window a cmd process?
                {
                    // We were started from a cmd window. Do not close the window
                }
                else
                {
                    // We were started by double-clicking an icon. Close the console window.
                    var handle = GetConsoleWindow();
                    ShowWindow(handle, SW_HIDE);
                }

                // Start up the main GUI app
                Application.EnableVisualStyles();
                Application.SetCompatibleTextRenderingDefault(false);
                Application.Run(new Form1());
                return 0;
            }
            else
            {
                // Do command line/silent logic here

                // Some guidelines on C# console applications:
                // 1 https://stackoverflow.com/questions/7198639/c-sharp-application-both-gui-and-commandline
                // 2 https://stackoverflow.com/questions/7198639/c-sharp-application-both-gui-and-commandline/7198945#7198945   <--- This seems not bad, but the console runs concurrently. So no way to know when TestDrive terminates
                // 3 https://stackoverflow.com/questions/3571627/show-hide-the-console-window-of-a-c-sharp-console-application  <-- This seems to work OK, but we get the console flashing up temporarily
                // 4 https://stackoverflow.com/questions/472282/show-console-in-windows-application?noredirect=1&lq=1  <-- Long Answer by Jeffrey Knight. The console does not wait for app to finish

                // We use a combination of 3 and 4. Start off with a console application and close the window if it looks like we were started by double clicking

                Console.Out.WriteLine("TestDrive is running in command line mode! ");

                // Parse command line arguments. The args should be specified in pairs
                if (args.Length % 2 > 0)
                {
                    Console.Out.WriteLine("#Error: Wrong number of command line arguments.");
                    Console.Out.WriteLine("Exanple usage: TestDrive.exe --visualStudio 2022 --arch x64 --build release --threads 8 --workspace \"%AQ%\" --filter \"*.UNIT_*\" --logFolder \"C:\\temp\"");
                    Environment.Exit(1);
                }

                string visualStudio = "2022";
                string arch = "x64";
                string build = "release";
                string numThreads = "8";
                string workspace = "";
                string logFolder = "";
                string filter = "";

                int numArgs = args.Length;
                for (int i = 0; i < numArgs; i+=2)
                {
                    if (args[i] == "--arch")
                    {
                        arch = args[i + 1];
                    }
                    else if (args[i] == "--build")
                    {
                        build = args[i + 1];
                    }
                    else if (args[i] == "--workspace")
                    {
                        workspace = args[i + 1];
                    }
                    else if (args[i] == "--logFolder")
                    {
                        logFolder = args[i + 1];
                    }
                    else if (args[i] == "--threads")
                    {
                        numThreads = args[i + 1];
                    }
                    else if (args[i] == "--visualStudio")
                    {
                        visualStudio = args[i + 1];
                    }
                    else if (args[i] == "--filter")
                    {
                        filter = args[i + 1];
                    }
                    else
                    {
                        Console.Out.WriteLine("#Error: Invalid command line parameter: " + args[i]);
                        Console.Out.WriteLine("Exanple usage: TestDrive.exe --visualStudio 2022 --arch x64 --build release --threads 8 --workspace \"%AQ%\" --logFolder \"C:\\temp\" --filter testfilter");
                        Environment.Exit(1);
                    }
                }

                if (workspace == "")
                {
                    Console.Out.WriteLine("#Error: Please specify a valid workspace. For example: --workspace \"%AQ%\"");
                    Environment.Exit(1);
                }

                Console.Out.WriteLine("Invoking test runner with parameters: " + visualStudio
                                                                               + ", " + arch
                                                                               + ", " + build
                                                                               + ", " + numThreads + " threads"
                                                                               + ", " + workspace
                                                                               + ", " + logFolder
                                                                               + ", " + filter);

                ConsoleTestRunner consoleTestRunner = new ConsoleTestRunner( visualStudio, arch, build, numThreads, workspace, logFolder, filter );

                // Time how long it takes for tests to run
                var watch = System.Diagnostics.Stopwatch.StartNew();
                
                // ----- Invoke the test runner
                int numFailures = consoleTestRunner.LoadAndRunTests();

                int numWarnings = consoleTestRunner.getNumWarnings();

                // -----

                watch.Stop();
                var elapsedMs = watch.ElapsedMilliseconds;
                double elapsedSec = elapsedMs / 1000.0;
                double roundedSec = Math.Round(elapsedSec, 1);

                Console.Out.WriteLine("Tests ran in: " + roundedSec + " sec. NumFailures: " + numFailures + "  NumWarnings: " + numWarnings );

                var culture = new CultureInfo("en-GB");

                DateTime localDate = DateTime.Now;
                Console.Out.WriteLine("TestDrive completed at: " + localDate.ToString(culture), localDate.Kind);
                return numFailures;
            }
        }
    }
}
