using System;
using System.Collections.Generic;
using System.Xml.Linq;
using System.Text;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Reflection;
using System.Threading;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace SIL.SpeechAnalyzer.GUI
{
    public enum VersionType
    {
        Alpha,
        Beta,
        Production
    }

    /// ----------------------------------------------------------------------------------------
    /// <summary>
    /// Public interface (exported with COM wrapper) for the splash screen
    /// </summary>
    /// ----------------------------------------------------------------------------------------
    [ComVisible(true)]
    [GuidAttribute("DAA1B20E-A16D-4358-A894-B48A0B55D836")]
    public interface ISplashScreen
    {
        /// ----------------------------------------------------------------------------------------
        /// <summary>
        /// Shows the splash screen
        /// </summary>
        /// ----------------------------------------------------------------------------------------
        void Show();

        /// ----------------------------------------------------------------------------------------
        /// <summary>
        /// Shows the splash screen
        /// </summary>
        /// ----------------------------------------------------------------------------------------
        void Show(bool showBuildDate, bool isBetaVersion);

        /// ----------------------------------------------------------------------------------------
        /// <summary>
        /// Shows the splash screen without the fading feature.
        /// </summary>
        /// ----------------------------------------------------------------------------------------
        void ShowWithoutFade();

        /// ----------------------------------------------------------------------------------------
        /// <summary>
        /// Activates (brings back to the top) the splash screen (assuming it is already visible
        /// and the application showing it is the active application).
        /// </summary>
        /// ----------------------------------------------------------------------------------------
        void Activate();

        /// ----------------------------------------------------------------------------------------
        /// <summary>
        /// Closes the splash screen
        /// </summary>
        /// ----------------------------------------------------------------------------------------
        void Close();

        /// ----------------------------------------------------------------------------------------
        /// <summary>
        /// Refreshes the display of the splash screen
        /// </summary>
        /// ----------------------------------------------------------------------------------------
        void Refresh();

        /// ------------------------------------------------------------------------------------
        /// <summary>
        /// The product name which appears in the Name label on the splash screen
        /// </summary>
        /// <remarks>
        /// .Net clients should not set this. It will be ignored.
        /// </remarks>
        /// ------------------------------------------------------------------------------------
        string ProdName { set; }

        /// ------------------------------------------------------------------------------------
        /// <summary>
        /// The product version which appears in the App Version label on the splash screen
        /// </summary>
        /// <remarks>
        /// .Net clients should not set this. It will be ignored.
        /// </remarks>
        /// ------------------------------------------------------------------------------------
        string ProdVersion { set; }

        /// ------------------------------------------------------------------------------------
        /// <summary>
        /// The copyright info to display on the splash screen
        /// </summary>
        /// <remarks>
        /// .Net clients should not set this. It will be ignored.
        /// </remarks>
        /// ------------------------------------------------------------------------------------
        string Copyright { set; }

        /// ------------------------------------------------------------------------------------
        /// <summary>
        /// The message to display to indicate startup activity on the splash screen
        /// </summary>
        /// ------------------------------------------------------------------------------------
        string Message { set; }

        /// ------------------------------------------------------------------------------------
        /// <summary>
        /// Gets a value indicating whether or not the ISplashScreen's underlying form is
        /// still available (i.e. non null).
        /// </summary>
        /// ------------------------------------------------------------------------------------
        bool StillAlive { get; }
    }
}
