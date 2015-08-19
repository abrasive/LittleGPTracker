#include "S60System.h"
#include "Adapters/Unix/FileSystem/UnixFileSystem.h"
#include "Adapters/SDL/GUI/GUIFactory.h"
#include "Adapters/SDL/GUI/SDLGUIWindowImp.h"
#include "Adapters/SDL/GUI/SDLEventManager.h"
#include "Adapters/Unix/Process/UnixProcess.h"
#include "Adapters/SDL/Audio/SDLAudio.h"
#include "Adapters/Dummy/Midi/DummyMidi.h"
#include "Externals/TinyXML/tinyxml.h"
#include "Application/Model/Config.h"
#include "Application/Controllers/ControlRoom.h"
#include "Application/Player/SyncMaster.h"
#include "Application/Commands/NodeList.h"
#include "Adapters/SDL/Timer/SDLTimer.h"
#include "System/Console/Logger.h"
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <sys/time.h> 
#include <stdlib.h>

#define Time E32Time
#include <e32std.h>

EventManager *S60System::eventManager_ = NULL ;

int S60System::MainLoop() {
  eventManager_->InstallMappings() ;
  eventManager_->MainLoop() ;
} ;

void S60System::Boot(int argc,char **argv) {

  // Install System
  System::Install(new S60System()) ;

  // Install FileSystem
  FileSystem::Install(new UnixFileSystem()) ;
  Config::GetInstance()->ProcessArguments(argc,argv) ;

  // XXX debug to SD card
  unlink("e:/lgpt.log");
  Path logPath("e:\\lgpt.log");
  FileLogger *fileLogger=new FileLogger(logPath);
  if(fileLogger->Init().Succeeded())
  {
    Trace::GetInstance()->SetLogger(*fileLogger);    
  }

  // Install GUI Factory
  I_GUIWindowFactory::Install(new GUIFactory()) ;

  // Install Timers
  TimerService::GetInstance()->Install(new SDLTimerService()) ;

  // Install Sound
  AudioSettings hints ;
  hints.bufferSize_=1024 ;
  hints.preBufferCount_=8 ;
  Audio::Install(new SDLAudio(hints)) ;
  // Install Midi
  MidiService::Install(new DummyMidi()) ;

  // Install Threads
  SysProcessFactory::Install(new UnixProcessFactory()) ;

  // XXX this conflicts with the SDLEventManager
  if ( SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0 )   {
    return;
  }
  Path::SetAlias("root","e:/lgpt") ;
  Path::SetAlias("bin","e:/lgpt") ;

  SDL_EnableUNICODE(1);

  atexit(SDL_Quit);

  eventManager_=I_GUIWindowFactory::GetInstance()->GetEventManager() ;
  eventManager_->Init() ;

  // defaults suitable for qwertyphones eg. E63
  eventManager_->MapAppButton("q", APP_BUTTON_A);
  eventManager_->MapAppButton("a", APP_BUTTON_B);
  eventManager_->MapAppButton("left", APP_BUTTON_LEFT);
  eventManager_->MapAppButton("right", APP_BUTTON_RIGHT);
  eventManager_->MapAppButton("up", APP_BUTTON_UP);
  eventManager_->MapAppButton("down", APP_BUTTON_DOWN);
  eventManager_->MapAppButton("home", APP_BUTTON_L);        // green call key
  eventManager_->MapAppButton("space", APP_BUTTON_R);       // left softkey
  eventManager_->MapAppButton("escape", APP_BUTTON_START);  // right softkey
} ;

void S60System::Shutdown() {
  delete Audio::GetInstance() ;
  exit(0);
} ;

static int      secbase; 

unsigned long S60System::GetClock() {
  struct timeval tp;

  gettimeofday(&tp, NULL);  
  if (!secbase)
  {
    secbase = tp.tv_sec;
    return long(tp.tv_usec/1000.0);
  }
  return long((tp.tv_sec - secbase)*1000 + tp.tv_usec/1000.0);
}

int S60System::GetBatteryLevel() {
  return -1;
} ;

void S60System::Sleep(int millisec) {
  User::After(millisec*1000);
}

void *S60System::Malloc(unsigned size) {
  void *ptr=malloc(size) ;
  return ptr ;
}

void S60System::Free(void *ptr) {
  free(ptr) ;
} 

//extern "C" void *gpmemset(void *s1,unsigned char val,int n) ;

void S60System::Memset(void *addr,char val,int size) 
{
  memset(addr,val,size) ;
} ;

void *S60System::Memcpy(void *s1, const void *s2, int n)
{
  return memcpy(s1,s2,n);
}

void S60System::PostQuitMessage() 
{
  Trace::Log("S60", "PostQuitMessage called");
  SDLEventManager::GetInstance()->PostQuitMessage() ;
}

unsigned int S60System::GetMemoryUsage() 
{
  return 0;
}