SVN Config File
------------------------


Location
------------------------
The SVN config file can be found in the following folder
%APPDATA%\Subversion


Line Endings - New Files
------------------------
This file manages amongst other things the line endings for files, which is important when building cross platform
on Windows AND Linux


Line Endings - Existing Files
------------------------
The config file will automanage the line endings of new files, however for existing files we have to set the 
svn property "eol-style=native" on each file by right clicking on the repository root in file explorer and selecting
TortoiseSVN->Properties

