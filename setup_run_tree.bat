@rem run previous script targets first, like the bigfile packer.
@rem this script does not check for the existence of some files.
Bigfilepacker.exe data.bigfile res/ stages/
mkdir run-tree
copy data.bigfile run-tree
copy game.exe run-tree
copy icon.ico run-tree
copy lua_engine_api.html run-tree
copy gamecontrollerdb.txt run-tree
copy *.dll run-tree
