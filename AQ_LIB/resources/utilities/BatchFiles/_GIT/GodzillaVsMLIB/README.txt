INSTRUCTIONS
------------
GodzillaVsMLIB is strictly an internal Quant tool to be used to reset the licence expiry date on expired MLIB Excel Add-Ins.
Granting the end-user a full licence term, which is usually 1 or 3 months depending on which Excel Add-In is being used.

To use the tool drag and drop the *xll file ontop of the GodzillaVsMLIB.exe. The executible will create a back-up of the original
*.xll file and append the orginal file with a *.bak suffix. the *.xll file generated will now have a fully refreshed licence.

Alternatively the executible can be called from the command-line with the name of the *.xll passed as the first argument if the
*.xll is in the same folder as GodzillVsMLIB.exe, if the *.xll is not in the same folder then the full file path to the *.xll 
file must be provided.