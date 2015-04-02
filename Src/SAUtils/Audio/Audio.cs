using System;
using System.IO;
using Yeti.MMedia;
using Yeti.WMFSdk;
using System.Collections.Generic;
using System.Text;
using System.Reflection;
using System.Runtime.InteropServices;

namespace SIL.SpeechTools.Audio {
[ProgId("SpeechToolsInterfaces.STAudio")]
[ClassInterface(ClassInterfaceType.None)]
[GuidAttribute("C42FB2D2-898E-456b-B76D-55792D093249")]
[ComVisible(true)]
public class STAudio : SIL.SpeechTools.Audio.ISTAudio {
    public STAudio() {
    }

    public bool PlayPartial(string fileName, uint offset, uint duration) {
        return false;
    }

    public bool Play(string fileName) {
        //return true;
        return false;
    }

    public bool PlayEx(string fileName, uint offset, uint duration, uint speed) {
        return false;
    }

    public bool ConvertToWAV(string inPath, string outPath, uint sampleRate, uint sampleSize, uint channels) {
        WaveLib.WaveFormat fmt = new WaveLib.WaveFormat((int)sampleRate, (int)sampleSize, (int)channels);
        using(WmaStream str = new WmaStream(inPath, fmt)) {
            byte[] buffer = new byte[str.SampleSize * 2];

            AudioWriter writer = new WaveWriter(new FileStream(outPath, FileMode.Create), str.Format);
            try {
                int read;
                while ((read = str.Read(buffer, 0, buffer.Length)) > 0) {
                    writer.Write(buffer, 0, read);
                }
            }
            finally {
                writer.Close();
            }
        } //str.Close() is automatically called by Dispose.

        return true;
    }

}
}
