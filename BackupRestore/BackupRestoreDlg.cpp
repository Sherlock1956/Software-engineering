#include "pch.h"
#include "framework.h"
#include "Resource.h"
#include "BackupRestore.h"  // 这是应用程序的主头文件，通常它会由 Visual Studio 创建
#include "BackupRestoreDlg.h"
#include "afxdialogex.h"
#include <afxdlgs.h>  // 用于 CFileDialog
#include <Shlwapi.h>  // 用于 PathFindFileName
#include <aclapi.h> // 用于权限和属主管理
#include <vector>  // 引入 vector 头文件
#include <algorithm>  // 引入 std::find

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CBackupRestoreDlg 对话框

CBackupRestoreDlg::CBackupRestoreDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_BACKUPRESTORE_DIALOG, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CBackupRestoreDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_SOURCE, m_sourceDirEdit);
    DDX_Control(pDX, IDC_EDIT_DEST, m_destDirEdit);
    DDX_Control(pDX, IDC_BUTTON_BACKUP, m_backupBtn);
    DDX_Control(pDX, IDC_BUTTON_RESTORE, m_restoreBtn);
    DDX_Control(pDX, IDC_BUTTON_SELECT_SOURCE, m_selectSourceBtn); // 源目录按钮
    DDX_Control(pDX, IDC_BUTTON_SELECT_DEST, m_selectDestBtn);     // 目标目录按钮
    DDX_Control(pDX, IDC_BUTTON_SELECT_SOURCE_DIR, m_selectSourceDirBtn);     // 目标目录按钮
}


BEGIN_MESSAGE_MAP(CBackupRestoreDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON_BACKUP, &CBackupRestoreDlg::OnBnClickedBackup)
    ON_BN_CLICKED(IDC_BUTTON_RESTORE, &CBackupRestoreDlg::OnBnClickedRestore)
    ON_BN_CLICKED(IDC_BUTTON_SELECT_SOURCE, &CBackupRestoreDlg::OnBnClickedSelectSource)  // 源目录选择按钮
    ON_BN_CLICKED(IDC_BUTTON_SELECT_DEST, &CBackupRestoreDlg::OnBnClickedSelectDest)      // 目标目录选择按钮
    ON_BN_CLICKED(IDC_BUTTON_SELECT_SOURCE_DIR, &CBackupRestoreDlg::OnBnClickedSelectSourceDir)  // 源目录选择按钮
END_MESSAGE_MAP()


BOOL CBackupRestoreDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    SetIcon(m_hIcon, TRUE);    // 设置大图标
    SetIcon(m_hIcon, FALSE);   // 设置小图标

    return TRUE;  // 除非设置了焦点，否则返回 TRUE
}

// 选择源目录
void CBackupRestoreDlg::OnBnClickedSelectSource()
{
    CString currentText;
    CString selectedPath;
    m_sourceDirEdit.GetWindowText(currentText);

    // 定义文件类型过滤器，允许用户指定特定文件格式
    CString fileFilter = _T("所有文件 (*.*)|*.*|文本文件 (*.txt)|*.txt|图像文件 (*.jpg;*.png)|*.jpg;*.png|");
    fileFilter += _T("文档文件 (*.doc;*.docx)|*.doc;*.docx||");

    CFileDialog fileDlg(TRUE, nullptr, nullptr, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT, fileFilter);

    fileDlg.m_ofn.lpstrTitle = _T("选择源文件");
    fileDlg.m_ofn.nMaxFile = 10000; // 允许选择多个文件，缓冲区大小
    CString fileBuffer;
    fileBuffer.GetBuffer(10000);
    fileDlg.m_ofn.lpstrFile = fileBuffer.GetBuffer();

    if (fileDlg.DoModal() == IDOK)
    {
        POSITION pos = fileDlg.GetStartPosition();
        while (pos != NULL)
        {
            if (!currentText.IsEmpty() && currentText.Right(2) != _T("\r\n"))
            {
                currentText += _T("\r\n");
            }
            selectedPath = fileDlg.GetNextPathName(pos);  // 获取选择的路径

            // 检查用户选择的文件是否符合要求的格式
            CString fileExt = selectedPath.Mid(selectedPath.ReverseFind(_T('.')) + 1).MakeLower(); // 获取扩展名
            currentText += selectedPath;

        }
        currentText += _T("\r\n");
        m_sourceDirEdit.SetWindowText(currentText);  // 更新源目录输入框
    }
}

