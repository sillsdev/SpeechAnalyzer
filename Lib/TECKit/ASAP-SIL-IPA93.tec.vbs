' See the "EC Installation Readme.txt" file for details of these methods, and parameters
' This map is for the ASAP IPA93 <> Unicode Encoding Conversion as defined in ASAP SIL-IPA93.tec
mapName = "ASAP SIL IPA93<>UNICODE"
fileName = "MapsTables\ASAP-SIL-IPA93.tec"
encodingNameLhs = "ASAP-SIL-IPA93"
encodingNameRhs = "Unicode IPA"
fontNameASAPDoulos = "ASAP SILDoulos"
fontNameASAPManuscript = "ASAP SILManuscript"
fontNameASAPSophia = "ASAP SILSophia"
fontNameASAPCharis = "ASAP SILCharis"
fontNameDoulosSIL = "Doulos SIL"
' keyboardName = "IPA93 1.0 (FF)"

' vbscript doesn't allow import of tlb info, so redefine them here for documentation purposes
Legacy_to_from_Unicode = 1
UnicodeEncodingConversion = &H1

' get the repository object and use it to add this converter
Dim aECs
Set aECs = CreateObject("SilEncConverters.EncConverters")

' WScript.Arguments(0) is the TARGETDIR on installation
aECs.Add mapName, WScript.Arguments(0) + fileName, Legacy_to_from_Unicode, _
    encodingNameLhs, encodingNameRhs, UnicodeEncodingConversion

' for the corresponding fonts, we also want to add an association between that font
' and the encoding name
aECs.AddFont fontNameASAPDoulos, 42, encodingNameLhs
aECs.AddFont fontNameASAPManuscript, 42, encodingNameLhs
aECs.AddFont fontNameASAPSophia, 42, encodingNameLhs
aECs.AddFont fontNameASAPCharis, 42, encodingNameLhs
aECs.AddUnicodeFontEncoding fontNameDoulosSIL, encodingNameRhs

' default conversions of SILDoulos to Doulos SIL for this mapping
aECs.AddFontMapping mapName, fontNameASAPDoulos, fontNameDoulosSIL
aECs.AddFontMapping mapName, fontNameASAPManuscript, fontNameDoulosSIL
aECs.AddFontMapping mapName, fontNameASAPSophia, fontNameDoulosSIL
aECs.AddFontMapping mapName, fontNameASAPCharis, fontNameDoulosSIL

' indicate which Keyman keyboard goes with these font names
PropertyType_FontName = 2
' aECs.Attributes(fontNameASAPDoulos, PropertyType_FontName).Add "Keyman Keyboard", keyboardName
' aECs.Attributes(fontNameASAPManuscript, PropertyType_FontName).Add "Keyman Keyboard", keyboardName
' aECs.Attributes(fontNameASAPSophia, PropertyType_FontName).Add "Keyman Keyboard", keyboardName
' aECs.Attributes(fontNameASAPCharis, PropertyType_FontName).Add "Keyman Keyboard", keyboardName

Set aECs = Nothing
