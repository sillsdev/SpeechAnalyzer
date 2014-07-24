#ifndef ELAN_H
#define ELAN_H

#include <Windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <XMLUtils.h>

using std::logic_error;
using std::vector;
using std::string;
using std::wstring;
using std::stringstream;

using namespace XML;

namespace Elan {

#define ALIGNABLE_ANNOTATION L"ALIGNABLE_ANNOTATION"
#define ANNOTATIONE         L"ANNOTATION"
#define ANNOTATION_DOCUMENT L"ANNOTATION_DOCUMENT"
#define ANNOTATION_ID       L"ANNOTATION_ID"
#define ANNOTATION_REF      L"ANNOTATION_REF"
#define ANNOTATION_VALUE    L"ANNOTATION_VALUE"
#define ANNOTATOR           L"ANNOTATOR"
#define ASSOCIATED_WITH     L"ASSOCIATED_WITH"
#define AUTHOR              L"AUTHOR"
#define CONSTRAINT          L"CONSTRAINT"
#define CONTROLLED_VOCABULARY L"CONTROLLED_VOCABULARY"
#define DATE                L"DATE"
#define DEFAULT_LOCALE      L"DEFAULT_LOCALE"
#define EXTERNAL_REF        L"EXTERNAL_REF"
#define EXTRACTED_FROM      L"EXTRACTED_FROM"
#define FORMAT              L"FORMAT"
#define HEADER              L"HEADER"
#define LEXICON_REF         L"LEXICON_REF"
#define LINGUISTIC_TYPE     L"LINGUISTIC_TYPE"
#define LINGUISTIC_TYPE_REF L"LINGUISTIC_TYPE_REF"
#define LINKED_FILE_DESCRIPTOR L"LINKED_FILE_DESCRIPTOR"
#define LINK_URL            L"LINK_URL"
#define LOCALE              L"LOCALE"
#define MEDIA_DESCRIPTOR    L"MEDIA_DESCRIPTOR"
#define MEDIA_FILE          L"MEDIA_FILE"
#define MEDIA_URL           L"MEDIA_URL"
#define MIME_TYPE           L"MIME_TYPE"
#define PARENT_REF          L"PARENT_REF"
#define PARTICIPANT         L"PARTICIPANT"
#define PREVIOUS_ANNOTATION L"PREVIOUS_ANNOTATION"
#define PROPERTY            L"PROPERTY"
#define REF_ANNOTATION      L"REF_ANNOTATION"
#define RELATIVE_LINK_URL   L"RELATIVE_LINK_URL"
#define RELATIVE_MEDIA_URL  L"RELATIVE_MEDIA_URL"
#define SVG_REF             L"SVG_REF"
#define TIER                L"TIER"
#define TIER_ID             L"TIER_ID"
#define TIME_ORDER          L"TIME_ORDER"
#define TIME_ORIGIN         L"TIME_ORIGIN"
#define TIME_SLOT           L"TIME_SLOT"
#define TIME_SLOT_ID        L"TIME_SLOT_ID"
#define TIME_SLOT_REF1      L"TIME_SLOT_REF1"
#define TIME_SLOT_REF2      L"TIME_SLOT_REF2"
#define TIME_UNITS          L"TIME_UNITS"
#define TIME_VALUE          L"TIME_VALUE"
#define VERSION             L"VERSION"

// the Elan model

/*
    <xsd:element name="MEDIA_DESCRIPTOR" minOccurs="0" maxOccurs="unbounded">
        <xsd:complexType>
            <xsd:attribute name="MEDIA_URL" type="xsd:anyURI" use="required"/>
            <xsd:attribute name="RELATIVE_MEDIA_URL" type="xsd:anyURI" use="optional"/>
            <xsd:attribute name="MIME_TYPE" type="xsd:string" use="required"/>
            <xsd:attribute name="TIME_ORIGIN" type="xsd:long" use="optional"/>
            <xsd:attribute name="EXTRACTED_FROM" type="xsd:anyURI" use="optional"/>
        </xsd:complexType>
    </xsd:element>
*/
class CMediaDescriptor {
public:
    CMediaDescriptor() {
    };
    CMediaDescriptor(XML::Element & right) {
        expect(MEDIA_DESCRIPTOR,right);
        restrict_elements(MEDIA_DESCRIPTOR,right);
        vector<XML::Attribute>::iterator it = right.attributes.begin();
        while (it!=right.attributes.end()) {
            Attribute & attribute = *it;
            if (attribute.localname.compare(MEDIA_URL)==0) {
                mediaURL = attribute.value;
            } else if (attribute.localname.compare(RELATIVE_MEDIA_URL)==0) {
                relativeMediaURL = attribute.value;
            } else if (attribute.localname.compare(MIME_TYPE)==0) {
                mimeType = attribute.value;
            } else if (attribute.localname.compare(TIME_ORIGIN)==0) {
                timeOrigin = _wtoi(attribute.value.c_str());
            } else if (attribute.localname.compare(EXTRACTED_FROM)==0) {
                extractedFrom = attribute.value;
            } else {
                unexpected(MEDIA_DESCRIPTOR,attribute);
            }
            it++;
        }
    };

