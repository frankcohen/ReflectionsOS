#! /bin/bash
cd /Users/frankcohen/Desktop/ReflectionsExperiments/ReadyForTrial
for i in *.mp4; do /Applications/ffmpeg -i "$i" -vf "fps=15,scale=-1:240:flags=lanczos,crop=240:in_h:(in_w-240)/2:0" -q:v 9 "/Users/frankcohen/Desktop/ReflectionsExperiments/Rendered/${i%.*}.mjpeg"; done
for i in *.mov; do /Applications/ffmpeg -i "$i" -vf "fps=15,scale=-1:240:flags=lanczos,crop=240:in_h:(in_w-240)/2:0" -q:v 9 "/Users/frankcohen/Desktop/ReflectionsExperiments/Rendered/${i%.*}.mjpeg"; done
for i in *.wav; do ffmpeg -i "$i" -c:a libfdk_aac -b:a 8k "/Users/frankcohen/Desktop/ReflectionsExperiments/Rendered/${i%.*}.m4a"; done
for i in *.m4a; do ffmpeg -i "$i" -c:a libfdk_aac -b:a 8k "/Users/frankcohen/Desktop/ReflectionsExperiments/Rendered/${i%.*}.m4a"; done
tar czf "/Users/frankcohen/Desktop/ReflectionsExperiments/Rendered/show.tar.gz" "/Users/frankcohen/Desktop/ReflectionsExperiments/show"
