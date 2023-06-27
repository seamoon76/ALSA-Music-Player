source /opt/st/myir/3.1-snapshot/environment-setup-cortexa7t2hf-neon-vfpv4-ostl-linux-gnueabi
$CC Music_App.c sonic.c sonic.h Equalizer.c Equalizer.h FirFilter.c FirFilter.h -I . -o Music_App -lasound -pthread -lm
