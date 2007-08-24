namespace Sil.Sa.Riff
{
    using System;
    using System.Diagnostics;
    using System.IO;
    using System.Text;

    public class RiffReader : Object
    {
        public RiffReader(string fileName)
        {
            fileReader = new RangeReader(fileName);
        }

        public RangeReader FindChunk(char a, char b, char c, char d)
        {
            Chunk riffWave = FindRiffWave();

            if (riffWave == null)
                return null;

            Chunk subChunk = ReadChunk(riffWave.Position);

            while (subChunk != null)
            {
                if (subChunk.IsTag(a, b, c, d))
                    break;
                subChunk = ReadChunk(subChunk.EndPosition);
            }

            return subChunk;
        }

        private RangeReader fileReader;

        private Chunk FindRiffWave()
        {
            fileReader.Position = 0;

            Chunk riffWave = new Chunk(fileReader);

            try
            {
                while (riffWave != null)
                {
                    if (riffWave.IsForm('W', 'A', 'V', 'E'))
                        return riffWave;
                    riffWave = ReadChunk(riffWave.EndPosition);
                }
            }
            catch
            {
            }
            return null;
        }

        private Chunk ReadChunk(long startPosition)
        {
            if (startPosition + 8 > fileReader.EndPosition)
                return null;
            fileReader.Position = startPosition + (startPosition & 1);
            return new Chunk(fileReader);
        }



        private class Chunk : RangeReader
        {
            public Chunk(RangeReader reader)
                : base(reader)
            {
                InitChunk();
            }

            public bool IsRiff
            {
                get { return IsTag('R', 'I', 'F', 'F'); }
            }

            public bool IsTag(char a, char b, char c, char d)
            {
                return IsMatch(tag, a, b, c, d);
            }

            public bool IsForm(char a, char b, char c, char d)
            {
                if (IsRiff)
                    return IsMatch(form, a, b, c, d);
                else
                    return false;
            }

            private bool IsMatch(byte[] tag, char a, char b, char c, char d)
            {
                if ((tag == null) || (tag.Length != 4))
                    return false;
                else if ((tag[0] != a) || (tag[1] != b) || (tag[2] != c) || (tag[3] != d))
                    return false;
                else
                    return true;
            }

            private byte[] tag;
            private byte[] form;

            private void InitChunk()
            {
                tag = ReadTag();
                long chunkLength = ReadLength();

                long lengthPosition = Position;

                if (IsRiff)
                {
                    form = ReadTag();
                }

                RestrictRange(Position, lengthPosition + chunkLength);
            }

            private byte[] ReadTag()
            {
                try
                {
                    byte[] tag = ReadBytes(4);

                    if (tag == null || tag.Length != 4)
                        return null;
                    else
                        return tag;
                }
                catch
                {
                    return null;
                }
            }

            private int ReadLength()
            {
                int length = ReadInt32();

                return length;
            }

            private static void WriteTag(byte[] tag)
            {
                foreach (byte b in tag)
                    Debug.Write((char)b);
                Debug.WriteLine("");
            }
        }

    }
}
