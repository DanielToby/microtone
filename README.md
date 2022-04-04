# microtone
A tiny (445 KB!) polyphonic synthesizer for macOS that reads MIDI input.

The goal of this project is to create a tiny polyphonic synthesizer capable of reading midi input, accessible from the command line.

I wanted an opportunity to try out std::audio, implemented [here](https://github.com/stdcpp-audio/libstdaudio). I discovered std::audio when watching this [fantastic C++Now presentation by Timur Doumler](https://www.youtube.com/watch?v=jNSiZqSQis4).

The audio synthesis techniques demonstrated in this project can be attributed to [The Audio Programming Book by Richard Boulanger](https://mitpress.mit.edu/books/audio-programming-book).

Midi input is handled by the [RtMidi library](https://www.music.mcgill.ca/~gary/rtmidi/).

I used [this paper on the interpretation of midi velocity](https://www.cs.cmu.edu/~rbd/papers/velocity-icmc2006.pdf) to determine an appropriate dynamic range (60dB) and calculate the amplitude of each midi note.
