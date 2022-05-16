# microtone
A tiny cross-platform polyphonic synthesis library with MIDI support.

<p align="center">
<iframe width="560" height="315" src="https://www.youtube.com/embed/3SpMUx2KE4o" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>
</p>

I've been fascinated with digital signal processing and real-time audio for a long time. I'm a musician, and wanted to combine my love of the piano with my understanding of C++. I'd heard of frameworks like JUCE, but wanted to try my hand at a lower level approach. I owe the foundation of my ideas to this [fantastic C++Now presentation by Timur Doumler](https://www.youtube.com/watch?v=jNSiZqSQis4).

Unfortunately std::audio is still in early stages, so I looked to [portaudio](http://www.portaudio.com/) for a more robust real-time audio callback.

The audio synthesis techniques demonstrated in this project can be attributed to [The Audio Programming Book by Richard Boulanger](https://mitpress.mit.edu/books/audio-programming-book) and tutorials by [Bela](https://learn.bela.io/using-bela/languages/c-plus-plus/).

Midi input is handled by the [RtMidi library](https://www.music.mcgill.ca/~gary/rtmidi/).

## Features
- Wavetable oscillation that supports fill functions as lambdas. Wavetables are passed into the microtone::Synthesizer constructor, with adjustable weights. This data is shared by the oscillators.
- Polyphony -- 127 voices, each wrapping an oscillator. This is probably overkill (try playing 127 notes at the same time!), but only active notes on the midi keyboard will trigger the voices
- Envelopes (Attack, Decay, Sustain, Release): The oscillators belonging to each voice conform to configurable envelopes. Without this, you'd hear clicks and pops when notes are released or pressed in rapid succession -- at least in continuous functions like sine waves. This also adds richness and character to the sound.
- Filters (low-pass, high-pass, etc).
- Forwarded audio buffers -- update your UI with live audio data by passing a lambda to the microtone::Synthesizer constructor.
- Midi input, including the sustain pedal.

# asciiboard
A console application that manages an instance of microtone and displays midi input and audio output through a piano roll and oscilloscope.

Another dream of mine was to write a tiny synthesizer for use in the terminal. I thought it'd be neat to spin up a little executable instead of waiting on some heavy-weight DAW every time I wanted to play the piano.

Microtone can be used for a lot more than what you see in asciiboard. If I find the time, I'd love to build Microtone into a larger API, one that I could use to create all sorts of interesting audio tools. For now it's just a synthesis API.
