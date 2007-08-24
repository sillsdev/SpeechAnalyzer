using System;
using System.Collections.Generic;
using System.Text;

namespace Sil.Sa.Riff
{
    public partial class SaRiff80 : ICloneable
    {
        /// <summary>
        /// Default SaRiff80 Constructor.  Creates default children.
        /// </summary>
        public SaRiff80()
        {
            Annotation = new Annotation();
            AudioEncodingInfo = new AudioEncodingInfo();
            DatabaseInfo = new DatabaseInfo();
            FileReference = new FileReference();
            LanguageInfo = new LanguageInfo();
            MusicInfo = new MusicInfo();
            Preferences = new Preferences();
            ReferenceInfo = new ReferenceInfo();
            SpeakerInfo = new SpeakerInfo();
        }

        /// <summary> 
        /// Deep Clone of SaRiff80 
        /// </summary>
        public SaRiff80 Clone()
        {
            SaRiff80 clone = MemberwiseClone() as SaRiff80;

            clone.Annotation = Annotation.Clone();
            clone.AudioEncodingInfo = AudioEncodingInfo.Clone();
            clone.DatabaseInfo = DatabaseInfo.Clone();
            clone.FileReference = FileReference.Clone();
            clone.LanguageInfo = LanguageInfo.Clone();
            clone.MusicInfo = MusicInfo.Clone();
            clone.Preferences = Preferences.Clone();
            clone.ReferenceInfo = ReferenceInfo.Clone();
            clone.SpeakerInfo = SpeakerInfo.Clone();

            return clone;
        }

        #region ICloneable Members

        object ICloneable.Clone()
        {
            return Clone();
        }

        #endregion
    }

    public partial class FileReference : ICloneable
    {
        public FileReference Clone()
        {
            return this.MemberwiseClone() as FileReference;
        }

        #region ICloneable Members

        object ICloneable.Clone()
        {
            return Clone();
        }

        #endregion
    }

    /// <remarks/>
    public partial class AudioEncodingInfo : ICloneable
    {
        public AudioEncodingInfo()
        {
            RecordingInfo = new RecordingInfo();
            SampleFlags = new SampleFlags();
            SignalRange = new SignalRange();
        }

        public AudioEncodingInfo Clone()
        {
            AudioEncodingInfo clone = this.MemberwiseClone() as AudioEncodingInfo;

            clone.RecordingInfo = this.RecordingInfo.Clone();
            clone.SampleFlags = this.SampleFlags.Clone();
            clone.SignalRange = this.SignalRange.Clone();

            return clone;
        }

        #region ICloneable Members

        object ICloneable.Clone()
        {
            return Clone();
        }

        #endregion
    }

    /// <remarks/>
    public partial class SignalRange : ICloneable
    {
        public SignalRange Clone()
        {
            return this.MemberwiseClone() as SignalRange;
        }

        #region ICloneable Members

        object ICloneable.Clone()
        {
            return Clone();
        }

        #endregion
    }

    /// <remarks/>
    public partial class SampleFlags : ICloneable
    {
        public SampleFlags Clone()
        {
            return this.MemberwiseClone() as SampleFlags;
        }

        #region ICloneable Members

        object ICloneable.Clone()
        {
            return Clone();
        }

        #endregion
    }

    /// <remarks/>
    public partial class RecordingInfo : ICloneable
    {
        public RecordingInfo Clone()
        {
            return this.MemberwiseClone() as RecordingInfo;
        }

        #region ICloneable Members

        object ICloneable.Clone()
        {
            return Clone();
        }

        #endregion
    }

    /// <remarks/>
    public partial class ReferenceInfo : ICloneable
    {
        public ReferenceInfo Clone()
        {
            return this.MemberwiseClone() as ReferenceInfo;
        }

        #region ICloneable Members

        object ICloneable.Clone()
        {
            return Clone();
        }

        #endregion
    }

    /// <remarks/>
    public partial class LanguageInfo : ICloneable
    {
        public LanguageInfo Clone()
        {
            return this.MemberwiseClone() as LanguageInfo;
        }

        #region ICloneable Members

        object ICloneable.Clone()
        {
            return Clone();
        }

        #endregion
    }