    // attributes
    wstring mediaURL;
    wstring relativeMediaURL;
    wstring mimeType;
    long timeOrigin;
    wstring extractedFrom;
};

class CRelativeLinkURL {
public:
};

class CAssociatedWith {
public:
};

/*
<xsd:element name="LINKED_FILE_DESCRIPTOR" minOccurs="0" maxOccurs="unbounded">
    <xsd:complexType>
        <xsd:attribute name="LINK_URL" type="xsd:anyURI" use="required"/>
        <xsd:attribute name="RELATIVE_LINK_URL" type="xsd:anyURI" use="optional"/>
        <xsd:attribute name="MIME_TYPE" type="xsd:string" use="required"/>
        <xsd:attribute name="TIME_ORIGIN" type="xsd:long" use="optional"/>
        <xsd:attribute name="ASSOCIATED_WITH" type="xsd:anyURI" use="optional"/>
    </xsd:complexType>
</xsd:element>
*/
class CLinkedFileDescriptor {
public:
    CLinkedFileDescriptor() {
    };
    CLinkedFileDescriptor(Element & right) {
        expect(LINKED_FILE_DESCRIPTOR,right);
        restrict_elements(LINKED_FILE_DESCRIPTOR,right);
        vector<Attribute>::iterator it = right.attributes.begin();
        while (it!=right.attributes.end()) {
            Attribute & attribute = *it;
            if (attribute.localname.compare(LINK_URL)==0) {
                linkURL = attribute.value;
            } else if (attribute.localname.compare(RELATIVE_LINK_URL)==0) {
                relativeLinkURL = attribute.value;
            } else if (attribute.localname.compare(MIME_TYPE)==0) {
                mimeType = attribute.value;
            } else if (attribute.localname.compare(TIME_ORIGIN)==0) {
                timeOrigin = _wtoi(attribute.value.c_str());
            } else if (attribute.localname.compare(ASSOCIATED_WITH)==0) {
                associatedWith = attribute.value;
            } else {
                unexpected(LINKED_FILE_DESCRIPTOR,attribute);
            }
            it++;
        }
    };

