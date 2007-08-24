using System;
using System.Collections.Generic;
using System.Text;
using SilEncConverters;

namespace Sil.Sa.Converter
{
    public class FontConverterMap : IConvertString
    {
        public FontConverterMap(string fontIn, string mapping, string fontOut, EncConverter converter)
        {
            this.fontFaceIn = fontIn;
            this.mapping = mapping;
            this.fontFaceOut = fontOut;
            this.converter = converter;
            directionForward = Converter.DirectionForward;
        }
        
        private readonly string fontFaceIn;
        public string FontFaceIn
        {
            get { return fontFaceIn; }
        }

        private readonly string mapping;
        public string Mapping
        {
            get { return mapping; }
        }

        private readonly string fontFaceOut;
        public string FontFaceOut
        {
            get { return fontFaceOut; }
        }

        private readonly EncConverter converter;
        public EncConverter Converter
        {
            get { return converter; }
        }

        private readonly bool directionForward;
        public bool DirectionForward
        {
            get { return directionForward; }
        }
	


        #region IConvertString Members

        public string Convert(string input)
        {
            int outLength;
            return Converter.ConvertEx(input, EncodingForm.Unspecified, input.Length, EncodingForm.Unspecified, out outLength, NormalizeFlags.None, DirectionForward);
        }

        #endregion
    }
}
