Instructions
------------
Please copy the *.prop files in this folder to the default start-up location, overriding the exisiting default prop files


copy
----
Microsoft.Cpp.Win32.user.props
Microsoft.Cpp.x64.user.props


Destination Folder
------------------
C:\Users\BurgNi\AppData\Local\Microsoft\MSBuild\v4.0


Property Manager
----------------
Property files can be loaded from any file or folder. This can be done by selecting View->Property Manager
and adding an existing *.props file. Note that if there are several *.props files shown for a particular
project config the top *.props file takes precedence. Prop file precedence can be adjusted using the up
and down arrows in the property manager. New props files can also be created from the property manager.


Restoration
-----------
Should you wish to restore the default *.props files to their original state, then open
these props files in the destination folder within a text editor such as notepad++ and 
delete and replace the contents with the below template:


<?xml version="1.0" encoding="utf-8"?> 
<Project DefaultTargets="Build" ToolsVersion="12.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ImportGroup Label="PropertySheets">
  </ImportGroup>
  <PropertyGroup Label="UserMacros" />
  <PropertyGroup />
  <ItemDefinitionGroup />
  <ItemGroup />
</Project>