    // attributes
    wstring linkURL;
    wstring relativeLinkURL;
    wstring mimeType;
    long timeOrigin;
    wstring associatedWith;
};

class CProperty {
public:
    CProperty() {
    };
    CProperty(Element & /*right*/) {
    }
};

/*
<xsd:attribute name="TIME_UNITS" use="optional" default="milliseconds">
    <xsd:simpleType>
        <xsd:restriction base="xsd:string">
            <xsd:enumeration value="NTSC-frames"/>
            <xsd:enumeration value="PAL-frames"/>
            <xsd:enumeration value="milliseconds"/>
        </xsd:restriction>
    </xsd:simpleType>
</xsd:attribute>
*/

enum ETimeUnits { NTSC_frames, PAL_frames, milliseconds};

/*
<xsd:complexType name="headType">
    <xsd:sequence>
        <xsd:element name="MEDIA_DESCRIPTOR" minOccurs="0" maxOccurs="unbounded">
            <xsd:complexType>
                <xsd:attribute name="MEDIA_URL" type="xsd:anyURI" use="required"/>
                <xsd:attribute name="RELATIVE_MEDIA_URL" type="xsd:anyURI" use="optional"/>
                <xsd:attribute name="MIME_TYPE" type="xsd:string" use="required"/>
                <xsd:attribute name="TIME_ORIGIN" type="xsd:long" use="optional"/>
                <xsd:attribute name="EXTRACTED_FROM" type="xsd:anyURI" use="optional"/>
            </xsd:complexType>
        </xsd:element>
        <xsd:element name="LINKED_FILE_DESCRIPTOR" minOccurs="0" maxOccurs="unbounded">
            <xsd:complexType>
                <xsd:attribute name="LINK_URL" type="xsd:anyURI" use="required"/>
                <xsd:attribute name="RELATIVE_LINK_URL" type="xsd:anyURI" use="optional"/>
                <xsd:attribute name="MIME_TYPE" type="xsd:string" use="required"/>
                <xsd:attribute name="TIME_ORIGIN" type="xsd:long" use="optional"/>
                <xsd:attribute name="ASSOCIATED_WITH" type="xsd:anyURI" use="optional"/>
            </xsd:complexType>
        </xsd:element>
        <xsd:element name="PROPERTY" type="propType" minOccurs="0" maxOccurs="unbounded"/>
    </xsd:sequence>
    <xsd:attribute name="MEDIA_FILE" use="optional" type="xsd:string">
        <xsd:annotation>
            <xsd:documentation xml:lang="en">
                This attribute is deprecated. Use MEDIA_DESCRIPTOR elements instead.
            </xsd:documentation>
            <xsd:appinfo>Ignore</xsd:appinfo>
        </xsd:annotation>
    </xsd:attribute>
    <xsd:attribute name="TIME_UNITS" use="optional" default="milliseconds">
        <xsd:simpleType>
            <xsd:restriction base="xsd:string">
                <xsd:enumeration value="NTSC-frames"/>
                <xsd:enumeration value="PAL-frames"/>
                <xsd:enumeration value="milliseconds"/>
            </xsd:restriction>
        </xsd:simpleType>
    </xsd:attribute>
</xsd:complexType>
*/
class CHeader {
public:
    CHeader() {
    };
    CHeader(Element & right) {
        expect(HEADER,right);

        vector<Element *>::iterator it = right.elements.begin();
        while (it!=right.elements.end()) {
            Element & element = **it;
            if (element.localname.compare(MEDIA_DESCRIPTOR)==0) {
                mediaDescriptors.push_back(CMediaDescriptor(element));
            } else if (element.localname.compare(LINKED_FILE_DESCRIPTOR)==0) {
                linkedFileDescriptors.push_back(CLinkedFileDescriptor(element));
            } else if (element.localname.compare(PROPERTY)==0) {
                properties.push_back(CProperty(element));
            } else {
                unexpected(HEADER,element);
            }
            it++;
        }

        vector<Attribute>::iterator it2 = right.attributes.begin();
        while (it2!=right.attributes.end()) {
            Attribute & attribute = *it2;
            if (attribute.localname.compare(MEDIA_FILE)==0) {
                mediaFile = attribute.value;
            } else if (attribute.localname.compare(TIME_UNITS)==0) {
                if (attribute.value.compare(L"NTSC-frames")==0) {
                    timeUnits = NTSC_frames;
                } else if (attribute.value.compare(L"PAL-frames")==0) {
                    timeUnits = PAL_frames;
                } else if (attribute.value.compare(L"milliseconds")==0) {
                    timeUnits = milliseconds;
                } else {
                    throw logic_error("unexpectd TIME_VALUE");
                }
            } else {
                unexpected(HEADER,attribute);
            }
            it2++;
        }
    };

