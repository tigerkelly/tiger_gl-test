#!/bin/bash

# Restart raspivid command when it stops due to remote system stops viewing video.

rm -f restart_cam.log

while :
do
	# NOTE: 10 fps is the most we can do, else it adds too much delay in the video.
	#       This might be due to the wireless network, or the Pi Zero??
	#   -a = --annotate
	#   -b = --bitrate
	#   -t = --timeout (0 = forever)
	#   -n = --nopreview
	# -fps = framerate
	#   -l = --listen
	#   -o = --output
	#  -vs = --vstab
	#   -w = --width
	#   -h = --height
	# /usr/bin/raspivid -a 4 -a "%Y-%m-%d %X" -b 500000 -t 0 -n -fps 10 -l -o tcp://192.168.2.5:43210 -vs -w 1024 -h 720
	# Used when testing.
	vid -a 4 -a "%Y-%m-%d %X" -t 0 -n -cd MJPEG -fps 12 -l -o tcp://192.168.2.5:43210 -vs -w 800 -h 768
	# if this sleep is not present then very hard to stop script.
	sleep 1
done
