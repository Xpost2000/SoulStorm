g++ src/lua_metagen.cpp -o ./metagen
./metagen
echo "run pandoc"
pandoc -c ./retro.css -s lua_engine_api.md -o lua_engine_api.html
