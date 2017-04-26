call D:\IntelSWTools\compilers_and_libraries_2017\windows\bin\iclvars.bat intel64

D:\IntelSWTools\compilers_and_libraries_2017\windows\bin\Intel64\icl.exe /Qm64 /nologo /W3 /O2 /Oi /Qipo /D __INTEL_COMPILER=1700 /D WIN32 /D NDEBUG /D _CONSOLE /D _UNICODE /D UNICODE /EHsc /GS /Zc:wchar_t /Zc:forScope /TP \\nas\projects\puzzle\puzzle.cpp /link /OUT:"\\nas\projects\puzzle\puzzle.exe" /INCREMENTAL:NO /NOLOGO kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /SUBSYSTEM:CONSOLE /OPT:REF /OPT:ICF /TLBID:1 /DYNAMICBASE /NXCOMPAT /MACHINE:X64 /OUT:"\\nas\projects\puzzle\puzzle.exe"

copy \\nas\projects\puzzle\puzzle.exe "d:\cloud\google drive\puzzle\puzzle.exe"

