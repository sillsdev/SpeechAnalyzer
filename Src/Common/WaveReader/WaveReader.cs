using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace SIL.SpeechTools
{
	public class WaveReader
	{
		public const string kidRiff = "RIFF";
		public const string kidWave = "WAVE";
		public const string kidFmtChunk = "fmt ";
		public const string kidDataChunk = "data";
		public const string kidSAChunk = "sa  ";
		public const string kidUttChunk = "utt ";
		public const string kidEticChunk = "etic";
		public const string kidEmicChunk = "emic";
		public const string kidToneChunk = "tone";
		public const string kidOrthoChunk = "orth";
		public const string kidMarkChunk = "mark";
		public const string kidSpkrChunk = "spkr";
		public const string kidLangChunk = "lang";
		public const string kidRefChunk = "ref ";

		private FileStream m_stream = null;
		private BinaryReader m_reader = null;

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// 
		/// </summary>
		/// ------------------------------------------------------------------------------------
		public WaveReader(string waveFile)
		{
			m_stream = File.Open(waveFile, FileMode.Open);
			m_reader = new BinaryReader(m_stream, Encoding.ASCII);
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Closes the file opened by the wave file reader.
		/// </summary>
		/// ------------------------------------------------------------------------------------
		public void Close()
		{
			if (m_stream != null)
				m_stream.Close();

			m_stream = null;
			m_reader = null;
		}

		/// ------------------------------------------------------------------------------------
		/// <summary> 
		/// This function will return the byte offset, in a WAV file, of the beginning of the
		/// desired chunk. The chunk is specified by passing it's four character ID via sChunk.
		/// Each chunk begins with a 4 character ID followed by 4 bytes that indicate how long
		/// the chunk is. That will allow us to find out where the next chunk begins. If,
		/// however, a chunks length is odd, there will be a 1 byte pad at the end of that
		/// chunk so the next chunk always starts on an even byte boundary.  
		/// </summary>
		/// ------------------------------------------------------------------------------------
		private long GetChunkOffset(string chunkId)
		{
			// Start at beginning of 'fmt' chunk.
			m_stream.Position = 12;

			while (m_stream.Position < m_stream.Length)
			{
				// Read the chunk id.
				string id = new string(Encoding.ASCII.GetChars(m_reader.ReadBytes(4)));

				// If we've found the chunk id we're looking for, then return the
				// current position less the 4 bytes we just read to get the id.
				if (id == chunkId)
					return m_stream.Position - 4;

				// Get chunk length.
				uint chunkLen = m_reader.ReadUInt32();

				//Account for odd length chunks.
				if ((chunkLen & 1) != 0)
					chunkLen++;

				// Make sure we don't move the file pointer beyond EOF.
				if ((m_stream.Position + chunkLen) >= m_stream.Length - 4)
					break;

				// Move the file pointer over the chunk to the beginning of the next chunk.
				m_stream.Position += chunkLen;
			}

			return -1;
		}
	}

}
