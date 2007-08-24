using System;
using System.Collections.Generic;
using System.Text;
using NUnit.Framework;
using Sil.Sa.Converter;

namespace Sil.Sa.Riff.Test
{
    [TestFixture]
    public class SaRiffConvertTests
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
        public void TestPhoneticConvert()
        {
            SaRiffConvert unicodeRiff = new SaRiffConvert(testRiff.Riff80);
            unicodeRiff.Convert();

            Assert.AreEqual(33, unicodeRiff.Riff.Annotation.Phonetic.Length);
            Assert.AreEqual("I", unicodeRiff.Riff.Annotation.Phonetic[3].Content);
        }

    }
}
