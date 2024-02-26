@call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
rem @call "build_and_run_docgen.bat"
MSBuild.exe BulletHell\BulletHell.sln /property:Configuration=ReleaseWithDebugInfo || exit /b !ERRORLEVEL!
