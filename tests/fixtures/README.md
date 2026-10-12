# Synthetic media fixture

`h264-aac.mp4` contains four seconds of FFmpeg testsrc2 imagery and a generated 440 Hz sine wave. No third-party footage is included.

Reproduce:

```sh
ffmpeg -f lavfi -i testsrc2=size=160x90:rate=15 -f lavfi -i sine=frequency=440:sample_rate=44100 -t 4 -c:v libx264 -profile:v baseline -pix_fmt yuv420p -c:a aac -b:a 48k -movflags +faststart h264-aac.mp4
```

H.264 Constrained Baseline, yuv420p, 160×90, 15 fps; AAC-LC mono at 44.1 kHz. The Actions playback test is muted and does not claim to verify audible sound.