    /// <remarks/>
    public partial class SpeakerInfo : ICloneable
    {
        public SpeakerInfo Clone()
        {
            return this.MemberwiseClone() as SpeakerInfo;
        }

        #region ICloneable Members

        object ICloneable.Clone()
        {
            return Clone();
        }

        #endregion
    }

    public enum AnnotationIndex
    {
        Gloss,
        Orthographic,
        PartOfSpeech,
        Phonemic,
        Phonetic,
        Phrase1,
        Phrase2,
        Phrase3,
        Phrase4,
        Reference,
        Tone,
        AnnotationArrayLength
    }

    /// <remarks/>
    public partial class Annotation : ICloneable
    {
        /// <summary>
        /// Create a deep clone of Annotation object
        /// </summary>
        /// <returns>Deep Clone</returns>
        public Annotation Clone()
        {
            Annotation clone = MemberwiseClone() as Annotation;

            for (AnnotationIndex a = 0; a < this.GetLength(); ++a)
            {
                if (this[a] != null)
                {
                    clone[a] = new Segment[this[a].GetLength(0)];

                    for (int i = 0; i < this[a].GetLength(0); ++i)
                    {
                        clone[a][i] = this[a][i].Clone();
                    }
                }
            }

            return clone;
        }

        public AnnotationIndex GetLength()
        {
            return AnnotationIndex.AnnotationArrayLength;
        }

        public Segment[] this[AnnotationIndex index]
        {
            get
            {
                switch (index)
                {
                    case AnnotationIndex.Gloss:
                        return Gloss;
                    case AnnotationIndex.Orthographic:
                        return Orthographic;
                    case AnnotationIndex.PartOfSpeech:
                        return PartOfSpeech;
                    case AnnotationIndex.Phonemic:
                        return Phonemic;
                    case AnnotationIndex.Phonetic:
                        return Phonetic;
                    case AnnotationIndex.Phrase1:
                        return Phrase1;
                    case AnnotationIndex.Phrase2:
                        return Phrase2;
                    case AnnotationIndex.Phrase3:
                        return Phrase3;
                    case AnnotationIndex.Phrase4:
                        return Phrase4;
                    case AnnotationIndex.Reference:
                        return Reference;
                    case AnnotationIndex.Tone:
                        return Tone;
                    default:
                        return null;
                }
            }
            set 
            { 
                switch (index)
                {
                    case AnnotationIndex.Gloss:
                        Gloss = value;
                        break;
                    case AnnotationIndex.Orthographic:
                        Orthographic = value;
                        break;
                    case AnnotationIndex.PartOfSpeech:
                        PartOfSpeech = value;
                        break;
                    case AnnotationIndex.Phonemic:
                        Phonemic = value;
                        break;
                    case AnnotationIndex.Phonetic:
                        Phonetic = value;
                        break;
                    case AnnotationIndex.Phrase1:
                        Phrase1 = value;
                        break;
                    case AnnotationIndex.Phrase2:
                        Phrase2 = value;
                        break;
                    case AnnotationIndex.Phrase3:
                        Phrase3 = value;
                        break;
                    case AnnotationIndex.Phrase4:
                        Phrase4 = value;
                        break;
                    case AnnotationIndex.Reference:
                        Reference = value;
                        break;
                    case AnnotationIndex.Tone:
                        Tone = value;
                        break;
                    default:
                        throw new ArgumentException("Annotation Index Out of Range");
                }
            }
        }


        #region ICloneable Members

        object ICloneable.Clone()
        {
            return Clone();
        }

        #endregion
    }

    /// <remarks/>
    public partial class Segment : ICloneable
    {
        public Segment Clone()
        {
            return this.MemberwiseClone() as Segment;
        }

        #region ICloneable Members

        object ICloneable.Clone()
        {
            return Clone();
        }

        #endregion
    }

    /// <remarks/>
    public partial class DatabaseInfo : ICloneable
    {
        /// <summary>
        /// Create Deep Cone 
        /// </summary>
        /// <returns>Deep Clone</returns>
        public DatabaseInfo Clone()
        {
            DatabaseInfo clone = MemberwiseClone() as DatabaseInfo;

            if (Registration != null)
            {
                for (int i = 0; i < Registration.GetLength(0); i++)
                {
                    clone.Registration[i] = Registration[i].Clone();
                }
            }

            return clone;
        }

