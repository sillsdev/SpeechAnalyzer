// --------------------------------------------------------------------------------------------
#region // Copyright © 2002-2004, SIL International. All Rights Reserved.   
// <copyright from='2002' to='2004' company='SIL International'>
//		Copyright © 2002-2004, SIL International. All Rights Reserved.   
//    
//		Distributable under the terms of either the Common Public License or the
//		GNU Lesser General Public License, as specified in the LICENSING.txt file.
// </copyright> 
#endregion
// 
// File: SplashScreen.cs
// Responsibility: TE Team
// Last reviewed: 
// 
// <remarks>
// Splash Screen
// </remarks>
// --------------------------------------------------------------------------------------------
using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Reflection;
using System.Threading;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace SIL.SpeechTools.Utils
{
	#region ISplashScreen interface
	/// ----------------------------------------------------------------------------------------
	/// <summary>
	/// Public interface (exported with COM wrapper) for the splash screen
	/// </summary>
	/// ----------------------------------------------------------------------------------------
	[ComVisible(true)]
	[GuidAttribute("E8431ECF-FA0A-4140-9F09-10628890FFF9")]
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
		string ProdName {set;}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// The product version which appears in the App Version label on the splash screen
		/// </summary>
		/// <remarks>
		/// .Net clients should not set this. It will be ignored.
		/// </remarks>
		/// ------------------------------------------------------------------------------------
		string ProdVersion {set;}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// The copyright info to display on the splash screen
		/// </summary>
		/// <remarks>
		/// .Net clients should not set this. It will be ignored.
		/// </remarks>
		/// ------------------------------------------------------------------------------------
		string Copyright { set;}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// The message to display to indicate startup activity on the splash screen
		/// </summary>
		/// ------------------------------------------------------------------------------------
		string Message { set;}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Gets a value indicating whether or not the ISplashScreen's underlying form is
		/// still available (i.e. non null).
		/// </summary>
		/// ------------------------------------------------------------------------------------
		bool StillAlive { get;}
	}
	#endregion

	#region SplashScreen implementation
	/// ----------------------------------------------------------------------------------------
	/// <summary>
	/// FW Splash Screen
	/// </summary>
	/// ----------------------------------------------------------------------------------------
	[ProgId("SpeechToolsInterfaces.SplashScreen")]
	// Key attribute to hide the "clutter" from System.Windows.Forms.Form
	[ClassInterface(ClassInterfaceType.None)]
	[GuidAttribute("AA4A6C20-9306-41f0-B525-483CDD5385EC")]
	[ComVisible(true)]
	public class SplashScreen : ISplashScreen
	{
		private SilUtils.SplashScreen m_splash;

		#region Constructor
		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Default Constructor for SplashScreen
		/// </summary>
		/// ------------------------------------------------------------------------------------
		public SplashScreen()
		{
			m_splash = new SilUtils.SplashScreen();
		}

		#endregion

		#region Public Methods
		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// 
		/// </summary>
		/// ------------------------------------------------------------------------------------
		void ISplashScreen.Show(bool showBuildDate, bool isBetaVersion)
		{
			m_splash = new SilUtils.SplashScreen(showBuildDate, isBetaVersion);
			((SilUtils.ISplashScreen)m_splash).Show();
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Shows the splash screen
		/// </summary>
		/// ------------------------------------------------------------------------------------
		void ISplashScreen.Show()
		{
			((SilUtils.ISplashScreen)m_splash).Show();
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Shows the splash screen
		/// </summary>
		/// ------------------------------------------------------------------------------------
		void ISplashScreen.ShowWithoutFade()
		{
			((SilUtils.ISplashScreen)m_splash).ShowWithoutFade();
		}

		/// ----------------------------------------------------------------------------------------
		/// <summary>
		/// Activates (brings back to the top) the splash screen (assuming it is already visible
		/// and the application showing it is the active application).
		/// </summary>
		/// ----------------------------------------------------------------------------------------
		void ISplashScreen.Activate()
		{
			((SilUtils.ISplashScreen)m_splash).Activate();
		}

		/// ----------------------------------------------------------------------------------------
		/// <summary>
		/// Closes the splash screen
		/// </summary>
		/// ----------------------------------------------------------------------------------------
		void ISplashScreen.Close()
		{
			((SilUtils.ISplashScreen)m_splash).Close();
		}

		/// ----------------------------------------------------------------------------------------
		/// <summary>
		/// Refreshes the display of the splash screen
		/// </summary>
		/// ----------------------------------------------------------------------------------------
		void ISplashScreen.Refresh()
		{
			((SilUtils.ISplashScreen)m_splash).Refresh();
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Gets a value indicating whether or not the ISplashScreen's underlying form is
		/// still available (i.e. non null).
		/// </summary>
		/// ------------------------------------------------------------------------------------
		bool ISplashScreen.StillAlive
		{
			get { return ((SilUtils.ISplashScreen)m_splash).StillAlive; }
		}

		#endregion

		#region Public Properties needed for all clients
		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// The message to display to indicate startup activity on the splash screen
		/// </summary>
		/// ------------------------------------------------------------------------------------
		string ISplashScreen.Message
		{
			set { ((SilUtils.ISplashScreen)m_splash).Message = value; }
		}
		#endregion

		#region Public properties set automatically in constructor for .Net apps
		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// The product name which appears in the Name label on the splash screen
		/// </summary>
		/// <remarks>
		/// .Net clients should not set this. It will be ignored. They should set the
		/// AssemblyTitle attribute in AssemblyInfo.cs of the executable.
		/// </remarks>
		/// ------------------------------------------------------------------------------------
		string ISplashScreen.ProdName
		{
			set { ((SilUtils.ISplashScreen)m_splash).ProdName = value; }
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// The product version which appears in the App Version label on the splash screen
		/// </summary>
		/// <remarks>
		/// .Net clients should not set this. It will be ignored. They should set the
		/// AssemblyFileVersion attribute in AssemblyInfo.cs of the executable.
		/// </remarks>
		/// ------------------------------------------------------------------------------------
		string ISplashScreen.ProdVersion
		{
			set { ((SilUtils.ISplashScreen)m_splash).ProdVersion = value; }
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// The copyright info which appears in the Copyright label on the splash screen
		/// </summary>
		/// <remarks>
		/// .Net clients should not set this. It will be ignored. They should set the
		/// AssemblyCopyrightAttribute attribute in AssemblyInfo.cs of the executable.
		/// </remarks>
		/// ------------------------------------------------------------------------------------
		string ISplashScreen.Copyright
		{
			set { ((SilUtils.ISplashScreen)m_splash).Copyright = value; }
		}

		#endregion
	}

	#endregion
}
