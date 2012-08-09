#!bin/bash

RUNS="10";

BARNESHUT_N="100000";
BARNESHUT_BS="2048";
BARNESHUT_ERR="barneshut.err";

RAYTRACER_N="100";
RAYTRACER_SPP="10000";
RAYTRACER_BS="16";
RAYTRACER_D="1";
RAYTRACER_ERR="raytracer.err";

POINTCORR_N="10000";
POINTCORR_R="10";
POINTCORR_BS="2048";
POINTCORR_ERR="pointcorrelation.err";

# Clear previous results
rm "$BARNESHUT_ERR";
rm "$RAYTRACER_ERR";
rm "$POINTCORR_ERR";

R="0";
while [ "$R" -lt "$RUNS" ];
do
	# Barneshut (only parallel)
	echo >> "$BARNESHUT_ERR";
	rls/apps/barneshut/barneshut -n="$BARNESHUT_N" -ts=1 -t=16 2>> "$BARNESHUT_ERR";
	# Barneshut (parallel + sorted)
	echo >> "$BARNESHUT_ERR";
	rls/apps/barneshut/barneshut -n="$BARNESHUT_N" -ts=1 -t=16 -sort 2>> "$BARNESHUT_ERR";
	# Barneshut (parallel + sorted + point blocking)
	echo >> "$BARNESHUT_ERR";
	rls/apps/barneshut/barneshut -n="$BARNESHUT_N" -ts=1 -t=16 -sort -bs="$BARNESHUT_BS" 2>> "$BARNESHUT_ERR";


	# Ray Tracer (only parallel)
	echo >> "$RAYTRACER_ERR";
	rls/apps/28_raytracernotblocked/raytracernotblocked -n="$RAYTRACER_N" -spp="$RAYTRACER_SPP" -d="$RAYTRACER_D" >> "$RAYTRACER_ERR";
	# Ray Tracer (parallel + sorted)
	echo >> "$RAYTRACER_ERR";
	rls/apps/28_raytracernotblocked/raytracernotblocked -n="$RAYTRACER_N" -spp="$RAYTRACER_SPP" -d="$RAYTRACER_D" -sort >> "$RAYTRACER_ERR";
	# Ray Tracer (parallel + sorted + point blocking)
	echo >> "$RAYTRACER_ERR";
	rls/apps/27_raytracerblocked/raytracerblocked -n="$RAYTRACER_N" -spp="$RAYTRACER_SPP" -d="$RAYTRACER_D" -b="$RAYTRACER_BS" >> "$RAYTRACER_ERR";


	# Two-Point Correlation (only parallel)
	echo >> "POINTCORR_ERR";
	rls/apps/pointcorrelation/pointcorrelation -n="$POINTCORR_N" -t=16 -r="$POINTCORR_R" 2>> "$POINTCORR_ERR";
	# Two-Point Correlation (parallel + sorted)
	echo >> "POINTCORR_ERR";
	rls/apps/pointcorrelation/pointcorrelation -n="$POINTCORR_N" -t=16 -r="$POINTCORR_R" -sort 2>> "$POINTCORR_ERR";
	# Two-Point Correlation (parallel + sorted + point blocking)
	echo >> "POINTCORR_ERR";
	rls/apps/pointcorrelation/pointcorrelation -n="$POINTCORR_N" -t=16 -r="$POINTCORR_R" -sort -bs="$POINTCORR_BS" 2>> "$POINTCORR_ERR";

	R=$(( $R + 1 ));
done;

BARNESHUT_ERR="papi_${BARNESHUT_ERR}";
RAYTRACER_ERR="papi_${RAYTRACER_ERR}";
POINTCORR_ERR="papi_${POINTCORR_ERR}";

PAPI_EVENT_NAME="L3_CACHE_MISSES:READ_BLOCK_EXCLUSIVE:ALL_CORES";

rm "$BARNESHUT_ERR";
rm "$RAYTRACER_ERR";
rm "$POINTCORR_ERR";

R="0";
while [ "$R" -lt "$RUNS" ];
do
	# Barneshut (only parallel)
	echo >> "$BARNESHUT_ERR";
	rls/apps/barneshut/barneshut -n="$BARNESHUT_N" -ts=1 -t=16 -papi="$PAPI_EVENT_NAME" 2>> "$BARNESHUT_ERR";
	# Barneshut (parallel + sorted)
	echo >> "$BARNESHUT_ERR";
	rls/apps/barneshut/barneshut -n="$BARNESHUT_N" -ts=1 -t=16 -sort -papi="$PAPI_EVENT_NAME" 2>> "$BARNESHUT_ERR";
	# Barneshut (parallel + sorted + point blocking)
	echo >> "$BARNESHUT_ERR";
	rls/apps/barneshut/barneshut -n="$BARNESHUT_N" -ts=1 -t=16 -sort -bs="$BARNESHUT_BS" -papi="$PAPI_EVENT_NAME" 2>> "$BARNESHUT_ERR";


	# Ray Tracer (only parallel)
	echo >> "$RAYTRACER_ERR";
	rls/apps/28_raytracernotblocked/raytracernotblocked -n="$RAYTRACER_N" -spp="$RAYTRACER_SPP" -d="$RAYTRACER_D" -papi >> "$RAYTRACER_ERR";
	# Ray Tracer (parallel + sorted)
	echo >> "$RAYTRACER_ERR";
	rls/apps/28_raytracernotblocked/raytracernotblocked -n="$RAYTRACER_N" -spp="$RAYTRACER_SPP" -d="$RAYTRACER_D" -sort -papi >> "$RAYTRACER_ERR";
	# Ray Tracer (parallel + sorted + point blocking)
	echo >> "$RAYTRACER_ERR";
	rls/apps/27_raytracerblocked/raytracerblocked -n="$RAYTRACER_N" -spp="$RAYTRACER_SPP" -d="$RAYTRACER_D" -b="$RAYTRACER_BS" -papi >> "$RAYTRACER_ERR";


	# Two-Point Correlation (only parallel)
	echo >> "POINTCORR_ERR";
	rls/apps/pointcorrelation/pointcorrelation -n="$POINTCORR_N" -t=16 -r="$POINTCORR_R" -papi="$PAPI_EVENT_NAME" 2>> "$POINTCORR_ERR";
	# Two-Point Correlation (parallel + sorted)
	echo >> "POINTCORR_ERR";
	rls/apps/pointcorrelation/pointcorrelation -n="$POINTCORR_N" -t=16 -r="$POINTCORR_R" -sort -papi="$PAPI_EVENT_NAME" 2>> "$POINTCORR_ERR";
	# Two-Point Correlation (parallel + sorted + point blocking)
	echo >> "POINTCORR_ERR";
	rls/apps/pointcorrelation/pointcorrelation -n="$POINTCORR_N" -t=16 -r="$POINTCORR_R" -sort -bs="$POINTCORR_BS" -papi="$PAPI_EVENT_NAME" 2>> "$POINTCORR_ERR";

	R=$(( $R + 1 ));
done;