To compile the code, run makefile.
File name- Plot.png
pairing of nodes: 
0-4
1-5
2-6
3-7
As seen in the bar chart, most of the times, 1-4 pair show high standard deviation.
This might be indicative of the network topology , implying that 0 and 4 are not directly connected. 


1st sample of timings are excluded from mean and std deviation calculations as it adds paddings the 1st time it sends messages. 

The 1st messages size 16 is ignored as it takes time to warm up the network and find the routes. Post populating the network topology it works fine. 

As message size increases, latency increases, and mean deviation decreases. As they get stabilized. 

Odd data points - msg size 128, 512,65536, 131072,262144. These points show in pair, few odd points, this might be due to network topology and traffic on network at that point in time.
