xcopy switches
--------------

/s/e - recursive copy, including copying empty directories.
/v - add this to verify the copy against the original. slower, but for the paranoid.
/h - copy system and hidden files.
/k - copy read-only attributes along with files. otherwise, all files become read-write.
/x - if you care about permissions, you might want /o or /x.
/y - don't prompt before overwriting existing files.
/z - if you think the copy might fail and you want to restart it, use this. It places a marker on each file as it copies, so you can rerun the xcopy command to pick up from where it left off.

A	Copies only files with the archive attribute set, doesn't change the attribute.
/M	Copies only files with the archive attribute set, turns off the archive attribute. Useful in backup.
/D:m-d-y	Copies files changed on or after the specified date. If no date is given, copies only those files whose source time is newer than the destination time. Useful in backup.
/P	Prompts you before creating each destination file.
/S	Copies directories and subdirectories except empty ones.
/E	Copies directories and subdirectories, including empty ones. Same as /S /E. May be used to modify /T.
/V	Verifies each new file. Not used by Windows XP.
/W	Prompts you to press a key before copying.
/C	Continues copying even if errors occur.
/I	If destination does not exist and copying more than one file, assumes that destination must be a directory.
/Q	Does not display file names while copying.
/F	Displays full source and destination file names while copying.
/L	Displays files that would be copied.
/G	Allows the copying of encrypted files to destination that does not support encryption.
/H	Copies hidden and system files also.
/R	Overwrites read-only files.
/T	Creates directory structure, but does not copy files. Does not include empty directories or subdirectories.
/U	Copies only files that already exist in destination.

xcopy /y "C:\Source\ "D:\Destination"