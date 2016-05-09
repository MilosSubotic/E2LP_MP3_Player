#!/bin/bash

ffmpeg -i 'Lindsey Stirling - Electric Daisy Violin.mp3' -acodec libmp3lame -ac 1 -ar 48000 -ab 128k 'Lindsey Stirling - Electric Daisy Violin - mono - 128kbps - 48kHz.mp3'

ffmpeg -i 'Lindsey Stirling - Electric Daisy Violin - mono - 128kbps - 48kHz.mp3' -acodec pcm_u8 'Lindsey Stirling - Electric Daisy Violin - mono - u8.wav'

ffmpeg -i 'Lindsey Stirling - Electric Daisy Violin - mono - 128kbps - 48kHz.mp3' -acodec pcm_s16le 'Lindsey Stirling - Electric Daisy Violin - mono - s16le.wav'
