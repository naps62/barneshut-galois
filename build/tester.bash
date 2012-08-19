#!bin/bash

function maxwell () {
#	Arguments
PAPI_EVENT_NAME="$1";
#	Globals
THREADS="8";

echo "Maxwell measuring $PAPI_EVENT_NAME";

#	Barnes-hut
BH_EXEC="rls/apps/barneshut/barneshut";
BH_TS="1";
BH_N="1000";
BH_BS="512";
BH_ERR="bh_${PAPI_EVENT_NAME}.err";
#		Preparation
echo "Barnes-hut";
if [ -f "$BH_ERR" ]; then rm "$BH_ERR"; fi;
#		Parallel
echo "* Parallel";
$BH_EXEC -t="$THREADS" -ts="$BH_TS" -n="$BH_N" -papi="$PAPI_EVENT_NAME" 2>> "$BH_ERR";
#		Parallel + Sort
echo "* Parallel + Sort";
$BH_EXEC -t="$THREADS" -ts="$BH_TS" -n="$BH_N" -sort -papi="$PAPI_EVENT_NAME" 2>> "$BH_ERR";
#		Parallel + Sort + Point Blocking
echo "* Parallel + Sort + Point Blocking";
$BH_EXEC -t="$THREADS" -ts="$BH_TS" -n="$BH_N" -sort -bs="$BH_BS" -papi="$PAPI_EVENT_NAME" 2>> "$BH_ERR";

# echo;
# 
# #	Ray Tracer
# RT_W="2";
# RT_H="2";
# RT_N="1";
# RT_SPP="10000";
# RT_BS="1024";
# RT_D="1";
# RT_ERR="rt_${PAPI_EVENT_NAME}.err";
# #		Preparation
# echo "Ray Tracer";
# if [ -f "$RT_ERR" ]; then rm "$RT_ERR"; fi;
# #		Parallel
# echo "* Parallel";
# rls/apps/28_raytracernotblocked/raytracernotblocked -t="$THREADS" -w="$RT_W" -h="$RT_H" -n="$RT_N" -spp="$RT_SPP" -d="$RT_D" >> "$RT_ERR";
# #		Parallel + Sort
# echo "* Parallel + Sort";
# rls/apps/28_raytracernotblocked/raytracernotblocked -t="$THREADS" -w="$RT_W" -h="$RT_H" -n="$RT_N" -spp="$RT_SPP" -d="$RT_D" -sort >> "$RT_ERR";
# #		Parallel + Sort + Point Blocking
# echo "* Parallel + Sort + Point Blocking";
# rls/apps/27_raytracerblocked/raytracerblocked -t="$THREADS" -w="$RT_W" -h="$RT_H" -n="$RT_N" -spp="$RT_SPP" -d="$RT_D" -b="$RT_BS" >> "$RT_ERR";

echo;

#	Point Correlation
PC_EXEC="rls/apps/pointcorrelation/pointcorrelation";
PC_RADIUS="10";
PC_N="3500";
PC_BS="512";
PC_ERR="pc_${PAPI_EVENT_NAME}.err";
#		Preparation
echo "Point Correlation";
if [ -f "$PC_ERR" ]; then rm "$PC_ERR"; fi;
#		Parallel
echo "* Parallel";
$PC_EXEC -t="$THREADS" -r="$PC_RADIUS" -n="$PC_N" -papi="$PAPI_EVENT_NAME" 2>> "$PC_ERR";
#		Parallel + Sort
echo "* Parallel + Sort";
$PC_EXEC -t="$THREADS" -r="$PC_RADIUS" -n="$PC_N" -sort -papi="$PAPI_EVENT_NAME" 2>> "$PC_ERR";
#		Parallel + Sort + Point Blocking
echo "* Parallel + Sort + Point Blocking";
$PC_EXEC -t="$THREADS" -r="$PC_RADIUS" -n="$PC_N" -sort -bs="512" -papi="$PAPI_EVENT_NAME" 2>> "$PC_ERR";

}
#end of function maxwell



PAPI_EVENT_NAMES=("LLC_MISSES" "PAPI_L2_DCM" "PAPI_L1_DCM");

