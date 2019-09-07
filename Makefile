# Rules to compile memory usage script

CC = clang

all:
	$(CC) -o TrackMemUsage MemUsageTracker.c Stringlib.c

clean:
	rm TrackMemUsage
	
	
