/////////////////////////////////////////////////////////////////////////////
// fnkeys.h:
// function key setup
// Author:
// copyright 2000 JAARS Inc. SIL
//
// Revision History
// 1.5Test8.3
//      SDM increased array size by one to allow use of
//        [24] by batch command processing 1.5Test8.3
// 1.5Test10.2
//      SDM increased array size by one to allow use of
//        [25] by SlowReplay processing
/////////////////////////////////////////////////////////////////////////////
#ifndef _FNKEYS_H
#define _FNKEYS_H

// function key setup

class Object_ostream;
class Object_istream;

class FnKeys 
{
public:
  // SDM increased array size by one to allow use of
  //      [24] by batch command processing 1.5Test8.3
  //      [25] by SlowReplay processing 1.5Test10.2
  BOOL         bRepeat[26];       // TRUE, if playback repeat enabled
  UINT         nDelay[26];        // repeat delay time in ms
  UINT         nVolume[26];       // play volume in %
  UINT         nSpeed[26];        // replay speed in %
  UINT       nMode[26];       // replay mode

  void WriteProperties(Object_ostream& obs);
  BOOL bReadProperties(Object_istream& obs);

  void SetupDefault(void);

};

#endif  //_FNKEYS_H

