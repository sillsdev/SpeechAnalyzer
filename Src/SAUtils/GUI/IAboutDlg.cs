using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Reflection;
using System.Resources;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using Microsoft.Win32;
using System.Diagnostics;
using System.IO;

namespace SIL.SpeechAnalyzer.GUI {
/// ----------------------------------------------------------------------------------------
/// <summary>
/// Public interface (exported with COM wrapper) for the FW Help About dialog box
/// </summary>
/// ----------------------------------------------------------------------------------------
[ComVisible(true)]
[GuidAttribute("8B69A2CD-32F3-435F-BC7B-35885E2E83D5")]
[InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
public interface IAboutDlg {
    /// -----------------------------------------------------------------------------------
    /// <summary>
    /// Shows the form as a modal dialog with the currently active form as its owner
    /// </summary>
    /// ------------------------------------------------------------------------------------
    int ShowDialog();

    /// ------------------------------------------------------------------------------------
    /// <summary>
    /// The build description which appears in the Build label on the about dialog box
    /// </summary>
    /// <remarks>
    /// .Net clients should not set this. It will be ignored.
    /// </remarks>
    /// ------------------------------------------------------------------------------------
    string Build
    {
        set;
    }

    /// ------------------------------------------------------------------------------------
    /// <summary>
    /// Sets a flag indicating whether or not the text "Beta" should follow the version no.
    /// </summary>
    /// ------------------------------------------------------------------------------------
    bool IsBetaVersion
    {
        set;
    }

    /// ------------------------------------------------------------------------------------
    /// <summary>
    /// The product name which appears in the Name label on the about dialog box
    /// </summary>
    /// <remarks>
    /// .Net clients should not set this. It will be ignored.
    /// </remarks>
    /// ------------------------------------------------------------------------------------
    string ProdName
    {
        set;
    }

    /// ------------------------------------------------------------------------------------
    /// <summary>
    /// The product version which appears in the App Version label on the about dialog box
    /// </summary>
    /// <remarks>
    /// .Net clients should not set this. It will be ignored.
    /// </remarks>
    /// ------------------------------------------------------------------------------------
    string ProdVersion
    {
        set;
    }

    /// ------------------------------------------------------------------------------------
    /// <summary>
    /// The copyright info which appears in the Copyright label on the splash screen
    /// </summary>
    /// <remarks>
    /// .Net clients should not set this. It will be ignored.
    /// </remarks>
    /// ------------------------------------------------------------------------------------
    string Copyright
    {
        set;
    }

    /// ------------------------------------------------------------------------------------
    /// <summary>
    /// The drive letter whose free space will be reported in the About box.
    /// </summary>
    /// <remarks>
    /// .Net clients should not set this. It will be ignored.
    /// </remarks>
    /// ------------------------------------------------------------------------------------
    string DriveLetter
    {
        set;
    }
}
}