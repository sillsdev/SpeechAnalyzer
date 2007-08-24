using System;
using System.Collections.Generic;
using System.Text;

namespace Sil.Sa.Converter
{
    public interface IConvertString
    {
        String FontFaceOut
        {
            get;
        }

        String Convert(String input);
    }
}
