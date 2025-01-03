﻿#include "pch.h"
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
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <queue>
#include <bitset>
#include <string>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <afx.h>
#include <sstream>
#include <filesystem>
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
    DDX_Control(pDX, IDC_EDIT_PASSWORD, m_passwordEdit);
    DDX_Control(pDX, IDC_EDIT_PACK_NAME, m_packnameEdit);

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
    ON_BN_CLICKED(IDC_BUTTON_COMPRESS_BACKUP, &CBackupRestoreDlg::OnBnClickedCompressBackup)
    ON_BN_CLICKED(IDC_BUTTON_DECOMPRESS_RESTORE, &CBackupRestoreDlg::OnBnClickedDecompressRestore)
    ON_BN_CLICKED(IDC_BUTTON_ENCODE_BACKUP, &CBackupRestoreDlg::OnBnClickedEncryptBackup)
    ON_BN_CLICKED(IDC_BUTTON_DECODE_RESTORE, &CBackupRestoreDlg::OnBnClickedDecryptRestore)
    ON_EN_SETFOCUS(IDC_EDIT_PASSWORD, &CBackupRestoreDlg::OnEditSetFocus)
    ON_EN_KILLFOCUS(IDC_EDIT_PASSWORD, &CBackupRestoreDlg::OnEditKillFocus)
    ON_EN_SETFOCUS(IDC_EDIT_PACK_NAME, &CBackupRestoreDlg::OnEditSetFocus2)
    ON_EN_KILLFOCUS(IDC_EDIT_PACK_NAME, &CBackupRestoreDlg::OnEditKillFocus2)
    ON_BN_CLICKED(IDC_BUTTON_CLEAR_SOURCE, &CBackupRestoreDlg::OnBnClickedClearSource)
    ON_BN_CLICKED(IDC_BUTTON_PACK_BACKUP, &CBackupRestoreDlg::OnBnClickedPackFiles)
    ON_BN_CLICKED(IDC_BUTTON_UNPACK_RESTORE, &CBackupRestoreDlg::OnBnClickedUnPackFiles)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

BOOL CBackupRestoreDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    SetIcon(m_hIcon, TRUE);    // 设置大图标
    SetIcon(m_hIcon, FALSE);   // 设置小图标
    m_passwordEdit.SetWindowText(_T("请在此输入加密密码"));
    m_passwordEdit.SetSel(0, -1);  // 选择所有文本
    m_passwordEdit.SetFocus();     // 设置焦点
    m_packnameEdit.SetWindowTextW(_T("请在此输入打包文件名称"));
    m_packnameEdit.SetSel(0, -1);
    m_packnameEdit.SetFocus();

    return TRUE;  // 除非设置了焦点，否则返回 TRUE
}

HBRUSH CBackupRestoreDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

    if (pWnd->GetDlgCtrlID() == IDC_EDIT_PASSWORD)  // 判断是否是指定的编辑框
    {
        // 设置灰色字体
        pDC->SetTextColor(RGB(169, 169, 169));  // 灰色
        pDC->SetBkMode(TRANSPARENT);            // 背景透明
    }
    if (pWnd->GetDlgCtrlID() == IDC_EDIT_PACK_NAME)  // 判断是否是指定的编辑框
    {
        // 设置灰色字体
        pDC->SetTextColor(RGB(169, 169, 169));  // 灰色
        pDC->SetBkMode(TRANSPARENT);            // 背景透明
    }

    return hbr;
}

// 设置提示文本框的焦点状态
void CBackupRestoreDlg::OnEditSetFocus()
{
    m_passwordEdit.SetWindowText(_T(""));
}

void CBackupRestoreDlg::OnEditSetFocus2()
{
    m_packnameEdit.SetWindowText(_T(""));
}

void CBackupRestoreDlg::OnEditKillFocus()
{
    CString currentText;
    m_passwordEdit.GetWindowText(currentText);
    // 如果文本框为空，则恢复提示文本
    if (currentText.IsEmpty())
    {
        m_passwordEdit.SetWindowText(_T("请在此输入加密密码"));
    }
}

