#/bin/sh
#EXEC="./apps/cleanbarneshut/cleanbarneshut";
EXEC="$1";
BODIES="$2";
STEPS="$3";

DATAFILE="tmp.dat";
PLOTFILE="tmp.gnuplot";

$EXEC -n="$BODIES" -steps="$STEPS" -out="1" | tail -n +14 | grep -v "STAT" | sed "s/^[[:digit:]]*, (\([^,]*\),\([^,]*\),\([^,]*\))$/\1\t\2\t\3/" 2>&-  > "$DATAFILE";

DIR=$(dirname "$0");
BASE=$(basename "$EXEC");

sed "s/<exec>/${BASE}/;s/<datafile>/${DATAFILE}/" "${DIR}/template.gnuplot" | gnuplot;

mv "$BASE" "${DIR}/";

rm "$PLOTFILE";
rm "$DATAFILE";

