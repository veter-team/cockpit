#!/bin/sh

# Save screencast as theora ogg file with original window size
#gst-launch ximagesrc xname="Cockpit" ! ffmpegcolorspace ! theoraenc ! oggmux ! filesink location=$1.ogg

# Save screencast as 800x600 theora ogg file
#gst-launch ximagesrc xname="Cockpit" ! ffmpegcolorspace ! videoscale ! video/x-raw-yuv,width=800,height=600 ! theoraenc ! oggmux ! filesink location=$1.ogg

# Save screencast as webm file with original window size
#gst-launch ximagesrc xname="Cockpit" ! ffmpegcolorspace ! vp8enc ! webmmux ! filesink location=$1.webm

# Save screencast as 800x600 webm file
gst-launch ximagesrc xname="Cockpit" ! ffmpegcolorspace ! videoscale ! video/x-raw-yuv,width=800,height=600 ! vp8enc ! webmmux ! filesink location=$1.webm

