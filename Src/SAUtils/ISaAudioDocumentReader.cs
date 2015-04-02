using System.Runtime.InteropServices;

namespace SIL.SpeechAnalyzer.Utils {
[ComVisible(true)]
[Guid("E5F3F025-9F70-4863-B143-55280670E17A")]
[InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
public interface ISaAudioDocumentReader {
    #region Methods
    bool Initialize(string audioFilePath, bool isTempDB);
    void Close();
    void ResetSegmentEnumerators();
    bool ReadSegment(int annotationType, out uint offset, out uint length, out string annotation);
    bool ReadMarkSegment(out uint offset, out uint length, out string gloss, out string glossNat, out string reference, out bool isBookmark);
    string ReadAsMusicXML();
    string ImportMusicXML(string fileName);
    #endregion

    #region Properties
    bool DocumentExistsInDB { get;}
    string FreeFormTranslation { get; }
    string LanguageName { get; }
    string MD5HashCode { get; }
    string NoteBookReference { get; }
    string Transcriber { get; }
    string Region { get; }
    string Country { get; }
    string Dialect { get; }
    string EthnologueId { get; }
    string Family { get; }
    string SADescription { get; }
    string SpeakerName { get; }
    char SpeakerGender { get; }
    int DataChunkSize { get; }
    int AverageBytesPerSecond { get; }
    int BitsPerSample { get; }
    int BlockAlignment { get; }
    int CalcFreqHigh { get; }
    int CalcFreqLow { get; }
    int CalcGrpSize { get; }
    int CalcIntrpGap { get; }
    int CalcPercntChng { get; }
    int CalcVoicingThd { get; }
    int Channels { get; }
    int FormatTag { get; }
    int NumberOfSamples { get; }
    int RecordBandWidth { get; }
    int RecordFileFormat { get; }
    int RecordSampleSize { get; }
    int RecordTimeStamp { get; }
    int SAFlags { get; }
    int SamplesPerSecond { get; }
    int SignalBandWidth { get; }
    int SignalEffSampSize { get; }
    int SignalMax { get; }
    int SignalMin { get; }
    #endregion
}
}