    // elements
    vector<CMediaDescriptor> mediaDescriptors;
    vector<CLinkedFileDescriptor> linkedFileDescriptors;
    vector<CProperty> properties;
    // attributes;
    wstring mediaFile;
    ETimeUnits timeUnits;
};

/*
    <xsd:element name="TIME_SLOT" minOccurs="0" maxOccurs="unbounded">
        <xsd:complexType>
            <xsd:attribute name="TIME_SLOT_ID" type="xsd:ID" use="required"/>
            <xsd:attribute name="TIME_VALUE" type="xsd:unsignedInt" use="optional"/>
        </xsd:complexType>
    </xsd:element>
*/
class CTimeSlot {
public:
    CTimeSlot() {
    }
    CTimeSlot(Element & right) {
        expect(TIME_SLOT,right);
        restrict_elements(TIME_SLOT,right);

        vector<Attribute>::iterator it = right.attributes.begin();
        while (it!=right.attributes.end()) {
            Attribute & attribute = *it;
            if (attribute.localname.compare(TIME_SLOT_ID)==0) {
                timeSlotID = attribute.value;
            } else if (attribute.localname.compare(TIME_VALUE)==0) {
                timeValue = _wtoi(attribute.value.c_str());
            } else {
                unexpected(TIME_SLOT,attribute);
            }
            it++;
        }
    };
    // attributes
    wstring timeSlotID;
    unsigned int timeValue;
};

/*
<xsd:complexType name="timeType">
    <xsd:sequence>
        <xsd:element name="TIME_SLOT" minOccurs="0" maxOccurs="unbounded">
            <xsd:complexType>
                <xsd:attribute name="TIME_SLOT_ID" type="xsd:ID" use="required"/>
                <xsd:attribute name="TIME_VALUE" type="xsd:unsignedInt" use="optional"/>
            </xsd:complexType>
        </xsd:element>
    </xsd:sequence>
</xsd:complexType>
*/
class CTimeOrder {
public:
    CTimeOrder() {
    };
    CTimeOrder(Element & right) {
        expect(TIME_ORDER,right);
        restrict_attributes(TIME_ORDER,right);

        vector<Element *>::iterator it = right.elements.begin();
        while (it!=right.elements.end()) {
            Element & element = **it;
            if (element.localname.compare(TIME_SLOT)==0) {
                timeSlots.push_back(CTimeSlot(element));
            } else {
                unexpected(TIME_ORDER,element);
            }
            it++;
        }
    };

    // elements
    vector<CTimeSlot> timeSlots;
};

/*
<xsd:complexType name="alignableType">
    <xsd:sequence>
        <xsd:element name="ANNOTATION_VALUE" type="xsd:string"/>
    </xsd:sequence>
    <xsd:attributeGroup ref="annotationAttribute"/>
    <xsd:attribute name="TIME_SLOT_REF1" type="xsd:IDREF" use="required"/>
    <xsd:attribute name="TIME_SLOT_REF2" type="xsd:IDREF" use="required"/>
    <xsd:attribute name="SVG_REF" type="xsd:string" use="optional"/>
</xsd:complexType>
*/
class CAlignableAnnotation {
public:
    CAlignableAnnotation() {
    }
    CAlignableAnnotation(Element & right) {
        expect(ALIGNABLE_ANNOTATION,right);

        vector<Element *>::iterator it = right.elements.begin();
        while (it!=right.elements.end()) {
            Element & element = **it;
            if (element.localname.compare(ANNOTATION_VALUE)==0) {
                annotationValue = element.value;
            } else {
                unexpected(ALIGNABLE_ANNOTATION,element);
            }
            it++;
        }

        vector<Attribute>::iterator it2 = right.attributes.begin();
        while (it2!=right.attributes.end()) {
            Attribute & attribute = *it2;
            if (attribute.localname.compare(TIME_SLOT_REF1)==0) {
                timeSlotRef1 = attribute.value;
            } else if (attribute.localname.compare(TIME_SLOT_REF2)==0) {
                timeSlotRef2 = attribute.value;
            } else if (attribute.localname.compare(SVG_REF)==0) {
                svgRef = attribute.value;
            } else if (attribute.localname.compare(ANNOTATION_ID)==0) {
                annotationID = attribute.value;
            } else {
                unexpected(ALIGNABLE_ANNOTATION,attribute);
            }
            it2++;
        }
    }

