# Ej 1
grep "model name" /proc/cpuinfo | head -n 1 | cut -d: -f2 | xargs echo

# Ej 2
grep "model name" /proc/cpuinfo | wc -l

# Ej 3
wget -O- -q https://raw.githubusercontent.com/dariomalchiodi/superhero-datascience/master/content/data/heroes.csv | cut -d ";" -f2 | \grep "\S" | tr "[:upper:]" "[:lower:]" | sed "s/ /_/g"

# Ej 4
sort -k5 -n weather_cordoba.in | tail -n 1 | cut -d " " -f1,2,3 | sed "s/ /\//g"
sort -k6 -n weather_cordoba.in | head -n 1 | cut -d " " -f1,2,3 | sed "s/ /\//g"

# Ej 5
sort -k3 -n atpplayers.in

# Ej 6
awk '{ print $2, $7 - $8, $0 }' superliga.in | sort -n | cut -d " " -f 3-

# Ej 7
ip address show | \grep --color ether | xargs echo | cut -d " " -f2

# Ej 8
# setup:
mkdir fma && for i in {1..11}; do touch "fma/fma_S01E${i}_es.srt"; done
for i (./fma/*_es.srt) mv $i $(sed -E "s/_es.srt$/.srt/" <<< $i)

# Ej 9a
ffmpeg -i in.mp4 -ss 00:00:5 -to 00:00:23 -c copy out.mp4

# Ej 9b
ffmpeg -i in1.mkv -i in2.mkv -filter_complex "[0][1]amix=inputs=2:duration=longest" out.mkv
