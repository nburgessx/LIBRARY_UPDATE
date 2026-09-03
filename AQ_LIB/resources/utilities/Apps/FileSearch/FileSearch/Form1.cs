using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using System.Reflection;
using System.Globalization;

using System.Text.RegularExpressions;

namespace FileSearch
{
    public partial class Form1 : Form
    {

        private int numberOfFiles_ = 0;
        private int numberOfLines_ = 0;
        private List<SourceFile> allFiles_;
        private HashSet<String> editedFilesSet_;

        public Form1()
        {
            InitializeComponent();

            // Subscribe to the form closing event so that we can save user settings on exit
            this.FormClosing += Form1_Closing;

            // Set focus on the main search box
            this.ActiveControl = textBox5;
        }

        /* @brief This method runs when the main application window is closed
         * It stores user settings into the application config file.
         */
        private void Form1_Closing(object sender, EventArgs e)
        {
            Properties.Settings.Default.Save();
        }

        protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
        {
            if (keyData == (Keys.Control | Keys.F1))
            {
                // Reset everything on the search page
                textBox2.Text = "";
                textBox3.Text = "";
                textBox4.Text = "";
                textBox5.Text = "";
                listView1.Items.Clear();
                toolStripStatusLabel5.Text = "Matches: 0";
                this.ActiveControl = textBox5;
                return true;
            }
            else if (keyData == (Keys.F1))
            {
                this.ActiveControl = textBox2;
                return true;
            }
            else if (keyData == (Keys.F2))
            {
                this.ActiveControl = textBox3;
                return true;
            }
            else if (keyData == (Keys.F3))
            {
                this.ActiveControl = textBox4;
                return true;
            }
            else if (keyData == (Keys.F4))
            {
                this.ActiveControl = textBox5;
                return true;
            }
            else if (keyData == (Keys.Control | Keys.R))
                {
                FileRenamer renamerWindow = new FileRenamer(this);
                renamerWindow.ShowDialog();
            }
            // Call the base class
            return base.ProcessCmdKey(ref msg, keyData);
        }

        public List<SourceFile> getAllFiles()
        {
            return allFiles_;
        }

        public String getCurrentFileSelection()
        {
            String filename = "";
            if (listView1.SelectedItems.Count == 1)
            {
                filename = listView1.SelectedItems[0].SubItems[0].Text;
            }
            return filename;
        }

        private void DirSearch(string sDir, string searchPattern)
        {
            try
            {

                foreach (string d in Directory.GetDirectories(sDir))
                {
                    // Ignore hidden folders
                    DirectoryInfo info = new DirectoryInfo(d);
                    if ((info.Attributes & FileAttributes.Hidden) != FileAttributes.Hidden)
                    {

                        var patterns = searchPattern.Split(';'); // *.cpp;*.h
                        foreach (string pattern in patterns)
                        {
                            foreach (string f in Directory.GetFiles(d, pattern))
                            {
                                String project = "";
                                bool editStatus = editedFilesSet_.Contains(f);
                                SourceFile sourceFile = new SourceFile(f, project, editStatus);
                                Console.WriteLine(f);
                                var lines = File.ReadLines(f);
                                sourceFile.lines_ = lines.ToList<String>();
                                int nLines = lines.Count();
                                numberOfLines_ += nLines;
                                allFiles_.Add(sourceFile);
                                numberOfFiles_++;

                                this.Invoke(new Action(() =>
                                {
                                    toolStripStatusLabel1.Text = "Files:  " + numberOfFiles_;
                                    toolStripStatusLabel3.Text = "Lines:  " + numberOfLines_;
                                }));
                            }
                        }
                        DirSearch(d, searchPattern);
                    }
                }
            }
            catch (System.Exception excpt)
            {
                Console.WriteLine(excpt.Message);
            }
        }

        /*  Returns 0 to indicate success; returns 1 to indicate a failure.
         */
        private int runExternalCommand(string command, string commandArg, string workspaceDir, out string output)
        {
            Console.WriteLine("About to run: " + command + " " + commandArg);

            Process process = new Process();
            process.StartInfo.FileName = command;
            process.StartInfo.Arguments = commandArg;
            process.StartInfo.WorkingDirectory = workspaceDir;
            process.StartInfo.CreateNoWindow = true;
            process.StartInfo.UseShellExecute = false;

            process.StartInfo.RedirectStandardOutput = true;
            process.StartInfo.RedirectStandardError  = true;
            process.StartInfo.RedirectStandardInput  = true;

            try
            {
                process.Start();
            }
            catch (Exception e)
            {
                output = "Unable to execute command: " + command;
                MessageBox.Show(output);
                return 1;
            }

            // Read all the output generated by the command
            output = process.StandardOutput.ReadToEnd();

            // Wait until the process exits and capture the exitCode
            process.WaitForExit();
            int exitCode = process.ExitCode;
            process.Close();

            return exitCode;
        }

