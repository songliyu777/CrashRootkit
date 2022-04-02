@echo off
:: $Id$
setlocal
:: Perform post-build steps
:: An example follows on the next two lines ...
copy /y ".\obj%BUILD_ALT_DIR%\i386\YYFD.sys" "..\Debug\dtsoftbus02.sys"
copy /y ".\obj%BUILD_ALT_DIR%\i386\YYFD.pdb" "..\Debug\dtsoftbus02.pdb"
endlocal