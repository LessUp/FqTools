# 贡献指南

我们非常欢迎并感谢您对 FastQTools 项目的贡献。本指南旨在帮助您顺利地参与到项目中来。

## 行为准则

我们期望所有贡献者都能遵守项目的行为准则。请确保您的言行是友好和互相尊重的。

## 如何贡献

### 报告 Bug

如果您发现了 Bug，请通过 [GitHub Issues](https://github.com/your-org/fastqtools/issues) 提交报告。请在报告中提供以下信息：
- 您使用的操作系统和工具链版本。
- 复现 Bug 的详细步骤。
- 相关的日志输出或截图。

### 提交功能请求

如果您有新的功能建议，也请通过 [GitHub Issues](https://github.com/your-org/fastqtools/issues) 提交。请详细描述您希望实现的功能和它的使用场景。

### 提交代码

我们欢迎通过 Pull Request (PR) 提交代码。请遵循以下流程：

1.  **Fork 仓库**: 首先，Fork 本项目到您的 GitHub 账户。
2.  **克隆您的 Fork**: `git clone https://github.com/YOUR_USERNAME/fastqtools.git`
3.  **创建分支**: `git checkout -b feature/your-new-feature`
4.  **进行修改**:
    - 确保您的代码遵循项目的 **[编码规范](./dev_coding_standards.md)**。
    - 为您的新功能或 Bug 修复添加必要的单元测试。
5.  **在本地运行检查**: 在提交前，请务必运行 lint 脚本以确保代码质量。
    ```bash
    ./scripts/lint.sh
    ```
6.  **提交您的更改**:
    - 使用清晰的提交信息。我们遵循 [Conventional Commits](https://www.conventionalcommits.org/) 规范。
    - 例如: `feat(parser): Add support for BAM file input` 或 `fix(stats): Correct GC content calculation`
7.  **推送分支**: `git push origin feature/your-new-feature`
8.  **创建 Pull Request**: 在 GitHub 上，向本项目的 `master` 分支发起一个新的 Pull Request。

您的 PR 将被审查，并在通过所有 CI 检查后被合并。