    // elements
    wstring annotationValue;
    // attributes
    wstring annotationID;
    wstring timeSlotRef1;
    wstring timeSlotRef2;
    wstring svgRef;
};

/*
<xsd:complexType name="refAnnoType">
    <xsd:sequence>
        <xsd:element name="ANNOTATION_VALUE" type="xsd:string"/>
    </xsd:sequence>
    <xsd:attributeGroup ref="annotationAttribute"/>
    <xsd:attribute name="ANNOTATION_REF" type="xsd:IDREF" use="required">
        <xsd:annotation>
            <xsd:documentation>
                This is in fact a reference to the parent annotation.
            </xsd:documentation>
        </xsd:annotation>
    </xsd:attribute>
    <xsd:attribute name="PREVIOUS_ANNOTATION" type="xsd:IDREF" use="optional"/>
</xsd:complexType>
*/
class CRefAnnotation {
public:
    CRefAnnotation() {
    }
    CRefAnnotation(Element & right) {
        expect(REF_ANNOTATION,right);

        vector<Element *>::iterator it = right.elements.begin();
        while (it!=right.elements.end()) {
            Element & element = **it;
            if (element.localname.compare(ANNOTATION_VALUE)==0) {
                annotationValue = element.value;
            } else {
                unexpected(REF_ANNOTATION,element);
            }
            it++;
        }

        vector<Attribute>::iterator it2 = right.attributes.begin();
        while (it2!=right.attributes.end()) {
            Attribute & attribute = *it2;
            if (attribute.localname.compare(ANNOTATION_REF)==0) {
                annotationRef = attribute.value;
            } else if (attribute.localname.compare(PREVIOUS_ANNOTATION)==0) {
                previousAnnotation = attribute.value;
            } else if (attribute.localname.compare(ANNOTATION_ID)==0) {
                annotationID = attribute.value;
            } else {
                unexpected(REF_ANNOTATION,attribute);
            }
            it2++;
        }
    }

    // elements
    wstring annotationValue;
    // attributes
    wstring annotationRef;
    wstring previousAnnotation;
    wstring annotationID;
};


/*
<xsd:complexType name="annotationType">
    <xsd:choice>
        <xsd:element name="ALIGNABLE_ANNOTATION" type="alignableType"/>
        <xsd:element name="REF_ANNOTATION" type="refAnnoType"/>
    </xsd:choice>
</xsd:complexType>
*/
class CAnnotation {
public:
    CAnnotation() {
    }
    CAnnotation(Element & right) {
        expect(ANNOTATIONE,right);
        restrict_attributes(ANNOTATIONE,right);

        vector<Element *>::iterator it = right.elements.begin();
        while (it!=right.elements.end()) {
            Element & element = **it;
            if (element.localname.compare(ALIGNABLE_ANNOTATION)==0) {
                alignableAnnotation = CAlignableAnnotation(element);
            } else if (element.localname.compare(REF_ANNOTATION)==0) {
                refAnnotation = CRefAnnotation(element);
            } else {
                unexpected(ANNOTATIONE,element);
            }
            it++;
        }
    }

    CAlignableAnnotation alignableAnnotation;
    CRefAnnotation refAnnotation;
};

/*
    <xsd:complexType name="tierType">
        <xsd:sequence>
            <xsd:element name="ANNOTATION" type="annotationType" minOccurs="0" maxOccurs="unbounded"/>
        </xsd:sequence>
        <xsd:attribute name="TIER_ID" type="xsd:string" use="required"/>
        <xsd:attribute name="PARTICIPANT" type="xsd:string" use="optional"/>
        <xsd:attribute name="ANNOTATOR" type="xsd:string" use="optional"/>
        <xsd:attribute name="LINGUISTIC_TYPE_REF" type="xsd:string" use="required"/>
        <xsd:attribute name="DEFAULT_LOCALE" type="xsd:IDREF" use="optional"/>
        <xsd:attribute name="PARENT_REF" type="xsd:string" use="optional"/>
    </xsd:complexType>
*/
class CTier {
public:
    CTier() {
    }
    CTier(Element & right) {
        expect(TIER,right);

        vector<Element *>::iterator it = right.elements.begin();
        while (it!=right.elements.end()) {
            Element & element = **it;
            if (element.localname.compare(ANNOTATIONE)==0) {
                annotations.push_back(CAnnotation(element));
            } else {
                unexpected(TIER,element);
            }
            it++;
        }

        vector<Attribute>::iterator it2 = right.attributes.begin();
        while (it2!=right.attributes.end()) {
            Attribute & attribute = *it2;
            if (attribute.localname.compare(TIER_ID)==0) {
                tierID = attribute.value;
            } else if (attribute.localname.compare(PARTICIPANT)==0) {
                participant = attribute.value;
            } else if (attribute.localname.compare(ANNOTATOR)==0) {
                annotator = attribute.value;
            } else if (attribute.localname.compare(LINGUISTIC_TYPE_REF)==0) {
                linguisticTypeRef = attribute.value;
            } else if (attribute.localname.compare(DEFAULT_LOCALE)==0) {
                defaultLocale = attribute.value;
            } else if (attribute.localname.compare(PARENT_REF)==0) {
                parentRef = attribute.value;
            } else {
                unexpected(TIER,attribute);
            }
            it2++;
        }
    }

