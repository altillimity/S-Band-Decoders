# S-Band-Decoders
A bunch of programs to decode satellites in S-Band

### Dependencies

Some projects will require (or / and) :
- [libccsds](https://github.com/altillimity/libccsds)
- [libcorrect](https://github.com/quiet/libcorrect)
- [libpng](https://github.com/glennrp/libpng) + [zlib](https://github.com/madler/zlib)
- [libjpeg](https://github.com/thorfdbg/libjpeg)

The flowcharts require GNU Radio 3.8 or above.

# Proba-1

**Supported downlink :** Dumps, 2235Mhz    
**Modulation :** BPSK  
**Symbolrate :** 2.048Mbps  
**Recording bandwidth :** >= 3MSPS, something like 4MSPS is preferred  

**Decoding :**
- Record a baseband of a pass
- Demodulate with Proba-1-2 Demodulator   
- Process with Proba Viterbi Decoder to get CADUs   
- TBD - Data decoder coming later   

# Proba-2

**Supported downlink :** Dumps, 2235Mhz    
**Modulation :** BPSK  
**Symbolrate :** 2.048Mbps  
**Recording bandwidth :** >= 3MSPS, something like 4MSPS is preferred  

**Decoding :**
- Record a baseband of a pass
- Demodulate with Proba-1-2 Demodulator   
- Process with Proba Viterbi Decoder to get CADUs   
- Run through Proba Decoder in Proba-2 mode (Proba-Decoder 2 proba2.cadu)

# Proba-V

**Supported downlink :** Dumps, 2235Mhz    
**Modulation :** BPSK  
**Symbolrate :** 1.919Mbps  
**Recording bandwidth :** >= 3MSPS, something like 4MSPS is preferred  

**Decoding :**
- Record a baseband of a pass
- Demodulate with Proba-V Demodulator   
- Process with Proba Viterbi Decoder to get CADUs in Proba-V mode (-v)   
- TBD - Data decoder coming later  