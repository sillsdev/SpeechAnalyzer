// --------------------------------------------------------------------------------------------
// <copyright from='2002' to='2004' company='SIL International'>
//      Copyright © 2002-2004, SIL International. All Rights Reserved.
//
//      Distributable under the terms of either the Common Public License or the
//      GNU Lesser General Public License, as specified in the LICENSING.txt file.
// </copyright>
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
using System.IO;

namespace SIL.SpeechAnalyzer.GUI {
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
[ProgId("AudioSync.AboutDlg")]
// Key attribute to hide the "clutter" from System.Windows.Forms.Form
[ClassInterface(ClassInterfaceType.None)]
[GuidAttribute("36D9E4A2-2634-494A-B2E3-95AD1640D0D9")]
[ComVisible(true)]

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
public class ASAboutDlg : Form, IAboutDlg {
    private IContainer components;

    private string m_sAvailableMemoryFmt;
    private string m_sAppVersionFmt;
    private string m_buildFmt;
    private string m_sTitleFmt;
    private string m_sAvailableDiskSpaceFmt;
    private bool m_showBuild = false;
    private bool m_isBetaVersion = false;
    private Panel panel1;
    private Label lblBuild;
    private Label lblAppVersion;
    private Label lblAvailableMemoryValue;
    private Label lblAvailableDiskSpaceValue;
    private Label lblCopyright;
    private Label lblName;
    private string m_sDriveLetter;
    private LinkLabel lnkFeedback;
    private string m_tmpProdVersion;

    /// ----------------------------------------------------------------------------------------
    /// <summary>
    /// Constructor
    /// </summary>
    /// ----------------------------------------------------------------------------------------
    public ASAboutDlg() {
        //
        // Required for Windows Form Designer support
        //
        InitializeComponent();

        // Cache the format strings. The control labels will be overwritten when the
        // dialog is shown.
        m_sAvailableMemoryFmt = lblAvailableMemoryValue.Text;
        m_sAppVersionFmt = lblAppVersion.Text;
        m_sTitleFmt = Text;
        m_sAvailableDiskSpaceFmt = lblAvailableDiskSpaceValue.Text;
        m_buildFmt = lblBuild.Text;

        Initialize();
    }

    /// ------------------------------------------------------------------------------------
    public ASAboutDlg(bool showBuild, bool isBetaVersion)
        : this() {
        m_showBuild = showBuild;
        m_isBetaVersion = isBetaVersion;

        InternalSetBuild(null);
        InternalSetVersionNumber(null);
    }

    /// ----------------------------------------------------------------------------------------
    /// <summary>
    /// Clean up any resources being used.
    /// </summary>
    /// ----------------------------------------------------------------------------------------
    protected override void Dispose(bool disposing) {
        // Must not be run more than once.
        if (IsDisposed) {
            return;
        }

        if (disposing) {
            if (components != null) {
                components.Dispose();
            }
        }
        base.Dispose(disposing);
    }

