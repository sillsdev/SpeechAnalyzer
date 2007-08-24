using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using SilEncConverters;

namespace Sil.Sa.Converter
{
    public class FindConverters
    {
        public static FontConverterMap[] GetFontConverterMappings(string fontIn)
        {
            List<FontConverterMap> result = new List<FontConverterMap>();
            
            EncConverters silEncConverters = new SilEncConverters.EncConverters();

            foreach(mappingRegistry.fontMappingRow row in silEncConverters.RepositoryFile.fontMapping)
            {
                if (row.name == fontIn)
                {
                    string mapping = row.fontMappingsRow.mappingRow.name;

                    EncConverter c = (EncConverter)silEncConverters[mapping];
                    if (c != null)
                    {
                        result.Add(new FontConverterMap(fontIn, mapping, row.assocFont, c));
                    }
                }
                else if (row.assocFont == fontIn)
                {
                    string mapping = row.fontMappingsRow.mappingRow.name;

                    EncConverter c = (EncConverter)silEncConverters[mapping];

                    if (c != null && !EncConverters.IsUnidirectional(c.ConversionType))
                    {
                        c.DirectionForward = false;
                        result.Add(new FontConverterMap(fontIn, mapping, row.name, c));
                    }
                }
            }

            return result.ToArray();
        }
    }
}
