class CBackupRestoreDlg : public CDialogEx
{
public:
    CBackupRestoreDlg(CWnd* pParent = nullptr);   // 标准构造函数

    // 对话框数据
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_BACKUPRESTOREAPP_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

protected:
    HICON m_hIcon;

    // 备份和恢复函数
    BOOL CopyFileCustom(const CString& sourceFile, const CString& destFile);
    BOOL BackupFiles(const CString& sourceDir, CString& destDir);
    BOOL RestoreFiles(const CString& backupDir, const CString& restoreDir);
    CString GetFileNameFromPath(const CString& filePath);
    BOOL HandleSymbolicLink(const CString& sourcePath, const CString& destDir);
    BOOL HandleFile(const CString& sourcePath, const CString& saveDir);
    BOOL HandleDirectory(const CString& sourcePath, CString& saveDir);
    bool IsAllowedFileFormat(const CString& fileExt);

    // 控件变量
    CEdit m_sourceDirEdit;
    CEdit m_destDirEdit;
    CButton m_backupBtn;
    CButton m_restoreBtn;
    CButton m_selectSourceBtn; // 选择源目录按钮
    CButton m_selectDestBtn;   // 选择目标目录按钮
    CButton m_selectSourceDirBtn;

    // 生成的消息映射函数
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedBackup();
    afx_msg void OnBnClickedRestore();
    afx_msg void OnBnClickedSelectSource(); // 选择源目录
    afx_msg void OnBnClickedSelectDest();   // 选择目标目录
    afx_msg void OnBnClickedSelectSourceDir(); // 选择源目录
    DECLARE_MESSAGE_MAP()
};