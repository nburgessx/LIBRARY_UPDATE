SVN PROPS
------------------

To configure check-in file settings and other settings for Windows and Linux we must
update our SVN config or property file called "config"

In particular file line endings must be set at native for windows and linux compatibility.

FOR EXISTING FILES
------------------
Highlight the files in quesiton, then right-click and select SVN->Properties->New->EOL->Platform Dependent (Native)

FOR NEW FILES
-------------------
This is done for new files by updating the SVN props file. Go to the file explorer, right-click
on your repository and select SVN. Then select settings->General->Edit and update the config
file with the one attached.

See below for the line ending syntax we using to update file line-endings.

### Section for configuring automatic properties.
enable-auto-props = yes
[auto-props]
*.c = svn:eol-style=native
*.cpp = svn:eol-style=native
*.h = svn:eol-style=native
*.dsp = svn:eol-style=CRLF
*.dsw = svn:eol-style=CRLF
*.sh = svn:eol-style=native;svn:executable
*.txt = svn:eol-style=native;
*.png = svn:mime-type=image/png
*.jpg = svn:mime-type=image/jpeg
Makefile = svn:eol-style=native
*.conf = svn:eol-style=native;svn:keywords=Id URL
*.csv = svn:eol-style=native;svn:keywords=Id URL
*.json = svn:eol-style=native;svn:keywords=Id URL
*.JSON = svn:eol-style=native;svn:keywords=Id URL