void CBackupRestoreDlg::OnEditKillFocus2()
{
    CString currentText;
    m_packnameEdit.GetWindowText(currentText);
    // 如果文本框为空，则恢复提示文本
    if (currentText.IsEmpty())
    {
        m_packnameEdit.SetWindowText(_T("请在此输入打包文件名称"));
    }
}

std::streamsize GetFileSize(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    return file.tellg();
}

// 填充块，tar文件打包最后需要加两个空白文件块
void WritePaddingBlock(std::ofstream& tarFile) {
    std::vector<char> endBlock(512, '\0');
    tarFile.write(endBlock.data(), 512); // 第一个空块
    tarFile.write(endBlock.data(), 512); // 第二个空块
}

// 填充tar文件头部信息
void WriteTarHeader(std::ofstream& tarFile, const std::string& fileName, std::streamsize fileSize) {
    // 初始化 512 字节的 Header Block
    std::vector<char> header(512, 0);
    // std::string local_fileName = fileName.substr(fileName.rfind("\\")+1);
    // 填写文件名 (100 字节)
    std::memcpy(header.data(), fileName.c_str(), min(fileName.size(), size_t(100)));

    // 填写文件权限 (8 字节，八进制，默认 0644)
    std::memcpy(header.data() + 100, "0000777\0", 8);

    // 填写用户 ID (8 字节，八进制，默认 0)
    std::memcpy(header.data() + 108, "0000000\0", 8);

    // 填写组 ID (8 字节，八进制，默认 0)
    std::memcpy(header.data() + 116, "0000000\0", 8);

    // 填写文件大小 (12 字节，八进制表示)
    std::ostringstream sizeStream;
    sizeStream << std::setw(11) << std::setfill('0') << std::oct << fileSize;
    std::string sizeStr = sizeStream.str();
    std::memcpy(header.data() + 124, sizeStr.c_str(), sizeStr.size());
    header[124 + sizeStr.size()] = '\0';

    // 填写修改时间 (12 字节，八进制，默认 0)
    std::memcpy(header.data() + 136, "00000000000\0", 12);

    // 填写文件类型标志 (typeflag)
    header[156] = '0';

    // 填写 magic 和 version
    std::memcpy(header.data() + 257, "ustar ", 6);
    std::memcpy(header.data() + 263, "  ", 2);

    // 填充校验和字段
    std::fill(header.begin() + 148, header.begin() + 156, ' '); // 先填充空格
    unsigned int checksum = 0;
    for (const char& byte : header) {
        checksum += static_cast<unsigned char>(byte);
    }
    std::ostringstream chksumStream;
    chksumStream << std::setw(6) << std::setfill('0') << std::oct << checksum;
    std::string chksumStr = chksumStream.str();
    chksumStr.append(2, '\0');  // 确保最后两个字节是 '\0 '
    std::memcpy(header.data() + 148, chksumStr.c_str(), 8);

    // 写入 TAR 文件头
    tarFile.write(header.data(), header.size());
}

// 将单个文件写入tar文件
void AddFileToTar(std::ofstream& tarFile, const std::string& filePath) {
    // 打开输入文件
    std::ifstream inputFile(filePath, std::ios::binary);
    if (!inputFile.is_open()) {
        std::cerr << "无法打开文件: " << filePath << std::endl;
        return;
    }

    // 获取文件大小
    std::streamsize fileSize = GetFileSize(filePath);


    // 写入 TAR 文件头
    std::string local_fileName = filePath.substr(filePath.rfind("\\") + 1);
    WriteTarHeader(tarFile, local_fileName, fileSize);

    // 写入文件数据
    std::vector<char> buffer(512);
    while (inputFile.read(buffer.data(), buffer.size())) {
        tarFile.write(buffer.data(), buffer.size());
    }
    tarFile.write(buffer.data(), inputFile.gcount()); // 写入最后一块

    // 填充到 512 字节的倍数
    size_t paddingSize = 512 - (fileSize % 512);
    if (paddingSize < 512) {
        std::vector<char> padding(paddingSize, '\0');
        tarFile.write(padding.data(), padding.size());
    }
}

// 遍历目录并将文件添加到 tar 文件中
void AddDirectoryToTar(std::ofstream& tarFile, const std::string& dirPath) {
    for (const auto& entry : std::filesystem::recursive_directory_iterator(dirPath)) {
        if (entry.is_regular_file()) {
            AddFileToTar(tarFile, entry.path().string());
        }
    }
}

