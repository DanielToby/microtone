# microtone
A cross-platform polyphonic synthesis library and MIDI player.

[![Watch the demo!](https://user-images.githubusercontent.com/41296254/170831269-3c965d7d-9fa6-4e9e-9729-c81c28f8a52b.png)](https://youtu.be/3SpMUx2KE4o)

[![Another Demo](assets/demo-thumbnail.png)](https://youtu.be/H49p-wMduN0)

### Build Steps
```sh
git clone git@github.com:DanielToby/microtone.git
mkdir build-microtone-release && cd build-microtone-release
cmake ../microtone -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
./Asciiboard/asciiboard
```

### About

This is a lightweight wavetable synthesizer with a few extra DSP features. I wanted a zippy synth that I could spin up for fun, or use as a building block for other stuff.

Unfortunately std::audio is still in early stages, so I used [portaudio](http://www.portaudio.com/) for a more robust real-time audio callback.

The audio synthesis techniques demonstrated in this project can be attributed to [The Audio Programming Book by Richard Boulanger](https://mitpress.mit.edu/books/audio-programming-book) and tutorials by [Bela](https://learn.bela.io/using-bela/languages/c-plus-plus/).

Midi input is handled by the [RtMidi library](https://www.music.mcgill.ca/~gary/rtmidi/).

### Audio Architecture

Libraries in this repo are designed to reduce coupling between synthesis, midi, audio output, and UI.
![Architecture](assets/architecture.png)

### Features
- Wavetable oscillation that supports fill functions as lambdas. Wavetables are passed into the synth::Synthesizer constructor with adjustable weights. This data is shared between the oscillators.
- Polyphony -- 127 voices, each wrapping an oscillator.
- Envelopes (Attack, Decay, Sustain, Release): The oscillators belonging to each voice conform to configurable envelopes. Without this, you'd hear clicks and pops when notes are released or pressed in rapid succession -- at least in continuous functions like sine waves. This also adds richness and character to the sound.
- Filters (low-pass, high-pass, etc).
- Forwarded audio buffers -- update your UI with live audio data by passing a lambda to the synth::Synthesizer constructor.
- Midi input, including the sustain pedal.

# asciiboard
A console application that manages an instance of microtone. It displays midi input and audio output through a piano roll and oscilloscope, and includes an envelope editor in a separate tab.
