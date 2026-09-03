using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.IO;
using System.Windows.Forms;

using System.Text.RegularExpressions;


namespace FileSearch
{
    public partial class FileRenamer : Form
    {
        private List<SourceFile> allFiles_;

        public FileRenamer(Form1 parent)
        {
            InitializeComponent();

            allFiles_ = parent.getAllFiles();

            String currentFilename = parent.getCurrentFileSelection();

            originalFilename.Text = currentFilename;
            newFilename.Text = currentFilename;
        }

        private void previewButton_Click(object sender, EventArgs e)
        {
            // Extract the original filename from the full path
            String originalFilePath = originalFilename.Text;
            String origFilePattern = Path.GetFileName(originalFilePath);  // Allowed to contain a regex


            // Extract the rename filename from the full path
            String renameFilePath = newFilename.Text;
            String renameFilePattern = Path.GetFileName(renameFilePath);

            // Search for that original filename in all code

            listView1.Items.Clear(); // clear list items before adding 
                                     // filter the items match with search key and add result to list view 


            // Find all the matching filenames which match origFilePattern
            List<String> matchingFiles = new List<String>();
            List<String> newFiles = new List<String>();

            foreach (SourceFile sFile in allFiles_)
            {
                Match fileMatch = Regex.Match(sFile.filename_, origFilePattern);
                if (fileMatch.Success)
                {
                    matchingFiles.Add(sFile.filename_);
                    String newFile = Regex.Replace(sFile.filename_, origFilePattern, renameFilePattern);
                    newFiles.Add(newFile);
                }
            }

            // Now loop over matchingFiles, newFiles and rename each one
            int numMatchingFiles = matchingFiles.Count();
            for (int i=0; i<numMatchingFiles; i++ )
            {
                String originalFileWithPath = matchingFiles[i];
                String originalFile = Path.GetFileName(originalFileWithPath);

                String newFileWithPath = newFiles[i];
                String newFile = Path.GetFileName(newFileWithPath);
                String textSearch = originalFile;

                StringComparison stringComparisonMethod = StringComparison.CurrentCulture;

                int count = 0;
                bool continueSearch = true;

                // Perform a full search using all filters
                foreach (SourceFile sFile in allFiles_)
                {
                    if (continueSearch)
                    {
                        List<String> lines = sFile.lines_;
                        int line_count = 1;
                        foreach (String line in lines)
                        {
                            bool lineMatch = false;

                            if (line.FancyContains(textSearch, stringComparisonMethod))
                            {
                                lineMatch = true;
                            }

                            if (lineMatch)
                            {
                                // Update the list view with each match
                                string[] data = { sFile.filename_, sFile.suffix_, line_count.ToString(), line };
                                ListViewItem lvi = new ListViewItem(data);
                                listView1.Items.Add(lvi);
                                count++;
                            }

                            line_count++;
                        }

                    }
                }
                totalMatches.Text = "Matches: " + count;
            }

        }

        private void doitButton_Click(object sender, EventArgs e)
        {
            // Extract the original filename from the full path
            String originalFilePath = originalFilename.Text;
            String origFilePattern = Path.GetFileName(originalFilePath);  // Allowed to contain a regex

            
            // Extract the rename filename from the full path
            String renameFilePath = newFilename.Text;
            String renameFilePattern = Path.GetFileName(renameFilePath);

            // Find all the matching filenames which match origFilePattern
            List<String> matchingFiles = new List<String>();
            List<String> newFiles = new List<String>();

            foreach (SourceFile sFile in allFiles_)
            {
                Match fileMatch = Regex.Match(sFile.filename_, origFilePattern);
                if (fileMatch.Success)
                {
                    matchingFiles.Add(sFile.filename_);
                    String newFile = Regex.Replace(sFile.filename_, origFilePattern, renameFilePattern);
                    newFiles.Add(newFile);
                }
            }


            // Now loop over matchingFiles, newFiles and rename each one
            int numMatchingFiles = matchingFiles.Count();
            for (int i = 0; i < numMatchingFiles; i++)
            {
                String originalFileWithPath = matchingFiles[i];
                String originalFile = Path.GetFileName(originalFileWithPath);

                String newFileWithPath = newFiles[i];
                String newFile = Path.GetFileName(newFileWithPath);


                String textSearch = originalFile;
                // For each match in the listview, replace the string on that line
                foreach (ListViewItem item in listView1.Items)
                {
                    String fileName = item.SubItems[0].Text;
                    int lineNumber = Convert.ToInt32(item.SubItems[2].Text);

                    foreach (SourceFile sourceFile in allFiles_)
                    { 
                        if (sourceFile.filename_ == fileName)
                        {
                            var linesInFile = sourceFile.lines_;
                            String matchingLine = linesInFile[lineNumber - 1];
                            String updatedLine = matchingLine.Replace(originalFile, newFile);
                            sourceFile.lines_[lineNumber - 1] = updatedLine;
                        }
                    }
                }
            }
       
            HashSet<String> writtenFiles = new HashSet<String>();

            // Then save / rewrite each file in listview
            foreach (ListViewItem item in listView1.Items)
            {
                String fileName = item.SubItems[0].Text;
                if ( ! writtenFiles.Contains( fileName ))
                {
                    writtenFiles.Add(fileName);

                    foreach (SourceFile sourceFile in allFiles_)
                    {
                        if (sourceFile.filename_ == fileName)
                        {
                            TextWriter tw = new StreamWriter(fileName);
                            var textToWrite = sourceFile.lines_;
                            foreach (String txtLine in textToWrite)
                            {
                                tw.WriteLine(txtLine);
                            }
                            tw.Close();
                        }
                    }

                }
            }

            // Finally rename the files
            for (int i = 0; i < numMatchingFiles; i++)
            {
                String originalFile = matchingFiles[i];
                String newFile = newFiles[i];
                System.IO.File.Move(originalFile, newFile);
            }

        }
    }
}

