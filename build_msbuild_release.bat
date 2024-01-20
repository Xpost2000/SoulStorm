@call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
MSBuild.exe BulletHell\BulletHell.sln /property:Configuration=Release || exit /b !ERRORLEVEL!
@call "distribute_game.bat"
