using System;
using System.Runtime.InteropServices;

namespace SIL.SpeechTools.Audio
{
    [ComVisible(true)]
    [Guid("D76B3878-C097-4600-A844-9B249BF2B9CE")]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    public interface ISTAudio
    {
        #region methods
        bool PlayPartial(string fileName, uint offset, uint duration);
		bool Play(string fileName);
		bool PlayEx(string fileName, uint offset, uint duration, uint speed);
		bool ConvertToWAV(string inPath, string outPath, uint sampleRate, uint sampleSize, uint channels);
        #endregion

        #region Properties
        // none yet.
        #endregion
    }
}
