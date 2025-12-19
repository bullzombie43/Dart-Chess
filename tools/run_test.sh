
skill_level=0
engine_name="MyEngine"

while getopts "s:n:" flag; do
 case $flag in
   s) # Handle the skill level
   skill_level=$OPTARG
   ;;
   n) # Handle the name
   engine_name=$OPTARG
   ;;
   \?)
   # Handle invalid options
   ;;
 esac
done


cd ./native
cd ./build
cmake --build . --target chess_engine_uci

cd ..

cp ./build/chess_engine_uci "./test_versions/${engine_name}"

cutechess-cli \
  -engine name="$engine_name" cmd=/Users/justin/VSCODE\ PROJECTS/chess_ui/native/build/chess_engine_uci proto=uci \
  -engine name=StockfishL0 cmd=/opt/homebrew/bin/stockfish proto=uci option."Skill Level"="$skill_level" \
  -each tc=40/60+0.6 \
  -rounds 100 \
  -repeat \
  -recover \
  -pgnout "./logs/${engine_name}_vs_sf${skill_level}.pgn" \
  2>&1 | tee "./logs/${engine_name}_vs_sf${skill_level}.log"

