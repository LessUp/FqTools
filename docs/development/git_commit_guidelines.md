# Git 提交信息规范

## 🎯 目标

为了保持项目 Git 历史的清晰、可读和一致性，所有提交都必须遵循本规范。本规范基于 [Conventional Commits](https://www.conventionalcommits.org/) 和 [Gitmoji](https://gitmoji.dev/)。

## 📜 格式

每条提交信息都遵循以下格式：

```
<type>(<scope>): <emoji> <subject>
```

### Type (类型)

必须是以下关键字之一：

- **feat**: ✨ (新功能)
- **fix**: 🐛 (Bug 修复)
- **docs**: 📝 (文档更改)
- **style**: 🎨 (代码风格，不影响代码含义的更改，如格式化)
- **refactor**: ♻️ (代码重构，既不是修复 Bug 也不是添加新功能)
- **perf**: ⚡️ (性能优化)
- **test**: ✅ (添加或修改测试)
- **build**: 📦 (影响构建系统或外部依赖的更改，如 CMake, Conan)
- **ci**: 🚀 (CI/CD 配置文件和脚本的更改)
- **chore**: 🔧 (其他不修改源文件或测试文件的更改，如更新 `.gitignore`)

### Scope (范围)

可选字段，用于指定本次提交影响的范围。例如：`core`, `cli`, `parser`, `devops`, `docs`。

### Emoji

一个与提交类型或内容相关的 Emoji，以增加可读性。

### Subject (主题)

- 使用祈使句，现在时态，例如 "change" 而不是 "changed" 或 "changes"。
- 第一个字母不要大写。
- 结尾不加句号 (`.`)。

## ✅ 示例

```
feat(parser): ✨ add support for BAM file input
fix(core): 🐛 correct GC content calculation for paired-end reads
docs(readme): 📝 update installation instructions
refactor(pipeline): ♻️ simplify processing loop logic
perf(io): ⚡️ replace std::endl with '\n' to avoid flushing
ci(github): 🚀 add automated linting job to workflow
```

```