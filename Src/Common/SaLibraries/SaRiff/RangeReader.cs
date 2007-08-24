using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace Sil.Sa.Riff
{
    public class RangeReader
    {
        public RangeReader(string fileName)
        {
            binaryReader = new BinaryReader(new BufferedStream(new FileStream(fileName, FileMode.Open, FileAccess.Read)));
            startPosition = Position;
            endPosition = binaryReader.BaseStream.Length;
        }

        public RangeReader(RangeReader reader)
        {
            this.binaryReader = reader.binaryReader;
            startPosition = reader.StartPosition;
            endPosition = reader.EndPosition;
        }

        public void RestrictRange(long startPosition, long endPosition)
        {
            if (startPosition > StartPosition)
                this.startPosition = startPosition;
            if (endPosition < EndPosition)
                this.endPosition = endPosition;
        }

        public long Position
        {
            get { return binaryReader.BaseStream.Position; }
            set { binaryReader.BaseStream.Position = value; }
        }

        public byte ReadByte()
        {
            if (!CanRead(1))
            {
                Position = EndPosition;
                return 0;
            }
            return binaryReader.ReadByte();
        }

        public sbyte ReadSByte()
        {
            if (!CanRead(1))
            {
                Position = EndPosition;
                return 0;
            }
            return binaryReader.ReadSByte();
        }

        public byte[] ReadBytes(int readLength)
        {
            if (!CanRead(readLength))
            {
                Position = EndPosition;
                return null;
            }
            return binaryReader.ReadBytes(readLength);
        }

        public int ReadInt16()
        {
            if (!CanRead(2))
            {
                Position = EndPosition;
                return 0;
            }
            return binaryReader.ReadInt16();
        }

        public int ReadUInt16()
        {
            if (!CanRead(2))
            {
                Position = EndPosition;
                return 0;
            }
            return binaryReader.ReadUInt16();
        }

        public int ReadInt32()
        {
            if (!CanRead(4))
            {
                Position = EndPosition;
                return 0;
            }
            return binaryReader.ReadInt32();
        }

        public uint ReadUInt32()
        {
            if (!CanRead(4))
            {
                Position = EndPosition;
                return 0;
            }
            return binaryReader.ReadUInt32();
        }

        public float ReadSingle()
        {
            if (!CanRead(4))
            {
                Position = EndPosition;
                return 0;
            }
            return binaryReader.ReadSingle();
        }

        public string ParseString()
        {
            byte b;
            StringBuilder s = new StringBuilder((int)Length);

            while ((b = ReadByte()) != 0)
            {
                s.Append((char)b);
            }

            return s.ToString();
        }

        public string ParseFixedString(int fixedLength)
        {
            byte[] b = ReadBytes(fixedLength);
            StringBuilder s = new StringBuilder((int)Length);
            int i = 0;

            while ((b[i]) != 0)
            {
                s.Append((char)b[i++]);
            }

            return s.ToString();
        }

        public string[] ParseStrings(int expectedStringCount)
        {
            string[] stringArray = new string[expectedStringCount];

            for (int j = 0; j < expectedStringCount; j++)
            {
                stringArray[j] = ParseString();
            }
            return stringArray;
        }

        public long StartPosition
        {
            get { return startPosition; }
        }

        public long Length
        {
            get { return EndPosition - StartPosition; }
        }

        public long EndPosition
        {
            get { return endPosition; }
        }

        private bool CanRead(int length)
        {
            return length <= EndPosition - Position;
        }

        private long startPosition;
        private long endPosition;

        readonly BinaryReader binaryReader;


    }
}
