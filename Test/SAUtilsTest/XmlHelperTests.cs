using System;
using System.IO;
using Microsoft.VisualStudio.TestTools.UnitTesting;


namespace SIL.SpeechAnalyzer.Utils {

/// ----------------------------------------------------------------------------------------
/// <summary>
///
/// </summary>
/// ----------------------------------------------------------------------------------------
[TestClass]
public class XmlnHelperTests {
    #region IsEmptyOrInvalid tests
    ///--------------------------------------------------------------------------------------
    /// <summary>
    ///
    /// </summary>
    ///--------------------------------------------------------------------------------------
    [TestMethod]
    public void IsEmptyOrInvalid_Null_True() {
        Assert.IsTrue(XmlHelper.IsEmptyOrInvalid(null));
    }

    ///--------------------------------------------------------------------------------------
    /// <summary>
    ///
    /// </summary>
    ///--------------------------------------------------------------------------------------
    [TestMethod]
    public void IsEmptyOrInvalid_EmptyFileName_True() {
        Assert.IsTrue(XmlHelper.IsEmptyOrInvalid(string.Empty));
    }

    ///--------------------------------------------------------------------------------------
    /// <summary>
    ///
    /// </summary>
    ///--------------------------------------------------------------------------------------
    [TestMethod]
    public void IsEmptyOrInvalid_NonExistentFile_True() {
        var tmpFile = Path.GetTempFileName();
        File.Delete(tmpFile);
        Assert.IsTrue(XmlHelper.IsEmptyOrInvalid(tmpFile));
    }

    ///--------------------------------------------------------------------------------------
    /// <summary>
    ///
    /// </summary>
    ///--------------------------------------------------------------------------------------
    [TestMethod]
    public void IsEmptyOrInvalid_EmptyFile_True() {
        var tmpFile = Path.GetTempFileName();
        Assert.IsTrue(XmlHelper.IsEmptyOrInvalid(tmpFile));
        File.Delete(tmpFile);
    }

    ///--------------------------------------------------------------------------------------
    /// <summary>
    ///
    /// </summary>
    ///--------------------------------------------------------------------------------------
    [TestMethod]
    public void IsEmptyOrInvalid_XmlDeclarationOnly_True() {
        var tmpFile = Path.GetTempFileName();
        using(var stream = File.CreateText(tmpFile)) {
            stream.WriteLine("<?xml version=\"1.0\" encoding=\"utf-8\" ?>");
            stream.Close();
            Assert.IsTrue(XmlHelper.IsEmptyOrInvalid(tmpFile));
            File.Delete(tmpFile);
        }
    }

    ///--------------------------------------------------------------------------------------
    /// <summary>
    ///
    /// </summary>
    ///--------------------------------------------------------------------------------------
    [TestMethod]
    public void IsEmptyOrInvalid_MissingRootClose_True() {
        var tmpFile = Path.GetTempFileName();
        using(var stream = File.CreateText(tmpFile)) {
            stream.WriteLine("<?xml version=\"1.0\" encoding=\"utf-8\" ?>");
            stream.WriteLine("<root>");
            stream.Close();
            Assert.IsTrue(XmlHelper.IsEmptyOrInvalid(tmpFile));
            File.Delete(tmpFile);
        }
    }

    ///--------------------------------------------------------------------------------------
    /// <summary>
    ///
    /// </summary>
    ///--------------------------------------------------------------------------------------
    [TestMethod]
    public void IsEmptyOrInvalid_MissingRootOpen_True() {
        var tmpFile = Path.GetTempFileName();
        using(var stream = File.CreateText(tmpFile)) {
            stream.WriteLine("<?xml version=\"1.0\" encoding=\"utf-8\" ?>");
            stream.WriteLine("</root>");
            stream.Close();
            Assert.IsTrue(XmlHelper.IsEmptyOrInvalid(tmpFile));
            File.Delete(tmpFile);
        }
    }

    ///--------------------------------------------------------------------------------------
    /// <summary>
    ///
    /// </summary>
    ///--------------------------------------------------------------------------------------
    [TestMethod]
    public void IsEmptyOrInvalid_MinimalValid_False() {
        var tmpFile = Path.GetTempFileName();
        using(var stream = File.CreateText(tmpFile)) {
            stream.WriteLine("<?xml version=\"1.0\" encoding=\"utf-8\" ?>");
            stream.WriteLine("<root/>");
            stream.Close();
            Assert.IsFalse(XmlHelper.IsEmptyOrInvalid(tmpFile));
            File.Delete(tmpFile);
        }
    }

    ///--------------------------------------------------------------------------------------
    /// <summary>
    ///
    /// </summary>
    ///--------------------------------------------------------------------------------------
    [TestMethod]
    public void IsEmptyOrInvalid_Valid_False() {
        var tmpFile = Path.GetTempFileName();
        using(var stream = File.CreateText(tmpFile)) {
            stream.WriteLine("<?xml version=\"1.0\" encoding=\"utf-8\" ?>");
            stream.WriteLine("<root>blah blah blah</root>");
            stream.Close();
            Assert.IsFalse(XmlHelper.IsEmptyOrInvalid(tmpFile));
            File.Delete(tmpFile);
        }
    }

    #endregion
}
}