// 主函数，打包文件或文件夹
void CreateTarArchive(const std::string& outputTarFile, const std::vector<std::string>& filePaths) {
    std::ofstream tarFile(outputTarFile, std::ios::binary);
    if (!tarFile.is_open()) {
        AfxMessageBox(_T("无法创建 TAR 文件"));
        return;
    }

    for (const auto& path : filePaths) {
        if (std::filesystem::is_directory(path)) {
            AddDirectoryToTar(tarFile, path); // 如果是文件夹，递归处理文件夹
        }
        else if (std::filesystem::is_regular_file(path)) {
            AddFileToTar(tarFile, path); // 如果是文件，直接添加
        }
    }
    WritePaddingBlock(tarFile);

    tarFile.close();
    AfxMessageBox(_T("文件打包完成，如需压缩，请使用压缩备份功能"));
}

void CBackupRestoreDlg::OnBnClickedPackFiles() {
    CString sourceDir, outputDir, packName;
    m_sourceDirEdit.GetWindowText(sourceDir);
    m_destDirEdit.GetWindowText(outputDir);
    m_packnameEdit.GetWindowText(packName);
    if (packName == "请在此输入打包文件名称") {
        AfxMessageBox(_T("请输入打包文件名称"));
        return;
    }
    CString outputTarFile = outputDir + _T("\\") + packName + _T(".tar");
    if (sourceDir.IsEmpty() || outputTarFile.IsEmpty()) {
        AfxMessageBox(_T("源目录和目标文件路径不能为空"));
        return;
    }
    CString sourcePath;
    int startPos = 0;
    int delimiterPos = sourceDir.Find(_T("\r\n"), 0);
    std::vector<std::string> filePaths;
    while (delimiterPos != -1)
    {
        sourcePath = sourceDir.Mid(startPos, delimiterPos - startPos);
        filePaths.push_back(std::string(CT2A(sourcePath)));
        startPos = delimiterPos + 2; // 跳过 \r\n
        delimiterPos = sourceDir.Find(_T("\r\n"), startPos);
    }
    CreateTarArchive(std::string(CT2A(outputTarFile)), filePaths);
}

void ExtractTarHeader(const std::vector<char>& header, std::string& fileName, size_t& fileSize) {
    // 解析文件名（100字节）
    fileName.assign(header.data(), 100);
    fileName = fileName.c_str(); // 去除可能存在的尾部空字符

    // 解析文件大小（12字节，八进制表示）
    std::string sizeStr(header.data() + 124, 12);
    fileSize = std::stoul(sizeStr, nullptr, 8);
}

void CBackupRestoreDlg::OnBnClickedUnPackFiles() {
    std::string tarFilePath;
    std::string outputDir;
    CString sourceFile, targetDir;
    m_sourceDirEdit.GetWindowText(sourceFile);
    m_destDirEdit.GetWindowText(targetDir);
    outputDir = CT2A(targetDir);
    int startPos = 0;
    int delimiterPos = sourceFile.Find(_T("\r\n"), 0);
    while (delimiterPos != -1)
    {
        tarFilePath = CT2A(sourceFile.Mid(startPos, delimiterPos - startPos));
        std::ifstream tarFile(tarFilePath, std::ios::binary);
        if (!tarFile.is_open()) {
            CString message;
            message.Format(_T("无法打开 TAR 文件: %s", tarFilePath));
            AfxMessageBox(message);
            return;
        }

        while (true) {
            // 读取头部信息（512字节）
            std::vector<char> header(512, 0);
            tarFile.read(header.data(), 512);

            // 检查是否是结束块（全零）
            if (tarFile.gcount() == 0 || std::all_of(header.begin(), header.end(), [](char c) { return c == '\0'; })) {
                break;
            }

            // 解析文件头部信息
            std::string fileName;
            size_t fileSize = 0;
            ExtractTarHeader(header, fileName, fileSize);

            if (fileName.empty()) {
                break; // 无效的文件名，退出
            }

            // 目标文件路径
            std::string outputPath = outputDir + "\\" + fileName;
            std::filesystem::path outputFilePath(outputPath);
            if (!std::filesystem::exists(outputFilePath.parent_path())) {
                std::filesystem::create_directories(outputFilePath.parent_path());
            }

            // 读取文件内容
            std::ofstream outputFile(outputPath, std::ios::binary);
            if (!outputFile.is_open()) {
                CString message;
                message.Format(_T("无法创建文件: %s", outputPath));
                AfxMessageBox(message);
                tarFile.seekg((fileSize + 511) / 512 * 512, std::ios::cur); // 跳过文件内容
                continue;
            }

            std::vector<char> buffer(512);
            size_t remainingSize = fileSize;
            while (remainingSize > 0) {
                size_t readSize = min(remainingSize, buffer.size());
                tarFile.read(buffer.data(), readSize);
                outputFile.write(buffer.data(), readSize);
                remainingSize -= readSize;
            }

            outputFile.close();

            // 跳过填充到512字节的部分
            size_t paddingSize = 512 - (fileSize % 512);
            if (paddingSize < 512) {
                tarFile.seekg(paddingSize, std::ios::cur);
            }
        }
        tarFile.close();
        startPos = delimiterPos + 2; // 跳过 \r\n
        delimiterPos = sourceFile.Find(_T("\r\n"), startPos);
    }
    CString message;
    message.Format(_T("解包完成，如需解压，请使用解压恢复功能"));
    AfxMessageBox(message);
}

