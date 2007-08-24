using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Reflection;
using NUnit.Framework;
using SIL.SpeechTools.TestUtils;

namespace SIL.SpeechTools
{
	/// ----------------------------------------------------------------------------------------
	/// <summary>
	/// 
	/// </summary>
	/// ----------------------------------------------------------------------------------------
	[TestFixture]
	public class WaveReaderTests : TestBase
	{
		private string m_wavFile;
		private WaveReader m_wavRdr;
		
		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// 
		/// </summary>
		/// ------------------------------------------------------------------------------------
		[TestFixtureSetUp]
		public void FixtureSetup()
		{
			// First thing is to extract the test wave file from the resources and write
			// it to the disk. The folder where it is written is the same folder where
			// this assembly's DLL is located.
			Assembly assembly = Assembly.GetExecutingAssembly();

			using (System.IO.Stream stream = assembly.GetManifestResourceStream(
				"SIL.SpeechTools.WavRdrTestData.wav"))
			{
				// CodeBase prepends "file:/", which must be removed.
				string wavDir = Path.GetDirectoryName(assembly.CodeBase).Substring(6);
				m_wavFile = Path.Combine(wavDir, "WavRdrTestData.wav");

				// Read all file into a big byte buffer.
				byte[] buff = new byte[stream.Length];
				stream.Read(buff, 0, (int)stream.Length);
				stream.Close();

				// Do this just in case the file didn't get deleted on a previous fixture run.
				try
				{
					File.Delete(m_wavFile);
				}
				catch {}

				// Now write the byte buffer to the disk.
				using (FileStream fs = new FileStream(m_wavFile, FileMode.CreateNew))
				{
					BinaryWriter bw = new BinaryWriter(fs);
					bw.Write(buff);
					bw.Close();
					fs.Close();
				}
			}

			m_wavRdr = new WaveReader(m_wavFile);
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// 
		/// </summary>
		/// ------------------------------------------------------------------------------------
		[TestFixtureTearDown]
		public void FixtureTearDown()
		{
			try
			{
				m_wavRdr.Close();
				File.Delete(m_wavFile);
			}
			catch {}
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Tests the GetChunkOffset method
		/// </summary>
		/// ------------------------------------------------------------------------------------
		[Test]
		public void GetChunkOffsetTest()
		{
			long offset = (long)GetResult(m_wavRdr, "GetChunkOffset", WaveReader.kidSAChunk);
			Assert.AreEqual(46476, offset, WaveReader.kidSAChunk + " chunk incorrect");
			
			offset = (long)GetResult(m_wavRdr, "GetChunkOffset", WaveReader.kidEticChunk);
			Assert.AreEqual(46794, offset, WaveReader.kidEticChunk + " chunk incorrect");

			offset = (long)GetResult(m_wavRdr, "GetChunkOffset", WaveReader.kidEmicChunk);
			Assert.AreEqual(46904, offset, WaveReader.kidEmicChunk + " chunk incorrect");

			offset = (long)GetResult(m_wavRdr, "GetChunkOffset", WaveReader.kidToneChunk);
			Assert.AreEqual(47078, offset, WaveReader.kidToneChunk + " chunk incorrect");

			offset = (long)GetResult(m_wavRdr, "GetChunkOffset", WaveReader.kidOrthoChunk);
			Assert.AreEqual(47170, offset, WaveReader.kidOrthoChunk + " chunk incorrect");

			offset = (long)GetResult(m_wavRdr, "GetChunkOffset", WaveReader.kidMarkChunk);
			Assert.AreEqual(47344, offset, WaveReader.kidMarkChunk + " chunk incorrect");

			offset = (long)GetResult(m_wavRdr, "GetChunkOffset", WaveReader.kidSpkrChunk);
			Assert.AreEqual(47572, offset, WaveReader.kidSpkrChunk + " chunk incorrect");
		
			offset = (long)GetResult(m_wavRdr, "GetChunkOffset", WaveReader.kidLangChunk);
			Assert.AreEqual(47582, offset, WaveReader.kidLangChunk + " chunk incorrect");
		
			offset = (long)GetResult(m_wavRdr, "GetChunkOffset", WaveReader.kidRefChunk);
			Assert.AreEqual(47664, offset, WaveReader.kidRefChunk + " chunk incorrect");

			// Make sure we get back -1 when the chunk id is invalid.
			offset = (long)GetResult(m_wavRdr, "GetChunkOffset", "bad ");
			Assert.AreEqual(-1, offset);
		}
	}
}
