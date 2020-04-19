# heatmap for lake.cu

set terminal png

set xrange[0:1]
set yrange[-1:1]
set cbrange[-0.4:0.4]

set output 'lake_c.png'
plot '<cat lake_c_0.dat lake_c_1.dat' using 1:2:3 with image