        #region ICloneable Members

        object ICloneable.Clone()
        {
            return Clone();
        }

        #endregion
    }

    /// <remarks/>
    public partial class Registration : ICloneable
    {
        /// <summary>
        /// Create Deep Cone 
        /// </summary>
        /// <returns>Deep Clone</returns>
        public Registration Clone()
        {
            Registration clone = MemberwiseClone() as Registration;

            return clone;
        }

        #region ICloneable Members

        object ICloneable.Clone()
        {
            return Clone();
        }

        #endregion
    }

    /// <remarks/>
    public partial class MusicInfo : ICloneable
    {
        public MusicInfo Clone()
        {
            return this.MemberwiseClone() as MusicInfo;
        }

        #region ICloneable Members

        object ICloneable.Clone()
        {
            return Clone();
        }

        #endregion
    }

    /// <remarks/>
    public partial class Preferences : ICloneable
    {
        /// <summary>
        /// Create Deep Cone 
        /// </summary>
        /// <returns>Deep Clone</returns>
        public Preferences Clone()
        {
            Preferences clone = MemberwiseClone() as Preferences;

            clone.Fonts = Fonts.Clone();
            clone.GrapplCustomPitch = GrapplCustomPitch.Clone();

            return clone;
        }

        #region ICloneable Members

        object ICloneable.Clone()
        {
            return Clone();
        }

        #endregion
    }

    /// <remarks/>
    public partial class Fonts : ICloneable
    {

        public string this[AnnotationIndex index]
        {
            get
            {
                switch (index)
                {
                    case AnnotationIndex.Gloss:
                        return Gloss;
                    case AnnotationIndex.Orthographic:
                        return Orthographic;
                    case AnnotationIndex.PartOfSpeech:
                        return PartOfSpeech;
                    case AnnotationIndex.Phonemic:
                        return Phonemic;
                    case AnnotationIndex.Phonetic:
                        return Phonetic;
                    case AnnotationIndex.Phrase1:
                        return Phrase1;
                    case AnnotationIndex.Phrase2:
                        return Phrase2;
                    case AnnotationIndex.Phrase3:
                        return Phrase3;
                    case AnnotationIndex.Phrase4:
                        return Phrase4;
                    case AnnotationIndex.Reference:
                        return Reference;
                    case AnnotationIndex.Tone:
                        return Tone;
                    default:
                        return null;
                }
            }
            set 
            {
                switch (index)
                {
                    case AnnotationIndex.Gloss:
                        Gloss = value;
                        break;
                    case AnnotationIndex.Orthographic:
                        Orthographic = value;
                        break;
                    case AnnotationIndex.PartOfSpeech:
                        PartOfSpeech = value;
                        break;
                    case AnnotationIndex.Phonemic:
                        Phonemic = value;
                        break;
                    case AnnotationIndex.Phonetic:
                        Phonetic = value;
                        break;
                    case AnnotationIndex.Phrase1:
                        Phrase1 = value;
                        break;
                    case AnnotationIndex.Phrase2:
                        Phrase2 = value;
                        break;
                    case AnnotationIndex.Phrase3:
                        Phrase3 = value;
                        break;
                    case AnnotationIndex.Phrase4:
                        Phrase4 = value;
                        break;
                    case AnnotationIndex.Reference:
                        Reference = value;
                        break;
                    case AnnotationIndex.Tone:
                        Tone = value;
                        break;
                    default:
                        throw new ArgumentException("Annotation Index Out of Reange");
                }
            }
        }

        public AnnotationIndex GetLength()
        {
            return AnnotationIndex.AnnotationArrayLength;
        }

        public Fonts Clone()
        {
            return this.MemberwiseClone() as Fonts;
        }

        #region ICloneable Members

        object ICloneable.Clone()
        {
            return Clone();
        }

        #endregion
    }

    /// <remarks/>
    public partial class GrapplCustomPitch : ICloneable
    {
        public GrapplCustomPitch Clone()
        {
            return this.MemberwiseClone() as GrapplCustomPitch;
        }

        #region ICloneable Members

        object ICloneable.Clone()
        {
            return Clone();
        }

        #endregion
    }
}