for papi_event_name in ${PAPI_EVENT_NAMES[@]};
do
	maxwell $papi_event_name;
done;

# RUNS="10";
# THREADS="24";
# 
# BARNESHUT_N="100000";
# BARNESHUT_BS="2048";
# BARNESHUT_ERR="barneshut.err";
# 
# RAYTRACER_W="2";
# RAYTRACER_H="2";
# RAYTRACER_N="100";
# RAYTRACER_SPP="10000";
# RAYTRACER_BS="16";
# RAYTRACER_D="1";
# RAYTRACER_ERR="raytracer.err";
# 
# POINTCORR_N="100000";
# POINTCORR_R="10";
# POINTCORR_BS="2048";
# POINTCORR_ERR="pointcorrelation.err";
# 
# # Clear previous results
# rm "$BARNESHUT_ERR";
# rm "$RAYTRACER_ERR";
# rm "$POINTCORR_ERR";
# 
# R="0";
# while [ "$R" -lt "$RUNS" ];
# do
# 	# Barneshut (only parallel)
# 	echo >> "$BARNESHUT_ERR";
# 	rls/apps/barneshut/barneshut -n="$BARNESHUT_N" -ts=1 -t="$THREADS" 2>> "$BARNESHUT_ERR";
# 	# Barneshut (parallel + sorted)
# 	echo >> "$BARNESHUT_ERR";
# 	rls/apps/barneshut/barneshut -n="$BARNESHUT_N" -ts=1 -t="$THREADS" -sort 2>> "$BARNESHUT_ERR";
# 	# Barneshut (parallel + sorted + point blocking)
# 	echo >> "$BARNESHUT_ERR";
# 	rls/apps/barneshut/barneshut -n="$BARNESHUT_N" -ts=1 -t="$THREADS" -sort -bs="$BARNESHUT_BS" 2>> "$BARNESHUT_ERR";
# 
# 
# 	# Ray Tracer (only parallel)
# 	echo >> "$RAYTRACER_ERR";
# 	rls/apps/28_raytracernotblocked/raytracernotblocked -t="$THREADS" -w="$RAYTRACER_W" -h="$RAYTRACER_H" -n="$RAYTRACER_N" -spp="$RAYTRACER_SPP" -d="$RAYTRACER_D" >> "$RAYTRACER_ERR";
# 	# Ray Tracer (parallel + sorted)
# 	echo >> "$RAYTRACER_ERR";
# 	rls/apps/28_raytracernotblocked/raytracernotblocked -t="$THREADS" -w="$RAYTRACER_W" -h="$RAYTRACER_H" -n="$RAYTRACER_N" -spp="$RAYTRACER_SPP" -d="$RAYTRACER_D" -sort >> "$RAYTRACER_ERR";
# 	# Ray Tracer (parallel + sorted + point blocking)
# 	echo >> "$RAYTRACER_ERR";
# 	rls/apps/27_raytracerblocked/raytracerblocked -t="$THREADS" -w="$RAYTRACER_W" -h="$RAYTRACER_H" -n="$RAYTRACER_N" -spp="$RAYTRACER_SPP" -d="$RAYTRACER_D" -b="$RAYTRACER_BS" >> "$RAYTRACER_ERR";
# 
# 
# 	# Two-Point Correlation (only parallel)
# 	echo >> "$POINTCORR_ERR";
# 	rls/apps/pointcorrelation/pointcorrelation -n="$POINTCORR_N" -t="$THREADS" -r="$POINTCORR_R" 2>> "$POINTCORR_ERR";
# 	# Two-Point Correlation (parallel + sorted)
# 	echo >> "$POINTCORR_ERR";
# 	rls/apps/pointcorrelation/pointcorrelation -n="$POINTCORR_N" -t="$THREADS" -r="$POINTCORR_R" -sort 2>> "$POINTCORR_ERR";
# 	# Two-Point Correlation (parallel + sorted + point blocking)
# 	echo >> "$POINTCORR_ERR";
# 	rls/apps/pointcorrelation/pointcorrelation -n="$POINTCORR_N" -t="$THREADS" -r="$POINTCORR_R" -sort -bs="$POINTCORR_BS" 2>> "$POINTCORR_ERR";
# 
# 	R=$(( $R + 1 ));
# done;
# 
# BARNESHUT_ERR="papi_${BARNESHUT_ERR}";
# RAYTRACER_ERR="papi_${RAYTRACER_ERR}";
# POINTCORR_ERR="papi_${POINTCORR_ERR}";
# 
# PAPI_EVENT_NAME="LLC_MISSES";
# 
# rm "$BARNESHUT_ERR";
# rm "$RAYTRACER_ERR";
# rm "$POINTCORR_ERR";
# 
# R="0";
# while [ "$R" -lt "$RUNS" ];
# do
# 	# Barneshut (only parallel)
# 	echo >> "$BARNESHUT_ERR";
# 	rls/apps/barneshut/barneshut -n="$BARNESHUT_N" -ts=1 -t="$THREADS" -papi="$PAPI_EVENT_NAME" 2>> "$BARNESHUT_ERR";
# 	# Barneshut (parallel + sorted)
# 	echo >> "$BARNESHUT_ERR";
# 	rls/apps/barneshut/barneshut -n="$BARNESHUT_N" -ts=1 -t="$THREADS" -sort -papi="$PAPI_EVENT_NAME" 2>> "$BARNESHUT_ERR";
# 	# Barneshut (parallel + sorted + point blocking)
# 	echo >> "$BARNESHUT_ERR";
# 	rls/apps/barneshut/barneshut -n="$BARNESHUT_N" -ts=1 -t="$THREADS" -sort -bs="$BARNESHUT_BS" -papi="$PAPI_EVENT_NAME" 2>> "$BARNESHUT_ERR";
# 
# 
# 	# Ray Tracer (only parallel)
# 	echo >> "$RAYTRACER_ERR";
# 	rls/apps/28_raytracernotblocked/raytracernotblocked -t="$THREADS" -w="$RAYTRACER_W" -h="$RAYTRACER_H" -n="$RAYTRACER_N" -spp="$RAYTRACER_SPP" -d="$RAYTRACER_D" -papi >> "$RAYTRACER_ERR";
# 	# Ray Tracer (parallel + sorted)
# 	echo >> "$RAYTRACER_ERR";
# 	rls/apps/28_raytracernotblocked/raytracernotblocked -t="$THREADS" -w="$RAYTRACER_W" -h="$RAYTRACER_H" -n="$RAYTRACER_N" -spp="$RAYTRACER_SPP" -d="$RAYTRACER_D" -sort -papi >> "$RAYTRACER_ERR";
# 	# Ray Tracer (parallel + sorted + point blocking)
# 	echo >> "$RAYTRACER_ERR";
# 	rls/apps/27_raytracerblocked/raytracerblocked -t="$THREADS" -w="$RAYTRACER_W" -h="$RAYTRACER_H" -n="$RAYTRACER_N" -spp="$RAYTRACER_SPP" -d="$RAYTRACER_D" -b="$RAYTRACER_BS" -papi >> "$RAYTRACER_ERR";
# 
# 
# 	# Two-Point Correlation (only parallel)
# 	echo >> "$POINTCORR_ERR";
# 	rls/apps/pointcorrelation/pointcorrelation -n="$POINTCORR_N" -t="$THREADS" -r="$POINTCORR_R" -papi="$PAPI_EVENT_NAME" 2>> "$POINTCORR_ERR";
# 	# Two-Point Correlation (parallel + sorted)
# 	echo >> "$POINTCORR_ERR";
# 	rls/apps/pointcorrelation/pointcorrelation -n="$POINTCORR_N" -t="$THREADS" -r="$POINTCORR_R" -sort -papi="$PAPI_EVENT_NAME" 2>> "$POINTCORR_ERR";
# 	# Two-Point Correlation (parallel + sorted + point blocking)
# 	echo >> "$POINTCORR_ERR";
# 	rls/apps/pointcorrelation/pointcorrelation -n="$POINTCORR_N" -t="$THREADS" -r="$POINTCORR_R" -sort -bs="$POINTCORR_BS" -papi="$PAPI_EVENT_NAME" 2>> "$POINTCORR_ERR";
# 
# 	R=$(( $R + 1 ));
# done;
