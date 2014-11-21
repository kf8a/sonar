sonar
=====

An arduino sonar project inspired by Ken Fishers 
water level sensors (http://www.cigrjournal.org/index.php/Ejounral/article/viewFile/2343/1823) , but using the MaxBot sonar sensors instead of the Parallax
sensor and the adafruit datalogger shield instead of the sparkfun SD card shield. We used a Caron marine outlet box e989nnj to house
the sensor assembly. The outlet box was modified by drilling a hole to accept the 3/4 inch  E996E box adapter and the 
sensor was 
screwed into an E942E 3/4 inch PVC female adapter. 

The Sketch uses the modified RCTlib https://github.com/kf8a/RTClib and the DSSCircuits I2C library https://github.com/DSSCircuits/I2C-Master-Library as well as the https://github.com/n0m1/Sleep_n0m1 library for the 
power down sleep.