        private List<String> getListOfEditedFiles( string workspaceDir )
        {
            List<String> editedFiles = new List<String>();

            String output;
            String gitCommand = "git";
            String gitCommandArg = "diff-index --name-only HEAD";
            int result = runExternalCommand(gitCommand, gitCommandArg, workspaceDir, out output);

            if ( result == 0 )
            {
                // Success
                editedFiles = output.Split('\n').ToList();
            }

            return editedFiles;

        }

        private void loadFilesHandler(string workspaceDir, string searchPattern)
        {
            List<String> editedFiles = getListOfEditedFiles(workspaceDir);
            editedFilesSet_ = new HashSet<String>();

            String workspaceDirWithSuffix = workspaceDir;
            if ( ! workspaceDir.EndsWith("\\"))
            {
                workspaceDirWithSuffix += "\\";
            }


            foreach (var fileWithRelativePath in editedFiles )
            {
                String fileWithAbsolutePath = workspaceDirWithSuffix + fileWithRelativePath;
                fileWithAbsolutePath = fileWithAbsolutePath.Replace("/", "\\"); // The output of Git has forward slashes to represent folder delimiters.
                editedFilesSet_.Add(fileWithAbsolutePath);
            }


            DirSearch(workspaceDir, searchPattern);
            DateTime localDate = DateTime.Now;
            var culture = new CultureInfo("en-GB");
            toolStripStatusLabel4.Text = "Update Time:  " + localDate.ToString(culture);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            string workspaceDir = textBox1.Text;
            string searchPattern = textBox6.Text;
            numberOfFiles_ = 0;
            numberOfLines_ = 0;
            allFiles_ = new List<SourceFile>();
            // Load files in background to keep UI responsive
            Task.Factory.StartNew(() => loadFilesHandler(workspaceDir, searchPattern));
        }

        private void textBox5_KeyDown(object sender, KeyEventArgs e)
        {
            // If no tests have been loaded, simply ignore the filter text.
            if (! (e.KeyData == Keys.Enter) )  // Enter
            {
                return;
            }

            if (allFiles_ == null || allFiles_.Count == 0)
            {
                return;
            }

            listView1.Items.Clear(); // clear list items before adding 
            // filter the items match with search key and add result to list view 
            
            string textSearch = textBox5.Text;
            string fileNameSearch = textBox2.Text;
            string suffixSearch = textBox3.Text;
            string gitStatusSearch = textBox4.Text;
            bool useRegEx = checkBox1.Checked;
            bool caseSensitiveCompare = checkBox2.Checked;
            StringComparison stringComparisonMethod = caseSensitiveCompare ? StringComparison.CurrentCulture : StringComparison.CurrentCultureIgnoreCase;

            //string projectSearch = textBox4.Text;

            int maxMatches;
            if (! Int32.TryParse(textBox7.Text, out maxMatches))
            {
                maxMatches = 1000; // Default maximum number of matches
            }
            int count = 0;
            bool continueSearch = true;

            if (textSearch == "")
            {
                // No text to search for?
                if (fileNameSearch == "" && suffixSearch == "" && gitStatusSearch == "")
                {
                    // All filters are empty
                    return;
                }

                // Perform a simpler search based only on filenames and suffix
                foreach (SourceFile sFile in allFiles_)
                {
                    if ( (count < maxMatches) && continueSearch )
                    {
                        List<String> lines = sFile.lines_;
                        int line_count = 1;
                        foreach (String line in lines)
                        {
                            if (sFile.filenameBase_.FancyContains(fileNameSearch, stringComparisonMethod) && sFile.suffix_.FancyContains(suffixSearch, stringComparisonMethod) && sFile.editStatus_.FancyContains( gitStatusSearch))
                            {
                                string[] data = { sFile.filename_, sFile.suffix_, sFile.editStatus_, line_count.ToString(), line };
                                ListViewItem lvi = new ListViewItem(data);
                                listView1.Items.Add(lvi);
                                count++;
                                break;
                            }
                            line_count++;
                        }
                    }
                }


            }
            else
            {
                // Perform a full search using all filters
                foreach (SourceFile sFile in allFiles_)
                {
                    if ( (count < maxMatches) && continueSearch )
                    {

                        if (sFile.filenameBase_.FancyContains(fileNameSearch, stringComparisonMethod) && sFile.suffix_.FancyContains(suffixSearch, stringComparisonMethod) && sFile.editStatus_.FancyContains(gitStatusSearch))
                        {
                            // filename and suffix matches... now look at the text 

                            List<String> lines = sFile.lines_;
                            int line_count = 1;
                            foreach (String line in lines)
                            {
                                bool lineMatch = false;

                                if (useRegEx)
                                {
                                    try
                                    {
                                        Match textMatch = Regex.Match(line, textSearch);
                                        if (textMatch.Success)
                                        {
                                            lineMatch = true;
                                        }
                                    }
                                    catch (ArgumentException)
                                    {
                                        string output = "Invalid RegEx pattern: " + textSearch;
                                        MessageBox.Show(output);
                                        continueSearch = false;
                                        break;
                                    }
                                }
                                else
                                {
                                    if (line.FancyContains(textSearch, stringComparisonMethod))
                                    {
                                        lineMatch = true;
                                    }
                                }

                                if (lineMatch)
                                {
                                    string[] data = { sFile.filename_, sFile.suffix_, sFile.editStatus_, line_count.ToString(), line };
                                    ListViewItem lvi = new ListViewItem(data);
                                    listView1.Items.Add(lvi);
                                    count++;
                                    if (count >= maxMatches)
                                    {
                                        continueSearch = false;
                                        break;
                                    }
                                }

                                line_count++;
                            }
                        }
                    }
                }
            }
            toolStripStatusLabel5.Text = "Matches: " + count;
        }

