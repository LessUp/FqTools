# 2025-10-21 文档重构（Phase 3）

## 背景
为落实“删除过时/不必要文档，分类设计与使用文档，优化目录结构（KISS）”的目标，对 `docs/` 进行第三阶段重构。

## 变更摘要
- 新增分类目录：`docs/design/`、`docs/development/`、`docs/_archive/`。
- 归档内部与历史材料：`docs/internal/` 全量迁移至 `docs/_archive/internal/`；旧 `contributing.md`、`cpp20_modules_migration_plan.md` 归档至 `docs/_archive/`。
- 重新归类开发与设计：
  - `docs/dev/architecture.md` → `docs/design/architecture.md`
  - `docs/dev/build.md` → `docs/development/build.md`
  - `docs/dev/coding_standards.md` → `docs/development/coding_standards.md`
  - `docs/dev/git_commit_guidelines.md` → `docs/development/git_commit_guidelines.md`
- 更新文档索引：重写 `docs/README.md`，对新结构进行导航；修正根 `README.md` 的目录树与“Developer Guide”链接。
- 修复断链：
  - `docs/development/build.md` 中的旧链接 `docs/dev/...` 改为新路径。

## 新目录结构
```
docs/
├── README.md
├── user/
│   ├── installation.md
│   ├── quick_start.md
│   └── usage.md
├── design/
│   └── architecture.md
├── development/
│   ├── build.md
│   ├── coding_standards.md
│   └── git_commit_guidelines.md
└── _archive/
    ├── contributing.md
    ├── cpp20_modules_migration_plan.md
    └── internal/
        ├── ANNOTATION_PROGRESS_TRACKER.csv
        ├── BUILD_SYSTEM_MODERNIZATION_REPORT.md
        ├── CHANGELOG_TECHNICAL_DEBT.md
        ├── FastQTools_Feature_Expansion_Analysis.md
        ├── annotation_tasks.md
        ├── changelog_code_style.md
        └── 类结构体注释补全任务报告.md
```

## 兼容性与注意事项
- 旧路径 `docs/dev/` 与 `docs/internal/` 已迁移，当前为空；建议删除空目录以避免误导（需维护者确认执行）。
- 历史变更记录中的旧链接不做改写（保持历史真实性）。
- 若外部文档引用了旧路径，请同步更新到新路径。

## 后续工作（建议）
- 在 `docs/design/` 中补充模块级设计与关键数据结构说明。
- 根据需要从 `_archive/` 挑选仍具时效性的材料并整理为活跃文档。
- 视需要新增 `docs/api/` 或在 `docs/README.md` 中标注 API 文档生成方式。