    /// <summary>
    /// Required method for Designer support - do not modify
    /// the contents of this method with the code editor.
    /// </summary>
    private void InitializeComponent() {
            this.components = new System.ComponentModel.Container();
            System.Windows.Forms.ToolTip m_toolTip;
            System.Windows.Forms.Button buttonOk;
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ASAboutDlg));
            System.Windows.Forms.Label lblAvailableMemory;
            System.Windows.Forms.Label lblAvailableDiskSpace;
            System.Windows.Forms.PictureBox fieldWorksIcon;
            this.panel1 = new System.Windows.Forms.Panel();
            this.lnkFeedback = new System.Windows.Forms.LinkLabel();
            this.lblBuild = new System.Windows.Forms.Label();
            this.lblAppVersion = new System.Windows.Forms.Label();
            this.lblAvailableMemoryValue = new System.Windows.Forms.Label();
            this.lblAvailableDiskSpaceValue = new System.Windows.Forms.Label();
            this.lblCopyright = new System.Windows.Forms.Label();
            this.lblName = new System.Windows.Forms.Label();
            m_toolTip = new System.Windows.Forms.ToolTip(this.components);
            buttonOk = new System.Windows.Forms.Button();
            lblAvailableMemory = new System.Windows.Forms.Label();
            lblAvailableDiskSpace = new System.Windows.Forms.Label();
            fieldWorksIcon = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(fieldWorksIcon)).BeginInit();
            this.panel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // m_toolTip
            // 
            m_toolTip.AutomaticDelay = 100;
            m_toolTip.AutoPopDelay = 1000;
            m_toolTip.InitialDelay = 100;
            m_toolTip.ReshowDelay = 100;
            // 
            // buttonOk
            // 
            buttonOk.DialogResult = System.Windows.Forms.DialogResult.OK;
            resources.ApplyResources(buttonOk, "buttonOk");
            buttonOk.Name = "buttonOk";
            m_toolTip.SetToolTip(buttonOk, resources.GetString("buttonOk.ToolTip"));
            buttonOk.UseVisualStyleBackColor = true;
            // 
            // lblAvailableMemory
            // 
            resources.ApplyResources(lblAvailableMemory, "lblAvailableMemory");
            lblAvailableMemory.Name = "lblAvailableMemory";
            // 
            // lblAvailableDiskSpace
            // 
            resources.ApplyResources(lblAvailableDiskSpace, "lblAvailableDiskSpace");
            lblAvailableDiskSpace.Name = "lblAvailableDiskSpace";
            // 
            // fieldWorksIcon
            // 
            resources.ApplyResources(fieldWorksIcon, "fieldWorksIcon");
            fieldWorksIcon.Image = global::SIL.SpeechAnalyzer.Properties.Resources.kimidSilLogo;
            fieldWorksIcon.Name = "fieldWorksIcon";
            fieldWorksIcon.TabStop = false;
            // 
            // panel1
            // 
            this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.panel1.Controls.Add(this.lnkFeedback);
            this.panel1.Controls.Add(this.lblBuild);
            this.panel1.Controls.Add(this.lblAppVersion);
            this.panel1.Controls.Add(this.lblAvailableMemoryValue);
            this.panel1.Controls.Add(this.lblAvailableDiskSpaceValue);
            this.panel1.Controls.Add(lblAvailableMemory);
            this.panel1.Controls.Add(lblAvailableDiskSpace);
            this.panel1.Controls.Add(this.lblCopyright);
            this.panel1.Controls.Add(fieldWorksIcon);
            this.panel1.Controls.Add(this.lblName);
            this.panel1.Controls.Add(buttonOk);
            resources.ApplyResources(this.panel1, "panel1");
            this.panel1.Name = "panel1";
            // 
            // lnkFeedback
            // 
            resources.ApplyResources(this.lnkFeedback, "lnkFeedback");
            this.lnkFeedback.Name = "lnkFeedback";
            this.lnkFeedback.TabStop = true;
            this.lnkFeedback.UseCompatibleTextRendering = true;
            this.lnkFeedback.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.HandleFeedbackLinkClicked);
            // 
            // lblBuild
            // 
            resources.ApplyResources(this.lblBuild, "lblBuild");
            this.lblBuild.Name = "lblBuild";
            // 
            // lblAppVersion
            // 
            resources.ApplyResources(this.lblAppVersion, "lblAppVersion");
            this.lblAppVersion.Name = "lblAppVersion";
            // 
            // lblAvailableMemoryValue
            // 
            this.lblAvailableMemoryValue.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            resources.ApplyResources(this.lblAvailableMemoryValue, "lblAvailableMemoryValue");
            this.lblAvailableMemoryValue.Name = "lblAvailableMemoryValue";
            // 
            // lblAvailableDiskSpaceValue
            // 
            this.lblAvailableDiskSpaceValue.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            resources.ApplyResources(this.lblAvailableDiskSpaceValue, "lblAvailableDiskSpaceValue");
            this.lblAvailableDiskSpaceValue.Name = "lblAvailableDiskSpaceValue";
            // 
            // lblCopyright
            // 
            resources.ApplyResources(this.lblCopyright, "lblCopyright");
            this.lblCopyright.Name = "lblCopyright";
            // 
            // lblName
            // 
            resources.ApplyResources(this.lblName, "lblName");
            this.lblName.Name = "lblName";
            // 
            // ASAboutDlg
            // 
            this.AcceptButton = buttonOk;
            resources.ApplyResources(this, "$this");
            this.BackColor = System.Drawing.Color.White;
            this.ControlBox = false;
            this.Controls.Add(this.panel1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "ASAboutDlg";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            ((System.ComponentModel.ISupportInitialize)(fieldWorksIcon)).EndInit();
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.ResumeLayout(false);

    }

    /// ------------------------------------------------------------------------------------
    /// <summary>
    /// Initialize the controls
    /// </summary>
    /// ------------------------------------------------------------------------------------
    private void Initialize() {
        try {
            SetVersionInformation();

            // Set the title bar text
            Text = string.Format(m_sTitleFmt, Application.ProductName);
            lblBuild.Visible = m_showBuild;

            SetMemoryAndDiskInformation();
            SetCopyrightInformation();
        } catch {
            // ignore errors
        }
    }

    /// ------------------------------------------------------------------------------------
    /// <summary>
    /// Sets the application's version information.
    /// </summary>
    /// ------------------------------------------------------------------------------------
    private void SetVersionInformation() {
        var assembly = (Assembly.GetEntryAssembly() ?? Assembly.GetExecutingAssembly());
        var attributes = assembly.GetCustomAttributes(typeof(AssemblyTitleAttribute), false);
        var productName = Application.ProductName;

        if (string.IsNullOrEmpty(productName) && attributes.Length > 0) {
            productName = ((AssemblyTitleAttribute)attributes[0]).Title;
        }

        lblName.Text = productName;
        InternalSetBuild(null);
        InternalSetVersionNumber(null);
    }

    /// ------------------------------------------------------------------------------------
    /// <summary>
    /// Sets the version number text on the about box.
    /// </summary>
    /// ------------------------------------------------------------------------------------
    private void InternalSetVersionNumber(string version) {
        if (string.IsNullOrEmpty(version)) {
            var ver = new Version(Application.ProductVersion);
            version = ver.ToString(3);
        }
#if DEBUG
        lblAppVersion.Text = string.Format(m_sAppVersionFmt, version, "(Debug version)", (m_isBetaVersion ? "Beta" : string.Empty));
#else
        lblAppVersion.Text = string.Format(m_sAppVersionFmt, version,
                                           string.Empty, (m_isBetaVersion ? "Beta" : string.Empty));
#endif
    }

    /// ------------------------------------------------------------------------------------
    /// <summary>
    /// Sets the build number (or date by default);
    /// </summary>
    /// ------------------------------------------------------------------------------------
    private void InternalSetBuild(string build) {
        lblBuild.Visible = m_showBuild;

        if (!string.IsNullOrEmpty(build)) {
            lblBuild.Text = build;
        } else {
            // The build number is just the number of days since 01/01/2000
            //var ver = new Version(Application.ProductVersion);
            //int bldNum = ver.Build;

            //var bldDate = (bldNum == 0 ? File.GetCreationTime(Application.ExecutablePath) :
            //      new DateTime(2000, 1, 1).Add(new TimeSpan(bldNum, 0, 0, 0)));

            var bldDate = File.GetCreationTime(Application.ExecutablePath);
            lblBuild.Text = string.Format(m_buildFmt, bldDate.ToString("dd-MMM-yyyy"));
        }
    }

    /// ------------------------------------------------------------------------------------
    /// <summary>
    /// Sets the copyright information.
    /// </summary>
    /// ------------------------------------------------------------------------------------
    private void SetCopyrightInformation() {
        // Get copyright information from assembly info. By doing this we don't have
        // to update the about dialog each year.
        var assembly = Assembly.GetEntryAssembly() ?? Assembly.GetExecutingAssembly();
        var attributes = assembly.GetCustomAttributes(typeof(AssemblyCopyrightAttribute), false);

        // Try to get the copyright from the executing assembly.
        // If that fails, use a generic one.
        var copyright = (attributes.Length > 0 ?
                         ((AssemblyCopyrightAttribute)attributes[0]).Copyright :
                         "(C) 2002-" + DateTime.Now.Year + " SIL International");

        lblCopyright.Text = string.Format(lblCopyright.Text, copyright.Replace("(C)", "©"));
    }

    /// ------------------------------------------------------------------------------------
    /// <summary>
    /// Fills in the memory usage and disk space information.
    /// </summary>
    /// ------------------------------------------------------------------------------------
    private void SetMemoryAndDiskInformation() {
        var strRoot = Application.ExecutablePath.Substring(0, 2);

        // Must be called from COM client
        if (!string.IsNullOrEmpty(m_sDriveLetter)) {
            strRoot = m_sDriveLetter;
        }

        if (!strRoot.EndsWith(Path.VolumeSeparatorChar.ToString())) {
            strRoot += Path.VolumeSeparatorChar;
        }

        strRoot = Application.ExecutablePath.Substring(0, 2) + Path.DirectorySeparatorChar;

        // Set the memory information in MB.
        Utils.MemoryStatus ms = new Utils.MemoryStatus();
        Utils.GlobalMemoryStatus(ref ms);
        double available = ms.dwAvailPhys / Math.Pow(1024, 2);
        double total = ms.dwTotalPhys / Math.Pow(1024, 2);
        lblAvailableMemoryValue.Text = string.Format(m_sAvailableMemoryFmt,
                                       available.ToString("###,###,###,###,###.##"),
                                       total.ToString("###,###,###,###,###"));

        // Set the disk space information in KB and GB.
        ulong freeDiskSpace = Utils.GetFreeDiskSpace(strRoot);
        ulong kbFree = freeDiskSpace / 1024;
        double gbFree = freeDiskSpace / Math.Pow(1024, 3);
        lblAvailableDiskSpaceValue.Text = string.Format(m_sAvailableDiskSpaceFmt,
                                          kbFree.ToString("###,###,###,###,###,###,###"),
                                          gbFree.ToString("##,###.#"), strRoot);
    }

    /// ----------------------------------------------------------------------------------------
    /// <summary>
    /// Shows the form as a modal dialog with the currently active form as its owner
    /// </summary>
    /// ----------------------------------------------------------------------------------------
    int IAboutDlg.ShowDialog() {
        return (int)ShowDialog();
    }

    /// ------------------------------------------------------------------------------------
    /// <summary>
    /// The build date which appears in the Build label on the about dialog box
    /// </summary>
    /// <remarks>
    /// .Net clients should not set this. It will be ignored.
    /// </remarks>
    /// ------------------------------------------------------------------------------------
    string IAboutDlg.Build {
        set {
            m_showBuild = !string.IsNullOrEmpty(value);
            InternalSetBuild(value);
        }
    }

    /// ------------------------------------------------------------------------------------
    /// <summary>
    /// Indicates whether or not the term "Beta" should follow the version number.
    /// </summary>
    /// ------------------------------------------------------------------------------------
    bool IAboutDlg.IsBetaVersion {
        set {
            m_isBetaVersion = value;
            InternalSetVersionNumber(m_tmpProdVersion);
        }
    }

    /// ------------------------------------------------------------------------------------
    /// <summary>
    /// The product name which appears in the Name label on the about dialog box
    /// </summary>
    /// <remarks>
    /// .Net clients should not set this. It will be ignored.
    /// </remarks>
    /// ------------------------------------------------------------------------------------
    string IAboutDlg.ProdName {
        set {
            lblName.Text = value;
            Text = string.Format(m_sTitleFmt, value);
        }
    }

    /// ------------------------------------------------------------------------------------
    /// <summary>
    /// The product version which appears in the App Version label on the about dialog box
    /// </summary>
    /// <remarks>
    /// .Net clients should not set this. It will be ignored.
    /// </remarks>
    /// ------------------------------------------------------------------------------------
    string IAboutDlg.ProdVersion {
        set {
            m_tmpProdVersion = value;
            InternalSetVersionNumber(value);
        }
    }

    /// ------------------------------------------------------------------------------------
    /// <summary>
    /// The copyright info which appears in the Copyright label on the about dialog box
    /// </summary>
    /// <remarks>
    /// .Net clients should not set this. It will be ignored.
    /// </remarks>
    /// ------------------------------------------------------------------------------------
    string IAboutDlg.Copyright {
        set {
            value = value.Replace("\n", " ");
            lblCopyright.Text = value.Replace("(C)", "©");
        }
    }
    /// ------------------------------------------------------------------------------------
    /// <summary>
    /// The drive letter whose free space will be reported in the About box.
    /// </summary>
    /// <remarks>
    /// .Net clients should not set this. It will be ignored.
    /// </remarks>
    /// ------------------------------------------------------------------------------------
    string IAboutDlg.DriveLetter {
        set {
            m_sDriveLetter = value;
            SetMemoryAndDiskInformation();
        }
    }

    /// ------------------------------------------------------------------------------------
    private void HandleFeedbackLinkClicked(object sender, LinkLabelLinkClickedEventArgs e) {
        try {
            System.Diagnostics.Process.Start("mailto:audio-sync_comments@sil.org?subject=Bug%20Report");
        } catch (Exception ex) {
            var msg = "There was an error trying to create an e-mail. It's possible a program for sending e-mail has not been installed.\n\n{0}";
            Utils.MsgBox(string.Format(msg, ex.Message));
        }
    }
}
}