        private void listView_DoubleClick(object sender, EventArgs e)
        {
            if (listView1.SelectedItems.Count == 1)
            {
                String filename = listView1.SelectedItems[0].SubItems[0].Text;

                String visualStudioExePath = textBox8.Text;
                String notepadExePath = textBox9.Text;

                String command;
                String cmdArgs;

                if (ModifierKeys.HasFlag(Keys.Control))
                {
                    command = notepadExePath;
                    cmdArgs = filename;
                }
                else
                {
                    command = visualStudioExePath;
                    cmdArgs = "/edit " + filename;
                }

                Process process = new Process();
                process.StartInfo.FileName = command;
                process.StartInfo.Arguments = cmdArgs;
                process.StartInfo.CreateNoWindow = true;
                process.StartInfo.UseShellExecute = false;

                try
                {
                    process.Start();
                }
                catch (Exception ex)
                {
                    string output = "Unable to execute command: " + command;
                    MessageBox.Show(output);
                }
            }
        }

        private void listView1_DragLeave(object sender, EventArgs e)
        {
            //string[] filesToDrag = { textBox2.Text };
            List<String> filesToDrag = new List<String>();

            int nItems = listView1.SelectedItems.Count;
            for (int i = 0; i < nItems; i++)
            {
                var itemRow = listView1.SelectedItems[i];
                string filename = itemRow.SubItems[0].Text;
                filesToDrag.Add(filename);
            }
            
            textBox2.DoDragDrop(new DataObject(DataFormats.FileDrop, filesToDrag), DragDropEffects.Copy);

        }

        private void listView1_ItemDrag(object sender, ItemDragEventArgs e)
        {
            List<string> filesToDrag = new List<string>();

            int nSelectedItems = listView1.SelectedItems.Count;
            if (nSelectedItems > 0)
            {
                for (int i = 0; i < nSelectedItems; i++)
                {
                    var itemRow = listView1.SelectedItems[i];
                    filesToDrag.Add(itemRow.Text);
                    Console.WriteLine(itemRow.Text);
                }
                string[] filenames = filesToDrag.ToArray();
                listView1.DoDragDrop(new DataObject(DataFormats.FileDrop, filenames), DragDropEffects.Copy);          
            }
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

    public static class StringExtensions
    {
        public static bool FancyContains(this string text, string value,
            StringComparison stringComparison = StringComparison.CurrentCultureIgnoreCase)
        {
            return text.IndexOf(value, stringComparison) >= 0;
        }
    }
}
