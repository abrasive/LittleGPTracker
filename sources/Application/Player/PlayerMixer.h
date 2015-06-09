
#ifndef _APPLICATION_MIXER_H_
#define _APPLICATION_MIXER_H_

#include "Framework/Instances/T_Singleton.h"
#include "Application/Model/Project.h"
#include "Application/Views/ViewData.h"
#include "Application/Utils/fixed.h"
#include "Application/Audio/AudioFileStreamer.h"
#include "PlayerChannel.h"
#include "Framework/Notifications/Observable.h"
#include "Services/Audio/AudioOut.h"

#define STREAM_MIX_BUS 8

class PlayerMixer: public T_Singleton<PlayerMixer>,public Observable,public Observer {
public:
	PlayerMixer() ;
	virtual ~PlayerMixer() {} ;

	bool Start() ;
	void Stop() ;
	bool Init(Project *project) ;
	void Close() ;

	void OnPlayerStart() ;
	void OnPlayerStop() ;

	void StartInstrument(int channel,I_Instrument *instrument,unsigned char note,bool newInstrument) ;
	void StopInstrument(int channel) ;

	int GetChannelNote(int Channel) ;

	I_Instrument *GetInstrument(int channel) ;

	I_Instrument *GetLastInstrument(int channel) ;
	
	void StartChannel(int channel) ;
	void StopChannel(int channel) ;

	bool IsChannelPlaying(int channel) ;
	
	void StartStreaming(const Path &) ;
	void StopStreaming()  ;

	bool Clipped() ;

	virtual void ObserverUpdate(Observable &o,ObservableData *d) ;
	int GetPlayedBufferPercentage() ;   

	void SetChannelMute(int channel,bool mute) ;
	bool IsChannelMuted(int channel) ;

	char *GetPlayedNote(int channel) ;
	char *GetPlayedOctive(int channel) ;
	
	AudioOut *GetAudioOut() ;

	void Lock() ;
	void Unlock() ;

private:

	Project *project_ ;
	bool clipped_ ;
	
    I_Instrument *lastInstrument_[SONG_CHANNEL_COUNT] ;
	bool isChannelPlaying_[SONG_CHANNEL_COUNT] ;

	AudioFileStreamer fileStreamer_ ;
	PlayerChannel *channel_[SONG_CHANNEL_COUNT] ;

	// store trigger notes, 0xFF = none
	
    unsigned char notes_[SONG_CHANNEL_COUNT] ;
} ;

#endif