// 检查文件是否为允许的格式
bool CBackupRestoreDlg::IsAllowedFileFormat(const CString& fileExt)
{
    // 允许的文件格式（根据需求调整）
    const std::vector<CString> allowedFormats = { _T("txt"), _T("jpg"), _T("png"), _T("doc"), _T("docx") };

    // 检查文件格式是否在允许的列表中
    return std::find(allowedFormats.begin(), allowedFormats.end(), fileExt) != allowedFormats.end();
}

void CBackupRestoreDlg::OnBnClickedSelectSourceDir()
{
    // 选择文件夹
    CString currentText;
    m_sourceDirEdit.GetWindowText(currentText);
    CFolderPickerDialog folderDlg;
    if (folderDlg.DoModal() == IDOK)
    {
        // 检查 currentText 是否为空，或者是否以 "\r\n" 结尾
        if (!currentText.IsEmpty() && currentText.Right(2) != _T("\r\n"))
        {
            currentText += _T("\r\n");
        }

        CString selectedFolder = folderDlg.GetPathName();  // 获取选择的文件夹路径
        currentText += selectedFolder;
    }
    currentText += _T("\r\n");
    // 更新源目录输入框
    m_sourceDirEdit.SetWindowText(currentText);
}

// 选择目标目录
void CBackupRestoreDlg::OnBnClickedSelectDest()
{
    // 使用 CFolderPickerDialog 选择文件夹
    CFolderPickerDialog folderDlg;
    if (folderDlg.DoModal() == IDOK)
    {
        CString selectedFolder = folderDlg.GetPathName();  // 获取选择的文件夹路径
        m_destDirEdit.SetWindowText(selectedFolder);       // 更新目标目录输入框
    }
}

int CBackupRestoreDlg::BackupFiles(const CString& sourcePath_list, CString& destDir)
{
    CString sourcePath;
    int startPos = 0;
    int delimiterPos = sourcePath_list.Find(_T("\r\n"), 0);

    while (delimiterPos != -1)
    {
        sourcePath = sourcePath_list.Mid(startPos, delimiterPos - startPos);
        CString saveDir = destDir;

        // 获取文件属性
        DWORD dwAttr = GetFileAttributes(sourcePath);
        if (dwAttr == INVALID_FILE_ATTRIBUTES)
        {
            AfxMessageBox(_T("无法访问源路径: ") + sourcePath);
            return 0; // 如果路径不可访问，退出函数
        }

        // 判断并处理文件类型
        if (dwAttr & FILE_ATTRIBUTE_REPARSE_POINT) // 符号链接
        {
            if (!HandleSymbolicLink(sourcePath, saveDir))
                return 0;
        }
        else if (!(dwAttr & FILE_ATTRIBUTE_DIRECTORY)) // 普通文件
        {
            if (!HandleFile(sourcePath, saveDir))
                return 0;
        }
        else if (dwAttr & FILE_ATTRIBUTE_DIRECTORY) // 目录
        {
            if (!HandleDirectory(sourcePath, saveDir))
                return 0;
        }
        else if (dwAttr & FILE_ATTRIBUTE_SYSTEM) // 管道文件
        {
            AfxMessageBox(_T("管道文件已跳过: ") + sourcePath);
        }

        startPos = delimiterPos + 2; // 跳过 \r\n
        delimiterPos = sourcePath_list.Find(_T("\r\n"), startPos);
    }

    return 1;
}

// 处理符号链接
BOOL CBackupRestoreDlg::HandleSymbolicLink(const CString& sourcePath, const CString& destDir)
{
    CString destLink = destDir + _T("\\") + GetFileNameFromPath(sourcePath);
    if (!CreateSymbolicLink(destLink, sourcePath, 0))
    {
        AfxMessageBox(_T("无法创建符号链接: ") + sourcePath);
        return false;
    }
    return true;
}

// 处理普通文件（包括硬链接）


