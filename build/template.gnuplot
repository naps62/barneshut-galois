#	Initial configurations
set term postscript eps enhanced clip
set output "<exec>.eps"

rgb(r,g,b) = r * 0xffff + g * 0xff + b;

#	Plotting
splot "<datafile>" using 1:2:3:(rgb($1,$2,$3)) with lines lc rgb variable;
