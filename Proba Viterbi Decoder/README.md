# Proba Viterbi Decoder

Small program taking demodulated soft symbols from the Proba downlinks and outputs CADU frames out. -v means Proba-V mode

Usage : `./HRD-Decoder softsymbols.bin outputframes.bin [-v]`

The correlator, viterbi wrapper and RS (using libcorrect in the background) are both from https://github.com/opensatelliteproject/libsathelper.