    // elements
    vector<CAnnotation> annotations;
    // attributes
    wstring tierID;
    wstring participant;
    wstring annotator;
    wstring linguisticTypeRef;
    wstring defaultLocale;
    wstring parentRef;

};

class CLinguisticType {
public:
    CLinguisticType() {
    }
    CLinguisticType(Element & /*right*/) {
    }
};

class CDate {
public:
    void load(Attribute & /*element*/) {
    };
};

class CLocale {
public:
    CLocale() {
    }
    CLocale(Element & /*right*/) {
    }
};

class CLexiconRef {
public:
    CLexiconRef() {
    };
    CLexiconRef(Element & /*right*/) {
    };
};

class CExternalRef {
public:
    CExternalRef() {
    }
    CExternalRef(Element & /*right*/) {
    }
};

class CConstraint {
public:
    CConstraint() {
    }
    CConstraint(Element & /*right*/) {
    }
};

class CCVEntry {
public:
};

class CControlledVocabulary {
public:
    CControlledVocabulary() {
    }
    CControlledVocabulary(Element & /*right*/) {
    }
};

/*
    <xsd:element name="ANNOTATION_DOCUMENT">
        <xsd:complexType>
            <xsd:sequence>
                <xsd:element name="HEADER" type="headType"/>
                <xsd:element name="TIME_ORDER" type="timeType"/>
                <xsd:element name="TIER" type="tierType" minOccurs="0" maxOccurs="unbounded"/>
                <xsd:element name="LINGUISTIC_TYPE" type="lingType" minOccurs="0" maxOccurs="unbounded"/>
                <xsd:element name="LOCALE" type="localeType" minOccurs="0" maxOccurs="unbounded"/>
                <xsd:element name="CONSTRAINT" type="constraintType" minOccurs="0" maxOccurs="unbounded"/>
                <xsd:element name="CONTROLLED_VOCABULARY" type="convocType" minOccurs="0" maxOccurs="unbounded"/>
                <xsd:element name="LEXICON_REF" type="lexRefType" minOccurs="0" maxOccurs="unbounded"/>
                <xsd:element name="EXTERNAL_REF" type="extRefType" minOccurs="0" maxOccurs="unbounded"/>
            </xsd:sequence>
            <xsd:attribute name="DATE" type="xsd:dateTime" use="required"/>
            <xsd:attribute name="AUTHOR" type="xsd:string" use="required"/>
            <xsd:attribute name="VERSION" type="xsd:string" use="required"/>
            <xsd:attribute name="FORMAT" type="xsd:string" use="optional" default="2.7"/>
        </xsd:complexType>
*/

class CAnnotationDocument {
public:
    void load(Element & element);
    unsigned int getTime(LPCTSTR timeSlotID);
    unsigned int getTime(CRefAnnotation & annot, bool start);

    // elements
    CHeader header;
    CTimeOrder timeOrder;
    vector<CTier> tiers;
    vector<CLinguisticType> linguisticTypes;
    vector<CLocale> locales;
    vector<CConstraint> constraints;
    vector<CControlledVocabulary> controlledVocabularies;
    vector<CLexiconRef> lexiconRefs;
    vector<CExternalRef> externalRefs;

    // attributes
    CDate date;
    wstring author;
    wstring version;
    wstring format;
};

CAnnotationDocument LoadDocument(LPCTSTR filename);

}
#endif
