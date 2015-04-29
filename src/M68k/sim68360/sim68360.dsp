# Microsoft Developer Studio Project File - Name="sim68360" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=sim68360 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sim68360.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sim68360.mak" CFG="sim68360 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sim68360 - Win32 Release" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "sim68360 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sim68360 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\Framework" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "USE_STD" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "sim68360 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "sim68360 - Win32 Release"
# Name "sim68360 - Win32 Debug"
# Begin Source File

SOURCE=..\Framework\AddressSpace.cxx
# End Source File
# Begin Source File

SOURCE=..\Framework\AddressSpace.hxx
# End Source File
# Begin Source File

SOURCE=..\Framework\BasicCPU.cxx
# End Source File
# Begin Source File

SOURCE=..\Framework\BasicCPU.hxx
# End Source File
# Begin Source File

SOURCE=..\Framework\BasicDevice.cxx
# End Source File
# Begin Source File

SOURCE=..\Framework\BasicDevice.hxx
# End Source File
# Begin Source File

SOURCE=..\Framework\BasicDeviceRegistry.cxx
# End Source File
# Begin Source File

SOURCE=..\Framework\BasicDeviceRegistry.hxx
# End Source File
# Begin Source File

SOURCE=..\Framework\BasicLoader.hxx
# End Source File
# Begin Source File

SOURCE=..\Framework\BreakpointList.cxx
# End Source File
# Begin Source File

SOURCE=..\Framework\BreakpointList.hxx
# End Source File
# Begin Source File

SOURCE=.\cpu\cpu32.cxx
# End Source File
# Begin Source File

SOURCE=.\cpu\cpu32.hxx
# End Source File
# Begin Source File

SOURCE=.\cpu\cpu32DecodeTable.hxx
# End Source File
# Begin Source File

SOURCE=.\cpu\decode.cxx
# End Source File
# Begin Source File

SOURCE=.\devices\DeviceRegistry.cxx
# End Source File
# Begin Source File

SOURCE=.\devices\DeviceRegistry.hxx
# End Source File
# Begin Source File

SOURCE=..\Framework\Event.cxx
# End Source File
# Begin Source File

SOURCE=..\Framework\Event.hxx
# End Source File
# Begin Source File

SOURCE=.\cpu\exec.cxx
# End Source File
# Begin Source File

SOURCE=..\Framework\Interface.cxx
# End Source File
# Begin Source File

SOURCE=..\Framework\Interface.hxx
# End Source File
# Begin Source File

SOURCE=.\loader\Loader.cxx
# End Source File
# Begin Source File

SOURCE=.\loader\Loader.hxx
# End Source File
# Begin Source File

SOURCE=.\main.cxx
# End Source File
# Begin Source File

SOURCE=.\devices\RAM.cxx
# End Source File
# Begin Source File

SOURCE=.\devices\RAM.hxx
# End Source File
# Begin Source File

SOURCE=..\Framework\RegInfo.cxx
# End Source File
# Begin Source File

SOURCE=..\Framework\RegInfo.hxx
# End Source File
# Begin Source File

SOURCE=..\Framework\StatInfo.cxx
# End Source File
# Begin Source File

SOURCE=..\Framework\StatInfo.hxx
# End Source File
# Begin Source File

SOURCE=..\Framework\Time.cxx
# End Source File
# Begin Source File

SOURCE=..\Framework\Time.hxx
# End Source File
# Begin Source File

SOURCE=.\devices\Timer.cxx
# End Source File
# Begin Source File

SOURCE=.\devices\Timer.hxx
# End Source File
# Begin Source File

SOURCE=..\Framework\Tools.cxx
# End Source File
# Begin Source File

SOURCE=..\Framework\Tools.hxx
# End Source File
# End Target
# End Project
