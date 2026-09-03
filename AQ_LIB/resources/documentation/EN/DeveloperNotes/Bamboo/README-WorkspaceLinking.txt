How to Create a Folder ("TheFolder") that Links to another folder ("TheLinkFolder")
-----------------------------------------------------------------------------------
We are using this here to link workspaces


FROM ADMINISTRATOR COMMAND LINE
-------------------------------

SYNTAX
------
mklink /D TheFolder "TheLinkFolder"

EXAMPLE
-------
mklink /D MQA-ENDOFDAY-MHSCTOK "D:\bamboo-agent-home\xml-data\build-dir\MQA-MQA-MHSCTOK"
mklink /D MQA-EDT-MHSCTOK "D:\bamboo-agent-home\xml-data\build-dir\MQA-MQA-MHSCTOK"