BOOL CBackupRestoreDlg::HandleFile(const CString& sourcePath, const CString& saveDir)
{
    HANDLE hFile = CreateFile(sourcePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        AfxMessageBox(_T("无法打开文件: ") + sourcePath);
        return FALSE;
    }

    BY_HANDLE_FILE_INFORMATION fileInfo;
    if (!GetFileInformationByHandle(hFile, &fileInfo))
    {
        AfxMessageBox(_T("无法获取文件信息: ") + sourcePath);
        CloseHandle(hFile);
        return FALSE;
    }

    CString destFile = saveDir + _T("\\") + GetFileNameFromPath(sourcePath);
    if (fileInfo.nNumberOfLinks > 1) // 处理硬链接
    {
        if (!CreateHardLink(destFile, sourcePath, NULL))
        {
            AfxMessageBox(_T("无法创建硬链接: ") + sourcePath);
            CloseHandle(hFile);
            return FALSE;
        }
    }
    else // 处理普通文件
    {
        if (!CopyFile(sourcePath, destFile, FALSE))
        {
            AfxMessageBox(_T("无法复制文件: ") + sourcePath);
            CloseHandle(hFile);
            return FALSE;
        }

        // 打开目标文件以设置元数据
        HANDLE hDestFile = CreateFile(destFile, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hDestFile == INVALID_HANDLE_VALUE)
        {
            AfxMessageBox(_T("无法打开目标文件以设置元数据: ") + destFile);
            CloseHandle(hFile);
            return FALSE;
        }

        // 设置时间戳
        FILETIME creationTime, lastAccessTime, lastWriteTime;
        if (GetFileTime(hFile, &creationTime, &lastAccessTime, &lastWriteTime))
        {
            if (!SetFileTime(hDestFile, &creationTime, &lastAccessTime, &lastWriteTime))
            {
                AfxMessageBox(_T("无法设置文件时间: ") + destFile);
                CloseHandle(hFile);
                CloseHandle(hDestFile);
                return FALSE;
            }
        }

        // 设置文件属性
        DWORD fileAttributes = GetFileAttributes(sourcePath);
        if (fileAttributes != INVALID_FILE_ATTRIBUTES)
        {
            if (!SetFileAttributes(destFile, fileAttributes))
            {
                AfxMessageBox(_T("无法设置文件属性: ") + destFile);
                CloseHandle(hFile);
                CloseHandle(hDestFile);
                return FALSE;
            }
        }

        // 获取并设置安全描述符（属主和权限）
        PSECURITY_DESCRIPTOR pSD = NULL;
        DWORD sdSize = 0;
        if (GetFileSecurity(sourcePath, OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION, NULL, 0, &sdSize) ||
            GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            pSD = (PSECURITY_DESCRIPTOR)malloc(sdSize);
            if (pSD && GetFileSecurity(sourcePath, OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION, pSD, sdSize, &sdSize))
            {
                if (!SetFileSecurity(destFile, OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION, pSD))
                {
                    DWORD errorCode = GetLastError();
                    CString errorMsg;
                    errorMsg.Format(_T("无法设置文件安全描述符: %s\n错误代码: %lu"), destFile, errorCode);
                    AfxMessageBox(errorMsg);
                    free(pSD);
                    CloseHandle(hFile);
                    CloseHandle(hDestFile);
                    return FALSE;
                }
            }
            free(pSD);
        }

        CloseHandle(hDestFile);
    }

    CloseHandle(hFile);
    return TRUE;
}


