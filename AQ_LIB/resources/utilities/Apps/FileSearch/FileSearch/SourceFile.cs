using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace FileSearch
{
    public class SourceFile
    {
        public SourceFile(String filename, String project, Boolean editStatus )
        {
            filename_ = filename;
            filenameBase_ = Path.GetDirectoryName(filename) + "\\" + Path.GetFileNameWithoutExtension(filename);
            
            suffix_ = Path.GetExtension(filename);
            project_ = project;

            editStatus_ = editStatus == true ? "M" : "";  // 'M' to indicate file is modified; '' to indicate untouched file.
        }

        public String filename_ { get; set; }
        public String filenameBase_ { get; set; }
        public String suffix_ { get; set; }
        public String project_ { get; set; }
        public List<String> lines_ { get; set; }
        public String editStatus_ { get; set; }
    }
}
