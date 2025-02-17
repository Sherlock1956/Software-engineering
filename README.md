# 文件备份与恢复工具

## 项目概述

本项目是一个基于MFC框架的文件备份与恢复工具，旨在为用户提供简单易用的文件备份、恢复、压缩、解压缩、加密和解密功能。用户可以通过图形界面选择源文件和目标目录，执行备份、恢复、压缩、解压缩、加密和解密操作。

## 功能特性

- **文件备份与恢复**：支持文件和目录的备份与恢复，保留文件的元数据（如时间戳、属性和权限）。
- **文件压缩与解压缩**：使用Huffman编码对文件进行压缩和解压缩，减少存储空间占用。
- **文件加密与解密**：使用AES-128加密算法对文件进行加密和解密，确保数据安全。
- **TAR文件打包与解包**：支持将多个文件或目录打包为TAR文件，并支持解包操作。
- **符号链接和硬链接处理**：支持备份和恢复符号链接和硬链接。
- **图形用户界面**：提供直观的图形界面，方便用户操作。

## 项目结构

- **BackupRestore.cpp**：应用程序的主入口和初始化代码。
- **BackupRestore.h**：应用程序类的声明。
- **BackupRestoreDlg.cpp**：主对话框的实现，包含备份、恢复、压缩、解压缩、加密、解密等功能的实现。
- **BackupRestoreDlg.h**：主对话框类的声明。

## 使用说明

### 1. 备份文件
1. 点击“选择源目录”按钮，选择要备份的文件或目录。
2. 点击“选择目标目录”按钮，选择备份文件保存的位置。
3. 点击“备份”按钮，开始备份操作。

### 2. 恢复文件
1. 点击“选择源目录”按钮，选择要恢复的备份文件或目录。
2. 点击“选择目标目录”按钮，选择恢复文件保存的位置。
3. 点击“恢复”按钮，开始恢复操作。

### 3. 压缩文件
1. 点击“选择源目录”按钮，选择要压缩的文件或目录。
2. 点击“选择目标目录”按钮，选择压缩文件保存的位置。
3. 点击“压缩备份”按钮，开始压缩操作。

### 4. 解压缩文件
1. 点击“选择源目录”按钮，选择要解压缩的文件。
2. 点击“选择目标目录”按钮，选择解压缩文件保存的位置。
3. 点击“解压恢复”按钮，开始解压缩操作。

### 5. 加密文件
1. 点击“选择源目录”按钮，选择要加密的文件或目录。
2. 点击“选择目标目录”按钮，选择加密文件保存的位置。
3. 在密码输入框中输入加密密码。
4. 点击“加密备份”按钮，开始加密操作。

### 6. 解密文件
1. 点击“选择源目录”按钮，选择要解密的文件。
2. 点击“选择目标目录”按钮，选择解密文件保存的位置。
3. 在密码输入框中输入解密密码。
4. 点击“解密恢复”按钮，开始解密操作。

### 7. 打包文件
1. 点击“选择源目录”按钮，选择要打包的文件或目录。
2. 点击“选择目标目录”按钮，选择打包文件保存的位置。
3. 在打包文件名输入框中输入打包文件的名称。
4. 点击“打包备份”按钮，开始打包操作。

### 8. 解包文件
1. 点击“选择源目录”按钮，选择要解包的TAR文件。
2. 点击“选择目标目录”按钮，选择解包文件保存的位置。
3. 点击“解包恢复”按钮，开始解包操作。

## 依赖项

- **MFC框架**：本项目基于MFC框架开发，需在支持MFC的环境下编译运行。
- **OpenSSL库**：用于AES加密和解密操作，需安装OpenSSL库并配置开发环境。

## 编译与运行

1. 使用Visual Studio打开项目文件。
2. 配置项目依赖项，确保OpenSSL库正确链接。
3. 编译并运行项目。

## 注意事项

- 在进行加密和解密操作时，请确保输入的密码正确，否则无法解密文件。
- 备份和恢复操作会保留文件的元数据（如时间戳、属性和权限），但某些特殊文件（如管道文件）可能会被跳过。
- 压缩和解压缩操作使用Huffman编码，适用于文本文件的压缩，对于已经压缩的文件（如图片、视频）效果可能不明显。

## 作者

- **姓名**：李宇潇
- **学号**：2021080907032
- **课程**：软件开发综合实验

## 联系方式

- **邮箱**：lee.liyuxiao@gmail.com
- **GitHub**：[GitHub主页](https://github.com/Sherlock1956)

---

感谢使用本文件备份与恢复工具！如有任何问题或建议，欢迎联系作者。
