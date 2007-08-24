//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

namespace Sil.Sa.Riff
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.IO;
    using System.Security.Cryptography;
    using System.Text;
    using System.Xml;
    using System.Xml.Serialization;



    /// <summary>
    /// Parser for Sa Riff wave files
    /// </summary>
    public class SaRiffReader : RiffReader
    {

        /// <summary>
        /// Default constructor - initializes all fields to default values
        /// </summary>
        public SaRiffReader(string fileName)
            : base(fileName)
        {
            riff80 = new SaRiff80();

            BuildFileReference(fileName);
            ParseWave();
        }

        public SaRiff80 Riff80
        {
            get { return riff80; }
        }

        private SaRiff80 riff80;

        private void BuildFileReference(string fileName)
        {
            FileInfo info = new FileInfo(fileName);

            if (info.Exists)
            {
                if (Riff80.FileReference == null)
                    Riff80.FileReference = new FileReference();

                FileReference fileRef = Riff80.FileReference;

                fileRef.Name = info.FullName;

                DriveInfo dInfo = new DriveInfo(info.FullName);

                fileRef.DriveVolume = dInfo.VolumeLabel;
                fileRef.DriveType = dInfo.DriveType.ToString("F");

                string hashAlg = "SHA1";
                HashAlgorithm hash = (HashAlgorithm)CryptoConfig.CreateFromName("SHA1");

                byte[] hashBytes = hash.ComputeHash(File.ReadAllBytes(fileRef.Name));
                StringBuilder s = new StringBuilder(2 * hashBytes.Length);
                foreach (byte b in hashBytes)
                    s.AppendFormat("{0:X2}", b);
                fileRef.Hash = s.ToString();
                fileRef.HashAlg = hashAlg;

                fileRef.Length = info.Length;
                fileRef.Modified = info.LastWriteTime;
            }
        }

        private void ParseWave()
        {
            if (Riff80.ReferenceInfo == null)
                Riff80.ReferenceInfo = new ReferenceInfo();

            RangeReader reader;


            reader = FindChunk('f', 'm', 't', ' ');
            if (reader != null)
            {
                ParseFmt(reader);
            }
            else
            {
                riff80 = null;
                return;
            }


            reader = FindChunk('d', 'a', 't', 'a');
            if (reader != null)
            {
                ParseData(reader);
            }
            else
            {
                riff80 = null;
                return;
            }

            reader = FindChunk('s', 'a', ' ', ' ');
            if (reader != null)
            {
                double bytesToTime = Riff80.AudioEncodingInfo.BitsPerSample/8/ (double)Riff80.AudioEncodingInfo.SamplesPerSecond;
                Annotation Annotation;

                if (Riff80.Annotation == null)
                    Riff80.Annotation = new Annotation();

                Annotation = Riff80.Annotation;

                if (Riff80.Preferences == null)
                    Riff80.Preferences = new Preferences();

                if (Riff80.Preferences.Fonts == null)
                    Riff80.Preferences.Fonts = new Fonts();

                if (Riff80.MusicInfo == null)
                    Riff80.MusicInfo = new MusicInfo();

                ParseSa(reader);

                reader = FindChunk('u', 't', 't', ' ');
                if (reader != null)
                    ParseUtt(reader);

                reader = FindChunk('e', 't', 'i', 'c');
                if (reader != null)
                    Annotation.Phonetic = ParseAnnData(reader, bytesToTime);

                reader = FindChunk('e', 'm', 'i', 'c');
                if (reader != null)
                    Annotation.Phonemic = ParseAnnData(reader, bytesToTime);

                reader = FindChunk('t', 'o', 'n', 'e');
                if (reader != null)
                    Annotation.Tone = ParseAnnData(reader, bytesToTime);

                reader = FindChunk('o', 'r', 't', 'h');
                if (reader != null)
                    Annotation.Orthographic = ParseAnnData(reader, bytesToTime);

                reader = FindChunk('m', 'a', 'r', 'k');
                if (reader != null)
                    ParseMarkData(reader, bytesToTime);

                reader = FindChunk('s', 'p', 'k', 'r');
                if (reader != null)
                    ParseSpkr(reader);

                reader = FindChunk('l', 'a', 'n', 'g');
                if (reader != null)
                    ParseLang(reader);

                reader = FindChunk('d', 'b', ' ', ' ');
                if (reader != null)
                    ParseDB(reader);

                reader = FindChunk('f', 'o', 'n', 't');
                if (reader != null)
                    ParseFont(reader);

                reader = FindChunk('r', 'e', 'f', ' ');
                if (reader != null)
                    ParseRef(reader);

                reader = FindChunk('m', 'd', 'a', 't');
                if (reader != null)
                    ParseMDat(reader);

                reader = FindChunk('m', 'p', 'l', '1');
                if (reader != null)
                    Annotation.Phrase1 = ParseAnnData(reader, bytesToTime);

                reader = FindChunk('m', 'p', 'l', '2');
                if (reader != null)
                    Annotation.Phrase2 = ParseAnnData(reader, bytesToTime);

                reader = FindChunk('m', 'p', 'l', '3');
                if (reader != null)
                    Annotation.Phrase3 = ParseAnnData(reader, bytesToTime);

                reader = FindChunk('m', 'p', 'l', '4');
                if (reader != null)
                    Annotation.Phrase4 = ParseAnnData(reader, bytesToTime);

                reader = FindChunk('m', 'f', 'o', 'n');
                if (reader != null)
                    ParseMFont(reader);
            }

        }

        private void ParseFmt(RangeReader reader)
        {
            if (Riff80.AudioEncodingInfo == null)
                Riff80.AudioEncodingInfo = new AudioEncodingInfo();

            int fromatTag = reader.ReadUInt16();
            Riff80.AudioEncodingInfo.Channels = reader.ReadUInt16();
            Riff80.AudioEncodingInfo.SamplesPerSecond = reader.ReadInt32();
            ulong avgBytesPerSecond = reader.ReadUInt32();
            int blockAlign = reader.ReadUInt16();
            Riff80.AudioEncodingInfo.BitsPerSample = reader.ReadInt16();
        }

        private void ParseData(RangeReader reader)
        {
            // throw new Exception("The method or operation is not implemented.");
        }

        private void ParseSa(RangeReader reader)
        {
            if (Riff80.AudioEncodingInfo == null)
                Riff80.AudioEncodingInfo = new AudioEncodingInfo();

            if (Riff80.AudioEncodingInfo.RecordingInfo == null)
                Riff80.AudioEncodingInfo.RecordingInfo = new RecordingInfo();

            if (Riff80.AudioEncodingInfo.SignalRange == null)
                Riff80.AudioEncodingInfo.SignalRange = new SignalRange();

            if (Riff80.AudioEncodingInfo.SampleFlags == null)
                Riff80.AudioEncodingInfo.SampleFlags = new SampleFlags();

            float version = reader.ReadSingle();

            Riff80.ReferenceInfo.Description = reader.ParseFixedString(256);
            int flags = reader.ReadUInt16();
            Riff80.AudioEncodingInfo.SampleFlags.SampleWordSizeConverted = BitTest(flags, 0x2);
            Riff80.AudioEncodingInfo.SampleFlags.SampleRateConverted = BitTest(flags, 0x4);
            Riff80.AudioEncodingInfo.SampleFlags.SampleDataCompressed = BitTest(flags, 0x40);
            Riff80.AudioEncodingInfo.SampleFlags.SampleDataLossy = BitTest(flags, 0x80);
            if (BitTest(flags, 0x20))
                Riff80.AudioEncodingInfo.RecordingInfo.HighpassFilter = HighpassFilter.HP70Hz;
            else
                Riff80.AudioEncodingInfo.RecordingInfo.HighpassFilter = HighpassFilter.None;

            int recordFileFormat = reader.ReadByte();

            switch (recordFileFormat)
            {
                case 0:
                    Riff80.AudioEncodingInfo.RecordingInfo.RecordingFormat = RecordingFormat.UTT;
                    break;
                case 1:
                    Riff80.AudioEncodingInfo.RecordingInfo.RecordingFormat = RecordingFormat.WAV;
                    break;
                case 2:
                    Riff80.AudioEncodingInfo.RecordingInfo.RecordingFormat = RecordingFormat.MAC;
                    break;
                case 3:
                    Riff80.AudioEncodingInfo.RecordingInfo.RecordingFormat = RecordingFormat.TIMIT;
                    break;
                default:
                    Riff80.AudioEncodingInfo.RecordingInfo.RecordingFormat = RecordingFormat.Unknown;
                    break;
            }

            Riff80.AudioEncodingInfo.RecordingInfo.RecordingTime =
                new DateTime(1970, 1, 1, 0, 0, 0).AddSeconds(reader.ReadUInt32());

            Riff80.AudioEncodingInfo.RecordingInfo.Bandwidth = reader.ReadInt32();

            Riff80.AudioEncodingInfo.RecordingInfo.BitsPerSample = reader.ReadByte();

            Riff80.AudioEncodingInfo.SampleCount = reader.ReadUInt32();

            Riff80.AudioEncodingInfo.SignalRange.Max = reader.ReadInt32();
            Riff80.AudioEncodingInfo.SignalRange.Min = reader.ReadInt32();

            Riff80.AudioEncodingInfo.Bandwidth = reader.ReadInt32();
            Riff80.AudioEncodingInfo.EffectiveBits = reader.ReadByte();
        }

        private static bool BitTest(int flags, int bitMask)
        {
            return ((flags & bitMask) == bitMask);
        }

        private void ParseUtt(RangeReader reader)
        {
            if (Riff80.Preferences == null)
                Riff80.Preferences = new Preferences();

            if (Riff80.Preferences.GrapplCustomPitch == null)
                Riff80.Preferences.GrapplCustomPitch = new GrapplCustomPitch();

            GrapplCustomPitch grappl = Riff80.Preferences.GrapplCustomPitch;

            grappl.CalcFreqLow = reader.ReadUInt16();
            grappl.CalcFreqHigh = reader.ReadUInt16();
            grappl.CalcVoicingThd = reader.ReadUInt16();
            grappl.CalcPercntChng = reader.ReadUInt16();
            grappl.CalcGrpSize = reader.ReadUInt16();
            grappl.CalcIntrpGap = reader.ReadUInt16();
        }

        private Segment[] ParseAnnData(RangeReader reader, double bytesToTime)
        {
            int AnnStringLength = reader.ReadInt16();
            byte[] AnnString = reader.ReadBytes(AnnStringLength + 1);
            UInt32[] PhoneLocUnpacked = new UInt32[2 * AnnStringLength];
            List<Segment> ann = new List<Segment>();

            for (int i = 0; i < AnnStringLength; i++)
            {
                PhoneLocUnpacked[i * 2] = reader.ReadUInt32();
                PhoneLocUnpacked[i * 2 + 1] = reader.ReadUInt32();
            }

            StringBuilder content = new StringBuilder(AnnStringLength);
            for (int i = 0; i < AnnStringLength; i++)
            {
                content.Append((char)AnnString[i]);

                if (((i + 1) == AnnStringLength) ||
                    (PhoneLocUnpacked[i * 2 + 0] != PhoneLocUnpacked[i * 2 + 2]) ||
                    (PhoneLocUnpacked[i * 2 + 1] != PhoneLocUnpacked[i * 2 + 3]))
                {
                    // This is the entire phone squence
                    Segment s = new Segment();

                    s.Begin = bytesToTime*PhoneLocUnpacked[i * 2];
                    s.End = bytesToTime*(PhoneLocUnpacked[i * 2] + PhoneLocUnpacked[i * 2 + 1]);
                    s.Content = content.ToString();

                    ann.Add(s);

                    // Start accumulating a new segment
                    content.Length = 0;
                }
            }

            if (ann.Count > 0)
                return ann.ToArray();
            else
                return null;
        }

        private void ParseMarkData(RangeReader reader, double bytesToSeconds)
        {
            List<Segment> mark = new List<Segment>();
            List<Segment> refr = new List<Segment>();
            List<Segment> pos = new List<Segment>();
            StringBuilder content = new StringBuilder((int)reader.Length);

            while ((reader.Position + 8) < (reader.EndPosition))
            {
                int LabelLength = reader.ReadInt16();
                byte[] LabelString = reader.ReadBytes(LabelLength + 1);
                UInt32[] PhoneLocUnpacked = new UInt32[] { reader.ReadUInt32(), reader.ReadUInt32() };
                int i;

                for (i = 0; i < LabelLength; i++)
                {
                    if (LabelString[i] == 0)
                        break;
                    content.Append((char)LabelString[i]);
                }

                addSegment(mark, content.ToString(),
                        bytesToSeconds * PhoneLocUnpacked[0],
                        bytesToSeconds * (PhoneLocUnpacked[0] + PhoneLocUnpacked[1]));

                // Start accumulating a new segment
                content.Length = 0;

                if (((i + 5) < LabelLength) &&
                    (LabelString[i + 1] == 'r') &&
                    (LabelString[i + 2] == 'e') &&
                    (LabelString[i + 3] == 'f') &&
                    (LabelString[i + 4] == ':'))
                {
                    for (i = i + 5; i < LabelLength; i++)
                    {
                        if (LabelString[i] == 0)
                            break;
                        content.Append((char)LabelString[i]);
                    }
                    addSegment(refr, content.ToString(),
                        bytesToSeconds * PhoneLocUnpacked[0],
                        bytesToSeconds * (PhoneLocUnpacked[0] + PhoneLocUnpacked[1]));

                    // Start accumulating a new segment
                    content.Length = 0;
                }
                if (((i + 5) < LabelLength) &&
                    (LabelString[i + 1] == 'p') &&
                    (LabelString[i + 2] == 'o') &&
                    (LabelString[i + 3] == 's') &&
                    (LabelString[i + 4] == ':'))
                {
                    for (i = i + 5; i < LabelLength; i++)
                    {
                        if (LabelString[i] == 0)
                            break;
                        content.Append((char)LabelString[i]);
                    }
                    addSegment(refr, content.ToString(),
                        bytesToSeconds * PhoneLocUnpacked[0],
                        bytesToSeconds * (PhoneLocUnpacked[0] + PhoneLocUnpacked[1]));

                    // Start accumulating a new segment
                    content.Length = 0;
                }
            }

            if (mark.Count > 0)
                Riff80.Annotation.Gloss = mark.ToArray();
            if (refr.Count > 0)
                Riff80.Annotation.Reference = refr.ToArray();
            if (pos.Count > 0)
                Riff80.Annotation.PartOfSpeech = pos.ToArray();
        }

        private void ParseFont(RangeReader reader)
        {
            string[] fontFaces = reader.ParseStrings(6);

            if (fontFaces[4] != null && fontFaces[4] != "")
            {
                Riff80.Preferences.Fonts.Phonetic = fontFaces[0];
                Riff80.Preferences.Fonts.Phonemic = fontFaces[1];
                Riff80.Preferences.Fonts.Tone = fontFaces[2];
                Riff80.Preferences.Fonts.Orthographic = fontFaces[3];
                Riff80.Preferences.Fonts.Gloss = fontFaces[4];
                Riff80.Preferences.Fonts.Reference = fontFaces[5];
            }
            else
            {
                Riff80.Preferences.Fonts.Phonetic = fontFaces[0];
                Riff80.Preferences.Fonts.Phonemic = fontFaces[1];
                Riff80.Preferences.Fonts.Tone = null;
                Riff80.Preferences.Fonts.Orthographic = fontFaces[3];
                Riff80.Preferences.Fonts.Gloss = fontFaces[4];
                Riff80.Preferences.Fonts.Reference = null;
            }
        }

        private void ParseSpkr(RangeReader reader)
        {
            if (Riff80.SpeakerInfo == null)
                Riff80.SpeakerInfo = new SpeakerInfo();

            SpeakerInfo spkr = Riff80.SpeakerInfo;

            spkr.GenderSpecified = true;
            spkr.AgeSpecified = true;
            switch ((char)reader.ReadByte())
            {
                case 'M':
                    spkr.Gender = Gender.Male;
                    spkr.Age = Age.Adult;
                    break;
                case 'F':
                    spkr.Gender = Gender.Female;
                    spkr.Age = Age.Adult;
                    break;
                case 'C':
                    spkr.Gender = Gender.Unknown;
                    spkr.Age = Age.Child;
                    break;
                default:
                    spkr.Gender = Gender.Unknown;
                    spkr.Age = Age.Unknown;
                    break;
            }

            spkr.SpeakerName = reader.ParseString();
        }

        private void ParseLang(RangeReader reader)
        {
            if (Riff80.LanguageInfo == null)
                Riff80.LanguageInfo = new LanguageInfo();

            LanguageInfo lang = Riff80.LanguageInfo;

            StringBuilder s = new StringBuilder(4);
            for (int i = 0; i < 3; ++i)
                s.Append((char)reader.ReadByte());

            lang.EthnologueID = s.ToString();
            lang.Region = reader.ParseString();
            lang.Country = reader.ParseString();
            lang.Family = reader.ParseString();
            lang.Language = reader.ParseString();
            lang.Dialect = reader.ParseString();
        }

        private void ParseDB(RangeReader reader)
        {
            if (Riff80.DatabaseInfo == null)
                Riff80.DatabaseInfo = new DatabaseInfo();

            Riff80.DatabaseInfo.ApplicationTitle = reader.ParseString();
            Riff80.DatabaseInfo.ApplicationTitle = reader.ParseString();

            int dbRegCount = reader.ReadUInt16();

            Riff80.DatabaseInfo.Registration =
                (dbRegCount > 0) ? new Registration[dbRegCount] : null;

            for (int i = 0; i < dbRegCount; ++i)
            {
                Registration reg = new Registration();

                reg.DatabaseName = reader.ParseString();

                int keyCount = reader.ReadUInt16();

                reg.Key = (keyCount > 0) ? new string[keyCount] : null;

                for (int key = 0; key < keyCount; ++key)
                {
                    reg.Key[key] = reader.ReadUInt32().ToString();
                }

                Riff80.DatabaseInfo.Registration[i] = reg;
            }
        }

        private void ParseRef(RangeReader reader)
        {
            string[] stringArray = reader.ParseStrings(3);

            Riff80.ReferenceInfo.NoteBook = stringArray[0];
            Riff80.ReferenceInfo.FreeTranslation = stringArray[1];
            Riff80.ReferenceInfo.Transcriber = stringArray[2];
        }

        private void ParseMDat(RangeReader reader)
        {
            string[] stringArray = reader.ParseStrings(1);

            Riff80.MusicInfo.MusicScore = stringArray[0];
        }

        private void ParseMFont(RangeReader reader)
        {
            string[] fontFaces = reader.ParseStrings(4);

            Riff80.Preferences.Fonts.Phrase1 = fontFaces[0];
            Riff80.Preferences.Fonts.Phrase2 = fontFaces[1];
            Riff80.Preferences.Fonts.Phrase3 = fontFaces[2];
            Riff80.Preferences.Fonts.Phrase4 = fontFaces[3];
        }

        private static void addSegment(List<Segment> list, String content, double begin, double end)
        {
            if (content.Length == 0)
                return;

            Segment s = new Segment();

            s.Begin = begin;
            s.End = end;
            s.Content = content;

            list.Add(s);
        }

    }
}
