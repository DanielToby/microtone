# microtone
A tiny (445 KB!) macOS midi synthesizer written in pure C++17.

The goal of this project is to create a tiny polyphonic synthesizer capable of reading midi input, accessible from the command line.

I wanted an opportunity to try out std::audio, implemented in this [exciting new proposal](https://github.com/stdcpp-audio/libstdaudio). I discovered std::audio when watching this [fantastic C++Now presentation by Timur Doumler.](https://www.youtube.com/watch?v=jNSiZqSQis4).

The audio synthesis techniques demonstrated in this project can be attributed to [The Audio Programming Book by Richard Boulanger](https://mitpress.mit.edu/books/audio-programming-book).
