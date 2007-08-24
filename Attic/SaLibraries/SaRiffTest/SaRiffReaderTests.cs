
namespace Sil.Sa.Riff.Test
{
    using System;
    using System.Text.RegularExpressions;
    using NUnit.Framework;
    using Sil.Sa.Riff;

    [TestFixture]
    public class SaRiffReaderTest
    {
        public Sil.Sa.Riff.SaRiffReader testRiff;

        [SetUp]
        public void Setup()
        {
            try
            {
                testRiff = new SaRiffReader("testWavFile.wav");
            }
            catch
            {
            }
        }

        [Test]
        public void TestWavFileFound()
        {
            RiffReader testRiff = new SaRiffReader("testWavFile.wav");
            Assert.IsNotNull(testRiff);
        }

        [Test]
        [ExpectedException(typeof(System.IO.FileNotFoundException))]
        public void TestWavFileNotFound()
        {
            RiffReader bogus = new SaRiffReader("bogus.wav");
            Assert.IsNull(bogus);
        }

        [Test]
        public void TestFontParse()
        {
            Assert.AreEqual("ASAP SILDoulos", testRiff.Riff80.Preferences.Fonts.Phonetic);
            Assert.AreEqual("ASAP SILManuscript", testRiff.Riff80.Preferences.Fonts.Phonemic);
        }

        [Test]
        public void TestPhoneticParse()
        {
            double bytesToSeconds = 2.0 / 22050;
            
            Assert.AreEqual(33, testRiff.Riff80.Annotation.Phonetic.Length);
            Assert.AreEqual("I", testRiff.Riff80.Annotation.Phonetic[3].Content);
            Assert.AreEqual(bytesToSeconds*13148, testRiff.Riff80.Annotation.Phonetic[3].Begin);
            Assert.AreEqual(bytesToSeconds*(13148 + 1466), testRiff.Riff80.Annotation.Phonetic[3].End);
        }

        [Test]
        public void TestMarkParse()
        {
            double bytesToSeconds = 2.0 / 22050;

            Assert.AreEqual(10, testRiff.Riff80.Annotation.Gloss.Length);
            Assert.AreEqual("#friends", testRiff.Riff80.Annotation.Gloss[5].Content);
            Assert.AreEqual(bytesToSeconds*51658, testRiff.Riff80.Annotation.Gloss[5].Begin);
            Assert.AreEqual(bytesToSeconds*(51658 + 11480), testRiff.Riff80.Annotation.Gloss[5].End);
        }

        [Test]
        public void TestDBParse()
        {
            Assert.AreEqual("SA106A", testRiff.Riff80.DatabaseInfo.ApplicationTitle);
        }

        [Test]
        public void TestLangParse()
        {
            Assert.AreEqual("???", testRiff.Riff80.LanguageInfo.EthnologueID);
            if (testRiff.Riff80.LanguageInfo.Dialect != null)
                Assert.AreEqual("", testRiff.Riff80.LanguageInfo.Dialect);
        }

        [Test]
        public void TestSpkrParse()
        {
            Assert.AreEqual(Age.Adult, testRiff.Riff80.SpeakerInfo.Age);
            Assert.AreEqual(Gender.Male, testRiff.Riff80.SpeakerInfo.Gender);
            if (testRiff.Riff80.SpeakerInfo.SpeakerName != null)
                Assert.AreEqual("", testRiff.Riff80.SpeakerInfo.SpeakerName);
        }

        [Test]
        public void TestUttParse()
        {
            Assert.AreEqual(70, testRiff.Riff80.Preferences.GrapplCustomPitch.CalcFreqLow);
            Assert.AreEqual(260, testRiff.Riff80.Preferences.GrapplCustomPitch.CalcFreqHigh);
            Assert.AreEqual(6912, testRiff.Riff80.Preferences.GrapplCustomPitch.CalcVoicingThd);
            Assert.AreEqual(11, testRiff.Riff80.Preferences.GrapplCustomPitch.CalcPercntChng);
            Assert.AreEqual(6, testRiff.Riff80.Preferences.GrapplCustomPitch.CalcGrpSize);
            Assert.AreEqual(7, testRiff.Riff80.Preferences.GrapplCustomPitch.CalcIntrpGap);
        }

        [Test]
        public void TestFmtParse()
        {
            Assert.AreEqual(1, testRiff.Riff80.AudioEncodingInfo.Channels);
            Assert.AreEqual(22050, testRiff.Riff80.AudioEncodingInfo.SamplesPerSecond);
            Assert.AreEqual(16, testRiff.Riff80.AudioEncodingInfo.BitsPerSample);
        }

