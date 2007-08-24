using System;
using System.Collections.Generic;
using System.Text;
using Sil.Sa.Converter;

namespace Sil.Sa.Riff
{
    public class SaRiffConvert
    {
        public SaRiffConvert(SaRiff80 baseline)
        {
            riff = baseline.Clone();
        }

        public void Convert()
        {
            Dictionary<AnnotationIndex, IConvertString> annDictionary = new Dictionary<AnnotationIndex,IConvertString>();

            Fonts fonts = Riff.Preferences.Fonts;

            for (AnnotationIndex i = 0; i < fonts.GetLength(); ++i)
            {
                FontConverterMap[] mappings = FindConverters.GetFontConverterMappings(fonts[i]);

                if (mappings != null && mappings.GetLength(0) > 0)
                {
                    annDictionary.Add(i, mappings[0]);
                }
            }
            Convert(annDictionary);
        }

        public void Convert(Dictionary<AnnotationIndex, IConvertString> annDictionary)
        {
            ConvertAnnotations(annDictionary);
            ConvertMusic();
        }

        void ConvertAnnotations(Dictionary<AnnotationIndex, IConvertString> annDictionary)
        {
            Annotation a = riff.Annotation;

            if (a != null)
            {
                for (AnnotationIndex i = 0; i < a.GetLength(); ++i)
                {
                    Segment[] segments = a[i];

                    if (annDictionary.ContainsKey(i))
                    {
                        IConvertString c = annDictionary[i];

                        riff.Preferences.Fonts[i] = c.FontFaceOut;
                        if (segments != null)
                        {
                            foreach (Segment s in segments)
                            {
                                s.Content = c.Convert(s.Content);
                            }
                        }
                    }
                }
            }
        }

        void ConvertMusic()
        {
        }

        private SaRiff80 riff;
        public SaRiff80 Riff
        {
            get { return riff; }
        }


    }
}
