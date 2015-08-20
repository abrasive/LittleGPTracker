#include "S60AudioDriver.h"
#include <math.h>
#include "System/Console/Trace.h"
#include "System/System/System.h"
#include "Services/Midi/MidiService.h"
#include <assert.h>

S60AudioDriver::S60AudioDriver(AudioSettings &settings):AudioDriver(settings) {
}

S60AudioDriver::~S60AudioDriver() {
}

bool S60AudioDriver::InitDriver() {
    return true;
}    

void S60AudioDriver::CloseDriver() {

} ;

bool S60AudioDriver::StartDriver() {
    thread_=new S60AudioDriverThread(this) ;
    thread_->Start() ;
    return true ;
} ; 

void S60AudioDriver::StopDriver() {
} ;

double S60AudioDriverThread::GetStreamTime() {
    return streamTime_;
}

double S60AudioDriver::GetStreamTime() {
    if (!thread_)
        return 0;
    return thread_->GetStreamTime() ;
} ;

bool S60AudioDriverThread::Execute() {
    _LIT(KThreadName,"lgpt_audio_thread");
    TPtrC threadname(KThreadName);
    User::RenameThread(threadname);

    RProcess cur_process;
    cur_process.SetPriority(EPriorityHigh);
    RThread cur_thread;
    cur_thread.SetPriority(EPriorityAbsoluteHigh);
    CActiveScheduler* activeScheduler = new (ELeave) CActiveScheduler;
    CleanupStack::PushL(activeScheduler);
    CActiveScheduler::Install(activeScheduler);
    dev = CMMFDevSound::NewL();
    dev->InitializeL(*this, KMMFFourCCCodePCM16, EMMFStatePlaying);
    CActiveScheduler::Start();
    CleanupStack::PopAndDestroy(activeScheduler);
    return true;
}

void S60AudioDriverThread::RequestTermination() {
}

void S60AudioDriverThread::InitializeComplete(TInt aError) {
    TInt err;
    TMMFPrioritySettings pri;
    pri.iPref = EMdaPriorityPreferenceTimeAndQuality;
    pri.iPriority = EMdaPriorityMax;
    TRAP(err, dev->SetPrioritySettings(pri));
    TMMFCapabilities caps = dev->Config();
    caps.iChannels = EMMFStereo;
    caps.iEncoding = EMMFSoundEncoding16BitPCM;
    caps.iRate = EMMFSampleRate44100Hz;
    caps.iBufferSize = 0;
    TRAP(err, dev->SetConfigL(caps));
    TRAP(err, dev->PlayInitL());
} ; 

void S60AudioDriver::FillBuffer(CMMFBuffer *aBuffer) {
    CMMFDataBuffer *buf = static_cast<CMMFDataBuffer*>(aBuffer);
    TDes8 &output = buf->Data();
    AudioBufferData *abd = &(pool_[poolPlayPosition_]);
    if (!abd->size_) {
        OnNewBufferNeeded();
        while (!abd->size_);    // XXX spinning!
    }
    output.Copy((TUint8*)abd->buffer_, abd->size_);
    SAFE_FREE(pool_[poolPlayPosition_].buffer_);
    poolPlayPosition_=(poolPlayPosition_+1)%SOUND_BUFFER_COUNT;
    output.SetLength(abd->size_);
    
    onAudioBufferTick();
    OnNewBufferNeeded();
}

void S60AudioDriverThread::BufferToBeFilled(CMMFBuffer *aBuffer) {
    driver->FillBuffer(aBuffer);
    dev->EmptyBuffers();
    TInt err;
    TRAP(err, dev->PlayData());
    streamTime_ = dev->SamplesPlayed() / 44100;
}

void S60AudioDriverThread::PlayError(TInt aError) {
}
