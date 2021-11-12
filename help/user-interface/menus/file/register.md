---
title: Register Speech Analyzer to open Wave Sound files
---

You can register Speech Analyzer as the program to open Wave Sound (WAV) files when you double-click them in Windows Explorer.

The procedure depends on the version of Microsoft Windows.

### **If you are using Windows Vista**
1. Click **Start**, and then click **Default Programs**.

    The **Default Programs** window opens.

1. Click **Associate a file type or protocol with a program**.

    The window becomes **Set Associations**.

1. In the list, select **.wav**.
1. Click **Change program**.

    The **Open With** window appears.

1. Click **Browse**.

    The **Open With** dialog box appears.

1. Navigate to the **C:\\Program Files\\SIL\\Speech Analyzer** folder.
1. Select **SA.exe**.
1. Click **Open**.
1. Click **OK**.
1. Click **Close**.
1. Close the **Default Programs** window.

### **If you are using Windows XP**
1. On the **Tools** menu of Windows Explorer, click **Folder Options**.

    The **Folder Options** dialog box appears.

1. Click the **File Types** tab.
1. In the **Registered file types** list, select **WAV Wave Sound**.
1. Click **Change**.

    The **Open With** dialog box appears.

1. In the **Programs** list, select **SA**.
1. Click **OK** twice.

### **If you are using Windows 2000**
1. On the **Tools** menu of Windows Explorer, click **Folder Options**.

    The **Folder Options** dialog box appears.

1. Click the **File Types** tab.
1. In the **Registered file types** list, select **WAV Wave Sound**.
1. Click **Advanced**.

    The **Edit File Type** dialog box appears.

1. Click **New**.

    The **Editing action for type: Wave Sound** dialog box appears.

1. In the **Application used to perform action** box, type the following:

    **"C:\\Program Files\\SIL\\Speech Analyzer\\Sa.exe" "%L"**

1. Do one of the following:
   * To be able to open *multiple* Wave Sound files, each in a *different* Speech Analyzer window, clear the **Use DDE** check box.
   * To be able to open *multiple* Wave Sound files, all in the *same* Speech Analyzer window, select the **Use DDE** check box, and then in the **DDE Message** box, type **[open ("%L")]**

1. Click **OK** three times.

#### **Related Topics**
[Open](open)