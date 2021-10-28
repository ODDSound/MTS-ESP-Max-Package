# MTS-ESP Max Package

The MTS-ESP library is a simple but versatile C/C++ library for adding microtuning support to audio and MIDI plugins:

[MTS-ESP Library on Github](http://github.com/ODDSound/MTS-ESP)

It allows for a single Master plugin to simultaneously control the tuning of any number of connected Client plugins across a DAW session.  Connection between a Master and Clients is automatic and invisible.  

This package includes objects that allow a Max patch or Max for Live device to use the MTS-ESP system and work as a client. 

A free master plugin, OddSound MTSMiniMaster, supports loading of .scl and .tun files and provides a simple way to start using the MTS-ESP system.

Note that when using these objects in Max for Live devices, individual devices will not appear as separate Clients to a Master plugin but rather Max for Live itself appears as a single Client.


## MTS-ESP.mtof

Any patch that receives and processes MIDI note data can be made compatible with MTS-ESP using the MTS-ESP.mtof object.

A Client patch can use the MTS-ESP.mtof object to query the re-tuning for a given MIDI note number either as an absolute frequency value or as the difference from the standard 12-TET tuning (i.e. 440*2^((midi_note-69) / 12)).  Ideally it should do this as often as possible whilst a note is being played or sound is being processed, not just when a note on is received, so that note frequencies can update in real-time along the flight of a note if the tuning is changed or automated in the Master plugin.  A bang at the left input of the MTS-ESP.mtof object will update the frequency and retuning values at the outlets.
 
When a note on message is received, a Client patch should check to see if the note should be ignored.  Doing so allows a Master plugin to define a keyboard map that includes unmapped keys.  The right outlet of the MTS-ESP.mtof device will output a 1 if a note should be played or 0 if it should be ignored.

When not connected to a Master plugin the MTS-ESP objects, and therefore a patch that implements them, will automatically revert to using 12-TET tuning.

Note that unlike the regular Max mtof object, the MTS-ESP.mtof object will convert float values received at the MIDI note inlet to int before calculating the output values.


## MTS-ESP.mtof~

A signal-rate version of the above with the difference that, for efficiency, it has no output informing whether the note should be played or not.

It is advised to use MTS-ESP.mtof when a note on is received to check whether it should be played and obtain the initial retuning amount, then use MTS-ESP.mtof~ in combination with a pitch modulation signal so that note frequencies can update in real-time if tuning is adjusted or automated in the Master plugin.


## MTS-ESP.ftom

An MTS-ESP.ftom object is supplied for converting a frequency to a MIDI note value.  The note value is output as an integer and is guaranteed to be mapped in the master plugin.  MTS-ESP supports multi-channel mappings and a second outlet provides the MIDI channel on which the note should be sent, if the device supports it.


For any queries, assistance or bug reports contact tech@oddsound.com.