// 从用户密码和盐值派生key和iv
void DeriveKeyAndIv(const std::string& password, const unsigned char* salt, unsigned char* key, unsigned char* iv) {
    EVP_BytesToKey(EVP_aes_128_cbc(), EVP_sha256(), salt,
        reinterpret_cast<const unsigned char*>(password.c_str()), password.length(), 1, key, iv);
}

void AES_Encrypt(const std::vector<unsigned char>& input, std::vector<unsigned char>& output,
    const unsigned char* key, const unsigned char* iv) {
    // 创建和初始化上下文
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP_CIPHER_CTX");
    }

    // 初始化加密操作，使用 AES-128-CBC 模式
    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptInit_ex failed");
    }

    // 添加填充
    EVP_CIPHER_CTX_set_padding(ctx, 1);

    // 分配输出缓冲区，大小要包括可能的填充字节
    int blockSize = EVP_CIPHER_block_size(EVP_aes_128_cbc());
    int maxOutputSize = input.size() + blockSize;
    output.resize(maxOutputSize);

    // 加密数据
    int outputLen = 0;
    int totalLen = 0;
    if (EVP_EncryptUpdate(ctx, output.data(), &outputLen, input.data(), input.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptUpdate failed");
    }
    totalLen += outputLen;

    // 处理加密最后一块数据
    if (EVP_EncryptFinal_ex(ctx, output.data() + totalLen, &outputLen) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptFinal_ex failed");
    }
    totalLen += outputLen;

    // 调整输出缓冲区大小
    output.resize(totalLen);

    // 清理上下文
    EVP_CIPHER_CTX_free(ctx);
}

void AES_Decrypt(const std::vector<unsigned char>& input, std::vector<unsigned char>& output,
    const unsigned char* key, const unsigned char* iv) {
    // 创建和初始化上下文
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP_CIPHER_CTX");
    }

    // 初始化解密操作，使用 AES-128-CBC 模式
    if (EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_DecryptInit_ex failed");
    }

    // 分配输出缓冲区，大小为输入大小的最大可能长度
    output.resize(input.size());

    int outputLen = 0;
    int totalLen = 0;

    // 解密数据
    if (EVP_DecryptUpdate(ctx, output.data(), &outputLen, input.data(), input.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_DecryptUpdate failed");
    }
    totalLen += outputLen;

    // 处理解密最后一块数据（包括去除填充）
    if (EVP_DecryptFinal_ex(ctx, output.data() + totalLen, &outputLen) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_DecryptFinal_ex failed");
    }
    totalLen += outputLen;

    // 调整输出缓冲区大小，去掉填充部分
    output.resize(totalLen);

    // 清理上下文
    EVP_CIPHER_CTX_free(ctx);
}