// 处理目录
BOOL CBackupRestoreDlg::HandleDirectory(const CString& sourcePath, CString& saveDir)
{
    CFileFind finder;
    BOOL bWorking = finder.FindFile(sourcePath + _T("\\*"));

    if (!bWorking)
    {
        AfxMessageBox(_T("无法打开源文件夹: ") + sourcePath);
        return false;
    }

    // 若目标文件夹存在，创建下一级目录，如果不存在就创建本级目录
    if (!PathFileExists(saveDir))
    {
        CreateDirectory(saveDir, NULL);
    }
    else {
        int pos = sourcePath.ReverseFind(_T('\\'));
        if (pos != -1)
        {
            CString lastSegment = sourcePath.Mid(pos + 1); // 提取最后一段路径
            saveDir += _T("\\") + lastSegment;
            CreateDirectory(saveDir, NULL);
        }
    }


    while (bWorking)
    {
        bWorking = finder.FindNextFile();

        if (finder.IsDirectory())
        {
            // 跳过“.”和“..”目录
            if (finder.IsDots())
                continue;

            CString subSourceDir = finder.GetFilePath() + _T("\r\n");
            CString subDestDir = saveDir + _T("\\") + finder.GetFileName();
            if (!BackupFiles(subSourceDir, subDestDir)) // 递归调用
                return false;
        }
        else
        {
            // 复制文件
            CString sourceFile = finder.GetFilePath();
            CString destFile = saveDir + _T("\\") + finder.GetFileName();

            if (!HandleFile(sourceFile, saveDir))
            {
                AfxMessageBox(_T("无法复制文件: ") + sourceFile);
                return false;
            }
        }
    }
    return true;
}


// 辅助函数：从路径中提取文件名
CString CBackupRestoreDlg::GetFileNameFromPath(const CString& filePath)
{
    int pos = filePath.ReverseFind(_T('\\'));
    if (pos == -1)
        return filePath;  // 如果没有找到 '\\'，返回整个路径（假设路径是文件名）
    return filePath.Mid(pos + 1);  // 返回文件名部分
}



int CBackupRestoreDlg::RestoreFiles(const CString& backupPath_list, const CString& restoreDir)
{
    CString backupPath;
    int startPos = 0;
    int delimiterPos = backupPath_list.Find(_T("\r\n"), 0);

    while (delimiterPos != -1)
    {
        backupPath = backupPath_list.Mid(startPos, delimiterPos - startPos);
        CString restoreDirPath = restoreDir;

        // 获取文件属性
        DWORD dwAttr = GetFileAttributes(backupPath);
        if (dwAttr == INVALID_FILE_ATTRIBUTES)
        {
            AfxMessageBox(_T("无法访问备份路径: ") + backupPath);
            return 0; // 如果路径不可访问，退出函数
        }

        // 判断并处理文件类型
        if (dwAttr & FILE_ATTRIBUTE_REPARSE_POINT) // 符号链接
        {
            if (!HandleSymbolicLink(backupPath, restoreDirPath))
                return 0;
        }
        else if (!(dwAttr & FILE_ATTRIBUTE_DIRECTORY)) // 普通文件
        {
            if (!HandleFile(backupPath, restoreDirPath))
                return 0;
        }
        else if (dwAttr & FILE_ATTRIBUTE_DIRECTORY) // 目录
        {
            if (!HandleDirectory(backupPath, restoreDirPath))
                return 0;
        }
        else if (dwAttr & FILE_ATTRIBUTE_SYSTEM) // 管道文件
        {
            AfxMessageBox(_T("管道文件已跳过: ") + backupPath);
        }

        startPos = delimiterPos + 2; // 跳过 \r\n
        delimiterPos = backupPath_list.Find(_T("\r\n"), startPos);
    }

    return 1;
}


void CBackupRestoreDlg::OnBnClickedBackup()
{
    CString sourceDir, destDir;
    m_sourceDirEdit.GetWindowText(sourceDir);
    m_destDirEdit.GetWindowText(destDir);

    if (sourceDir.IsEmpty() || destDir.IsEmpty())
    {
        AfxMessageBox(_T("源目录和目标目录不能为空"));
        return;
    }

    if (BackupFiles(sourceDir, destDir)) {
        AfxMessageBox(_T("备份完成"));
    }
    else {
        AfxMessageBox(_T("备份出现问题"));
    };
    
}

void CBackupRestoreDlg::OnBnClickedRestore()
{
    CString backupDir, restoreDir;
    m_sourceDirEdit.GetWindowText(backupDir);
    m_destDirEdit.GetWindowText(restoreDir);

    if (backupDir.IsEmpty() || restoreDir.IsEmpty())
    {
        AfxMessageBox(_T("备份目录和恢复目录不能为空"));
        return;
    }

    if (RestoreFiles(backupDir, restoreDir)) {
        AfxMessageBox(_T("恢复完成"));
    };
    
}