        [Test]
        public void TestSaParse()
        {
            Assert.AreEqual("Swiss-French (Indo-Euro) \"Many thanks to all the friends here today.\"",
                testRiff.Riff80.ReferenceInfo.Description);

            Assert.AreEqual(HighpassFilter.HP70Hz, testRiff.Riff80.AudioEncodingInfo.RecordingInfo.HighpassFilter);
            Assert.AreEqual(true, testRiff.Riff80.AudioEncodingInfo.SampleFlags.SampleWordSizeConverted);
            Assert.AreEqual(false, testRiff.Riff80.AudioEncodingInfo.SampleFlags.SampleRateConverted);
            Assert.AreEqual(false, testRiff.Riff80.AudioEncodingInfo.SampleFlags.SampleDataLossy);
            Assert.AreEqual(false, testRiff.Riff80.AudioEncodingInfo.SampleFlags.SampleDataCompressed);
            Assert.AreEqual(RecordingFormat.UTT, testRiff.Riff80.AudioEncodingInfo.RecordingInfo.RecordingFormat);
//            Assert.AreEqual(new DateTime(2002,8,21,11,30,36), testRiff.Doc.AudioEncodingInfo.RecordingInfo.RecordingTime);
            Assert.AreEqual(9680, testRiff.Riff80.AudioEncodingInfo.RecordingInfo.Bandwidth);
            Assert.AreEqual(8, testRiff.Riff80.AudioEncodingInfo.RecordingInfo.BitsPerSample);
            Assert.AreEqual(32512, testRiff.Riff80.AudioEncodingInfo.SignalRange.Max);
            Assert.AreEqual(-24064, testRiff.Riff80.AudioEncodingInfo.SignalRange.Min);
            Assert.AreEqual(57105, testRiff.Riff80.AudioEncodingInfo.SampleCount);
            Assert.AreEqual(9680, testRiff.Riff80.AudioEncodingInfo.Bandwidth);
            Assert.AreEqual(8, testRiff.Riff80.AudioEncodingInfo.EffectiveBits);
        }

        [Test]
        public void TestFileReference()
        {
            Regex nameEx = new Regex(".+testWavFile\\.wav");
            Assert.IsNotNull(testRiff.Riff80.FileReference);
            Assert.IsTrue(nameEx.IsMatch(testRiff.Riff80.FileReference.Name));
            Assert.Less(16, testRiff.Riff80.FileReference.Name.Length);
            Assert.AreEqual(40, testRiff.Riff80.FileReference.Hash.Length);
            Assert.AreEqual("SHA1", testRiff.Riff80.FileReference.HashAlg);
            Assert.AreEqual(116324, testRiff.Riff80.FileReference.Length);
        }
    }
}

namespace Sil.Sa.Riff.Test
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.IO;
    using System.Text;
    using System.Xml;
    using System.Xml.Serialization;
    using Sil.Sa.Riff;

    public class ConsoleTest
    {
        public static void Main(string[] args)
        {
            // test code
            SaRiffReader testFile = new SaRiffReader("testWavFile.wav");


            // Create an XmlTextWriter using a FileStream.
            XmlSerializer serializer =
            new XmlSerializer(typeof(SaRiff80));
            Stream fs = new FileStream("testWavFile.xml", FileMode.Create);
            XmlWriter writer = new XmlTextWriter(fs, Encoding.UTF8);
            // Serialize using the XmlTextWriter.
            serializer.Serialize(writer, testFile.Riff80);
            writer.Close();

            fs = new FileStream("testWavFile.xml.gz", FileMode.Create);
            System.IO.Compression.GZipStream gz = new System.IO.Compression.GZipStream(fs, System.IO.Compression.CompressionMode.Compress);
            writer = new XmlTextWriter(gz, Encoding.UTF8);
            // Serialize using the XmlTextWriter.
            serializer.Serialize(writer, testFile.Riff80);
            writer.Close();

            SaRiffConvert unicodeRiff = new SaRiffConvert(testFile.Riff80);
            unicodeRiff.Convert();
            fs = new FileStream("testWavFileUnicode.xml", FileMode.Create);
            writer = new XmlTextWriter(fs, Encoding.UTF8);
            // Serialize using the XmlTextWriter.
            serializer.Serialize(writer, unicodeRiff.Riff);
            writer.Close();


        }

    }
}
