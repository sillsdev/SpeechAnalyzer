namespace Sil.Sa.Riff.Test
{
    using System;
    using NUnit.Framework;
    using Sil.Sa.Riff;

    [TestFixture]
    public class RiffTest
    {
        public Sil.Sa.Riff.RiffReader testRiff;

        [SetUp]
        public void Setup()
        {
            try
            {
                testRiff = new RiffReader("testWavFile.wav");
            }
            catch
            {
            }
        }

        [Test]
        public void TestWavFileFound()
        {
            RiffReader testRiff = new RiffReader("testWavFile.wav");
            Assert.IsNotNull(testRiff);
        }

        [Test]
        [ExpectedException(typeof(System.IO.FileNotFoundException))]
        public void TestWavFileNotFound()
        {
            RiffReader bogus = new RiffReader("bogus.wav");
            Assert.IsNull(bogus);
        }

        [Test]
        public void TestFindChunk()
        {
            Assert.IsNotNull(testRiff.FindChunk('s', 'a', ' ', ' '));
            Assert.IsNotNull(testRiff.FindChunk('f', 'm', 't', ' '));
            Assert.IsNotNull(testRiff.FindChunk('f', 'o', 'n', 't'));
        }

        [Test]
        public void TestMissingChunk()
        {
            RangeReader reader;

            Assert.IsNull(reader = testRiff.FindChunk('f', 'o', 'o', ' '));
        }

        [Test]
        public void TestParseString()
        {
            RangeReader reader = testRiff.FindChunk('f', 'o', 'n', 't');
            string phonetic = reader.ParseString();
            string phonemic = reader.ParseString();

            Assert.AreEqual("ASAP SILDoulos", phonetic);
            Assert.AreEqual("ASAP SILManuscript", phonemic);
        }

        [Test]
        public void TestParseStrings()
        {
            RangeReader reader = testRiff.FindChunk('f', 'o', 'n', 't');
            string[] font = reader.ParseStrings(6);

            Assert.AreEqual(6, font.Length);
            Assert.AreEqual("ASAP SILDoulos", font[0]);
            Assert.AreEqual("ASAP SILManuscript", font[1]);

            // There are no more strings in the chunk
            // We should not fail anyways
            font = reader.ParseStrings(4);
            Assert.AreEqual(4, font.Length);
            // the strings should be either empty or null
            if(font[0] != null)
                Assert.AreEqual("", font[0]);
        }

    }

}
