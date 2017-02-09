This is a simple command-line utility that modifies the required OS and subsystem version fields in the header of a Windows PE (.exe) executable. The main purpose of this program is to get rid of the "<program> is not a valid Win32 application." error message when trying to run a program designed for Windows Vista or later on Windows XP. This can allow some programs that were compiled with Visual Studio 2015 without targeting Windows XP to run.

Usage: exeosver 'file' 'os_version'
where 'file' is a Windows .exe file you wish to patch and 'os_version' is the version of Windows you wish to target. If not specifed, 'os_version' defaults to 4.0.
