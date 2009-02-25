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
// File: AboutDlg.cs
// Responsibility: TE Team
// 
// <remarks>
// Help About dialog
// </remarks>
// --------------------------------------------------------------------------------------------
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

namespace SIL.SpeechTools.Utils
{
	#region IAboutDlg interface
	/// ----------------------------------------------------------------------------------------
	/// <summary>
	/// Public interface (exported with COM wrapper) for the FW Help About dialog box
	/// </summary>
	/// ----------------------------------------------------------------------------------------
	[ComVisible(true)]
	[GuidAttribute("8E90485A-1047-4270-A4D2-8BFA9C1D5ED7")]
	public interface IAboutDlg
	{
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

	#endregion

	#region AboutDlg implementation
	/// ----------------------------------------------------------------------------------------
	/// <summary>
	/// FW Help about dialog (previously HelpAboutDlg in AfDialog.cpp)
	/// </summary>
	/// <remarks>
	/// This dialog shows the registration key from HKLM\Software\SIL\FieldWorks\FwUserReg.
	/// If a DropDeadDate is to something different then 1/1/3000 it also displays the date 
	/// after which the program is no longer working. 
	/// </remarks>
	/// ----------------------------------------------------------------------------------------
	[ProgId("SpeechToolsInterfaces.AboutDlg")]
	// Key attribute to hide the "clutter" from System.Windows.Forms.Form
	[ClassInterface(ClassInterfaceType.None)]
	[GuidAttribute("7ACF99EB-7FB6-482a-B1C2-11E2E14C02A0")]
	[ComVisible(true)]
	public class AboutDlg : IAboutDlg
	{
		#region Data members

		private SilUtils.AboutDlg m_aboutDlg;

		#endregion

		#region Constructor
		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Default Constructor for AboutDlg
		/// </summary>
		/// ------------------------------------------------------------------------------------
		public AboutDlg()
		{
			m_aboutDlg = new SilUtils.AboutDlg();
		}

		#endregion

		#region IAboutDlg interface implementation
		/// ----------------------------------------------------------------------------------------
		/// <summary>
		/// Shows the form as a modal dialog with the currently active form as its owner
		/// </summary>
		/// ----------------------------------------------------------------------------------------
		int IAboutDlg.ShowDialog()
		{
			return ((SilUtils.IAboutDlg)m_aboutDlg).ShowDialog();
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// The build description which appears in the Build label on the about dialog box
		/// </summary>
		/// <remarks>
		/// .Net clients should not set this. It will be ignored.
		/// </remarks>
		/// ------------------------------------------------------------------------------------
		string IAboutDlg.Build
		{
			set { ((SilUtils.IAboutDlg)m_aboutDlg).Build = value; }
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Indicates whether or not the term "Beta" should follow the version number.
		/// </summary>
		/// ------------------------------------------------------------------------------------
		bool IAboutDlg.IsBetaVersion
		{
			set { ((SilUtils.IAboutDlg)m_aboutDlg).IsBetaVersion = value; }
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// The product name which appears in the Name label on the about dialog box
		/// </summary>
		/// <remarks>
		/// .Net clients should not set this. It will be ignored.
		/// </remarks>
		/// ------------------------------------------------------------------------------------
		string IAboutDlg.ProdName
		{
			set { ((SilUtils.IAboutDlg)m_aboutDlg).ProdName = value; }
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// The product version which appears in the App Version label on the about dialog box
		/// </summary>
		/// <remarks>
		/// .Net clients should not set this. It will be ignored.
		/// </remarks>
		/// ------------------------------------------------------------------------------------
		string IAboutDlg.ProdVersion
		{
			set { ((SilUtils.IAboutDlg)m_aboutDlg).ProdVersion = value; }
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// The copyright info which appears in the Copyright label on the about dialog box
		/// </summary>
		/// <remarks>
		/// .Net clients should not set this. It will be ignored.
		/// </remarks>
		/// ------------------------------------------------------------------------------------
		string IAboutDlg.Copyright
		{
			set { ((SilUtils.IAboutDlg)m_aboutDlg).Copyright = value; }
		}
		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// The drive letter whose free space will be reported in the About box.
		/// </summary>
		/// <remarks>
		/// .Net clients should not set this. It will be ignored.
		/// </remarks>
		/// ------------------------------------------------------------------------------------
		string IAboutDlg.DriveLetter
		{
			set { ((SilUtils.IAboutDlg)m_aboutDlg).DriveLetter = value; }
		}

		#endregion
	}

	#endregion
}