void CBackupRestoreDlg::OnBnClickedEncryptBackup() {
    CString sourceDir, destDir;
    CString inputFilePath, outputFilePath;
    m_sourceDirEdit.GetWindowText(sourceDir);
    m_destDirEdit.GetWindowText(destDir);
    if (sourceDir.IsEmpty() || destDir.IsEmpty())
    {
        AfxMessageBox(_T("源目录和目标目录不能为空"));
        return;
    }

    // 提示用户输入密码

    CString password_t;
    m_passwordEdit.GetWindowText(password_t);
    if (password_t == _T("") || password_t == _T("请在此输入加密密码")) {
        AfxMessageBox(_T("请输入加密密码"));
        return;
    }
    std::string password = CT2A(password_t);

    // 生成盐值（salt），它将和密码一起用于派生密钥和 IV
    unsigned char salt[8];
    if (!RAND_bytes(salt, sizeof(salt))) {
        AfxMessageBox(_T("Failed to generate salt"));
        return;
    }

    unsigned char aesKey[16];  // 128-bit key
    unsigned char aesIv[AES_BLOCK_SIZE];  // 128-bit IV
    DeriveKeyAndIv(password, salt, aesKey, aesIv);

    int startPos = 0;
    int delimiterPos = sourceDir.Find(_T("\r\n"), 0);

    while (delimiterPos != -1) {
        inputFilePath = sourceDir.Mid(startPos, delimiterPos - startPos);
        CString destFile = destDir + _T("\\") + GetEncryptedFileNameFromPath(inputFilePath);

        std::ifstream inputFile(inputFilePath, std::ios::binary);
        if (!inputFile.is_open()) {
            CString message;
            message.Format(_T("无法打开文件: %s", inputFilePath));
            AfxMessageBox(message);
            return;
        }

        std::vector<unsigned char> fileData((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
        inputFile.close();

        std::vector<unsigned char> encryptedData;
        AES_Encrypt(fileData, encryptedData, aesKey, aesIv);

        std::ofstream outputFile(destFile, std::ios::binary);
        if (!outputFile.is_open()) {
            CString message;
            message.Format(_T("Failed to open destination file: %s", destFile));
            AfxMessageBox(message);
            return;
        }

        // 写入盐值
        outputFile.write(reinterpret_cast<const char*>(salt), sizeof(salt));

        // 写入加密后的数据
        outputFile.write(reinterpret_cast<const char*>(encryptedData.data()), encryptedData.size());
        outputFile.close();

        startPos = delimiterPos + 2;  // 跳过 \r\n
        delimiterPos = sourceDir.Find(_T("\r\n"), startPos);
    }
    AfxMessageBox(_T("加密备份完成"));
}

void CBackupRestoreDlg::OnBnClickedDecryptRestore() {
    CString sourceDir, destDir;
    CString inputFilePath, outputFilePath;
    m_sourceDirEdit.GetWindowText(sourceDir);
    m_destDirEdit.GetWindowText(destDir);
    if (sourceDir.IsEmpty() || destDir.IsEmpty())
    {
        AfxMessageBox(_T("源目录和目标目录不能为空"));
        return;
    }
    // 提示用户输入密码
    CString password_t;
    m_passwordEdit.GetWindowText(password_t);
    if (password_t == _T("") || password_t == _T("请在此输入加密密码")) {
        AfxMessageBox(_T("请输入加密密码"));
        return;
    }
    std::string password = CT2A(password_t);
    unsigned char aesKey[16];  // 128-bit key
    unsigned char aesIv[AES_BLOCK_SIZE];  // 128-bit IV
    bool passwordCorrect = false;

    int startPos = 0;
    int delimiterPos = sourceDir.Find(_T("\r\n"), 0);

    while (delimiterPos != -1) {
        inputFilePath = sourceDir.Mid(startPos, delimiterPos - startPos);
        std::ifstream inputFile(inputFilePath, std::ios::binary);
        if (!inputFile.is_open()) {
            CString message;
            message.Format(_T("无法打开文件: %s", inputFilePath));
            AfxMessageBox(message);
            return;
        }

        // 读取盐值
        unsigned char salt[8];
        inputFile.read(reinterpret_cast<char*>(salt), sizeof(salt));

        std::vector<unsigned char> encryptedData((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
        inputFile.close();

        // 使用密码和盐值生成密钥和 IV
        DeriveKeyAndIv(password, salt, aesKey, aesIv);

        // 解密数据
        std::vector<unsigned char> decryptedData;
        try {
            // 解密数据
            std::vector<unsigned char> decryptedData;
            AES_Decrypt(encryptedData, decryptedData, aesKey, aesIv);
            // 输出解密后的数据到目标文件
            CString destFile = destDir + _T("\\") + GetDecryptedFileNameFromPath(inputFilePath);
            std::ofstream outputFile(destFile, std::ios::binary);
            if (!outputFile.is_open()) {
                CString message;
                message.Format(_T("无法打开文件: %s", destFile));
                AfxMessageBox(message);
                return;
            }
            outputFile.write(reinterpret_cast<const char*>(decryptedData.data()), decryptedData.size());
            outputFile.close();
        }
        catch (const std::exception& e) {
            AfxMessageBox(_T("密码不正确，无法解密文件。"));
            return;
        }
        startPos = delimiterPos + 2;  // 跳过 \r\n
        delimiterPos = sourceDir.Find(_T("\r\n"), startPos);
    }

    AfxMessageBox(_T("解密恢复完成"));
}

// Huffman 节点
struct HuffmanNode {
    unsigned char data;  // 字节值（0-255）
    int freq;            // 字节频率
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(unsigned char d, int f) : data(d), freq(f), left(nullptr), right(nullptr) {}
};

// 将二进制字符串转换为字节流
std::vector<unsigned char> PackBitsToBytes(const std::string& binaryString) {
    std::vector<unsigned char> byteStream;
    size_t length = binaryString.size();    // 遍历整个字符串，每次处理 8 位
    for (size_t i = 0; i < length; i += 8) {
        // 获取当前 8 位的数据，如果不足 8 位，补充 '0' 到右侧
        std::string byteString = binaryString.substr(i, 8);
        if (byteString.size() < 8) {
            // 右侧填充 '0'
            byteString.append(8 - byteString.size(), '0');
            std::bitset<8> byte(byteString);
            // 将 bitset 转换为 unsigned char 并添加到字节流中
            byteStream.push_back(static_cast<unsigned char>(byte.to_ulong()));
            return byteStream;
        }
        // 将补充后的 byteString 转换为 std::bitset<8>
        std::bitset<8> byte(byteString);
        // 将 bitset 转换为 unsigned char 并添加到字节流中
        byteStream.push_back(static_cast<unsigned char>(byte.to_ulong()));
    }
    return byteStream;
}

void DeleteHuffmanTree(HuffmanNode* node) {
    if (!node) return;
    DeleteHuffmanTree(node->left);
    DeleteHuffmanTree(node->right);
    delete node;
}

// 比较器用于优先队列
struct Compare {
    bool operator()(HuffmanNode* a, HuffmanNode* b) {
        return (a->freq == b->freq) ? (a->data > b->data) : (a->freq > b->freq);
    }
};

// 生成Huffman树
HuffmanNode* BuildHuffmanTree(const std::vector<std::pair<unsigned char, int>> freqVector) {
    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, Compare> pq;

    // 创建叶节点并加入优先队列
    for (const auto& entry : freqVector) {
        pq.push(new HuffmanNode(entry.first, entry.second));
    }

    // 合并节点构建树
    while (pq.size() > 1) {
        HuffmanNode* left = pq.top(); pq.pop();
        HuffmanNode* right = pq.top(); pq.pop();

        HuffmanNode* parent = new HuffmanNode(0, left->freq + right->freq);
        parent->left = left;
        parent->right = right;
        pq.push(parent);
    }
    return pq.top();  // 根节点
}

// 根据Huffman树对每个字节生成对应编码
void GenerateHuffmanCodes(HuffmanNode* root, const std::string& code,std::unordered_map<unsigned char, std::string>& huffmanCodes) {
    if (!root) return;

    // 叶节点：存储编码
    if (!root->left && !root->right) {
        huffmanCodes[root->data] = code;
    }

    GenerateHuffmanCodes(root->left, code + "0", huffmanCodes);
    GenerateHuffmanCodes(root->right, code + "1", huffmanCodes);
}

void CBackupRestoreDlg::OnBnClickedCompressBackup() {
    CString sourceDir;
    CString inputFilePath, outputFilePath;
    m_sourceDirEdit.GetWindowText(sourceDir);
    m_destDirEdit.GetWindowText(outputFilePath);
    if (sourceDir.IsEmpty() || outputFilePath.IsEmpty())
    {
        AfxMessageBox(_T("源目录和目标目录不能为空"));
        return;
    }
    
    int startPos = 0;
    int delimiterPos = sourceDir.Find(_T("\r\n"), 0);
    
    while (delimiterPos != -1)
    {
        inputFilePath = sourceDir.Mid(startPos, delimiterPos - startPos);
        CString destFile = outputFilePath + _T("\\") + GetCompressedFileNameFromPath(inputFilePath);
        // 统计字符频率
        std::unordered_map<unsigned char, int> frequencyTable;
        std::ifstream inputFile(inputFilePath, std::ios::binary);
        if (!inputFile.is_open()) {
            CString message;
            message.Format(_T("无法打开文件: %s", inputFilePath));
            AfxMessageBox(message);
            return;
        }
        char ch;
        while (inputFile.read(reinterpret_cast<char*>(&ch), sizeof(byte))) {
            frequencyTable[static_cast<unsigned char>(ch)]++;
        }
        std::vector<std::pair<unsigned char, int>> freqVector(frequencyTable.begin(), frequencyTable.end());
        std::sort(freqVector.begin(), freqVector.end(),
            [](const std::pair<unsigned char, int>& a, const std::pair<unsigned char, int>& b) {
                return a.first < b.first; // 按照字符值升序排序
            });
        inputFile.clear();
        inputFile.seekg(0, std::ios::beg);

        // 构建 Huffman 树
        HuffmanNode* root = BuildHuffmanTree(freqVector);

        // 生成编码表
        std::unordered_map<unsigned char, std::string> huffmanCodes;
        GenerateHuffmanCodes(root, "", huffmanCodes);

        // 编码文件内容
        std::string encodedData;
        while (inputFile.get(ch)) {
            unsigned char ch_t = static_cast<unsigned char>(ch);
            std::string data_t = huffmanCodes[ch_t];
            encodedData += data_t;
        }
        inputFile.close();
        std::vector<unsigned char> packedData = PackBitsToBytes(encodedData);
        // 将编码和 Huffman 树写入文件
        std::ofstream outputFile(destFile, std::ios::binary);
        outputFile << frequencyTable.size() << "\r\n";
        for (const auto& entry : frequencyTable) {
            outputFile << static_cast<unsigned int>(static_cast<unsigned char>(entry.first)) << ":" << entry.second << "\r\n";
        }
        outputFile << "###\r\n";  // 元数据结束标记
        // 写入压缩后的数据
        outputFile.write(reinterpret_cast<const char*>(packedData.data()), packedData.size());
        outputFile.close();
        DeleteHuffmanTree(root);
        startPos = delimiterPos + 2; // 跳过 \r\n
        delimiterPos = sourceDir.Find(_T("\r\n"), startPos);
    }
    AfxMessageBox(_T("压缩备份完成"));
}

void CBackupRestoreDlg::OnBnClickedDecompressRestore() {
    CString compressedFilePath_list, outputFilePath, compressedFilePath;
    m_sourceDirEdit.GetWindowText(compressedFilePath_list);
    m_destDirEdit.GetWindowText(outputFilePath);
    if (compressedFilePath_list.IsEmpty() || outputFilePath.IsEmpty())
    {
        AfxMessageBox(_T("源目录和目标目录不能为空"));
        return;
    }

    int startPos = 0;
    int delimiterPos = compressedFilePath_list.Find(_T("\r\n"), 0);

    while (delimiterPos != -1)
    {
        compressedFilePath = compressedFilePath_list.Mid(startPos, delimiterPos - startPos);
        CString saveFilePath = outputFilePath + _T("\\") + GetDeCompressedFileNameFromPath(compressedFilePath);
        std::ifstream inputFile(compressedFilePath,std::ios::binary);
        if (!inputFile.is_open()) {
            AfxMessageBox(_T("Failed to open file: %s", compressedFilePath));
            return;
        }

        // 读取 Huffman 树元数据
        std::unordered_map<unsigned char, int> frequencyTable;
        std::string line;
        std::getline(inputFile, line);
        int freqTableSize = std::stoi(line);
        
        while (std::getline(inputFile, line)) {
            if (line == "###\r") 
                break;  // 遇到结束标记时停止读取

            size_t delimiterPos = line.find(':');
            if (delimiterPos == std::string::npos) {
                AfxMessageBox(_T("Invalid frequency table entry:  %s", line));
                return;
            }

            // 提取字符的 ASCII 值并转换回字符类型
            int byteValue = std::stoi(line.substr(0, delimiterPos));
            int freq = std::stoi(line.substr(delimiterPos + 1));

            // 还原为字符并存储到频率表中
            frequencyTable[static_cast<unsigned char>(byteValue)] = freq;
        }
        // 构建 Huffman 树
        std::vector<std::pair<unsigned char, int>> freqVector(frequencyTable.begin(), frequencyTable.end());
        std::sort(freqVector.begin(), freqVector.end(),
            [](const std::pair<unsigned char, int>& a, const std::pair<unsigned char, int>& b) {
                return a.first < b.first; // 按照字符值升序排序
            });
        HuffmanNode* root = BuildHuffmanTree(freqVector);
        printf("%d, %d", frequencyTable['p'], frequencyTable['P']);

        // 解码数据
        HuffmanNode* currentNode = root;
        std::ofstream outputFile(saveFilePath, std::ios::binary);

        char bitChar;
        while (inputFile.get(bitChar)) {
            std::bitset<8> bits(bitChar);
            for (int i = 7; i >= 0; --i) {
                if (bits[i] == 0) {
                    currentNode = currentNode->left;
                }
                else {
                    currentNode = currentNode->right;
                }

                if (!currentNode->left && !currentNode->right) {
                    outputFile.put(currentNode->data);
                    currentNode = root;
                }
            }
        }
        inputFile.close();
        outputFile.close();
        DeleteHuffmanTree(root);
        startPos = delimiterPos + 2; // 跳过 \r\n
        delimiterPos = compressedFilePath_list.Find(_T("\r\n"), startPos);
    }
    AfxMessageBox(_T("解压恢复完成"));
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
                    errorMsg = _T("无法设置文件安全描述符: ") + destFile + _T("\n请以管理员身份运行程序");
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

CString CBackupRestoreDlg::GetCompressedFileNameFromPath(const CString& filePath)
{
    int pos = filePath.ReverseFind(_T('\\'));
    if (pos == -1)
        return filePath;  // 如果没有找到 '\\'，返回整个路径（假设路径是文件名）
    CString tem_name = filePath.Mid(pos + 1);  // 返回文件名部分
    return tem_name + _T(".huff");
}

CString CBackupRestoreDlg::GetDeCompressedFileNameFromPath(const CString& filePath)
{
    int pos = filePath.ReverseFind(_T('\\'));
    if (pos == -1)
        return filePath;  // 如果没有找到 '\\'，返回整个路径（假设路径是文件名）
    CString tem_name = filePath.Mid(pos + 1);  // 返回文件名部分
    pos = tem_name.ReverseFind(_T('.huff'));
    return tem_name.Left(pos + 1);
}

CString CBackupRestoreDlg::GetEncryptedFileNameFromPath(const CString& filePath)
{
    int pos = filePath.ReverseFind(_T('\\'));
    if (pos == -1)
        return filePath;  // 如果没有找到 '\\'，返回整个路径（假设路径是文件名）
    CString tem_name = filePath.Mid(pos + 1);  // 返回文件名部分
    return tem_name + _T(".aes");
}

CString CBackupRestoreDlg::GetDecryptedFileNameFromPath(const CString& filePath)
{
    int pos = filePath.ReverseFind(_T('\\'));
    if (pos == -1)
        return filePath;  // 如果没有找到 '\\'，返回整个路径（假设路径是文件名）
    CString tem_name = filePath.Mid(pos + 1);  // 返回文件名部分
    pos = tem_name.ReverseFind(_T('.aes '));
    return tem_name.Left(pos + 1);
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

// 清空源文件
void CBackupRestoreDlg::OnBnClickedClearSource() {
    m_sourceDirEdit.SetWindowText(_T(""));
}
