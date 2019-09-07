# MemUsageTracker
Light-weight script to track memory usage overtime by monitoring the highest water mark periodically. The script is implemented to be as efficient and light-weight (use less memory resources) as possible. 

## Build
* On Linux:
  ```
  $make
  ```
  or
  ```
  $make all
  ```
 
## Run
* On Linux:
  ```
  $./TrackMemUage <PID> <interval (optional)>
  ```
  For example, in order to run the script to monitor memory usage by a process with process ID 2180 every 15 seconds (10 seconds is default if no time is explicitly mentioned):
   ```
  $./TrackMemUage 2180 15
  ```
