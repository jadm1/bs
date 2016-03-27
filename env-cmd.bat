@echo off
cd %~dp0
rem set python path
set PATH=C:\Anaconda2;C:\Anaconda2\Scripts;%PATH%
rem add dlls to the environment
set PATH=%~dp0\dlls;%PATH%
rem add the mingw gcc compiler to the environment (remove if already in PATH)
set PATH=D:\mingw\w64\i686-5.3.0-posix-dwarf-rt_v4-rev0\mingw32\bin;%PATH%
cmd
