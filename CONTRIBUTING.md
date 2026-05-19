# 贡献指南 / Contributing Guide

感谢您对 FloraMind 项目的关注！欢迎提交 Issue 和 Pull Request。

## 报告问题 / Report Issues

- 使用 GitHub Issues 报告 bug 或提出功能建议
- 请提供详细的问题描述、复现步骤和环境信息
- 如果涉及硬件问题，请说明使用的开发板型号和接线方式

## 提交代码 / Submit Code

1. Fork 本仓库
2. 创建功能分支: `git checkout -b feature/your-feature`
3. 提交更改: `git commit -m "Add your feature"`
4. 推送分支: `git push origin feature/your-feature`
5. 创建 Pull Request

## 代码规范 / Code Style

- **CubeMX USER CODE 区域**: 所有用户代码必须写在 `/* USER CODE BEGIN */` 和 `/* USER CODE END */` 之间，避免重新生成代码时丢失
- **命名规范**: 变量和函数使用小写字母+下划线 (snake_case)，宏定义使用大写字母+下划线
- **注释**: 关键算法和复杂逻辑需要添加中文注释
- **头文件保护**: 使用 `#ifndef` / `#define` / `#endif` 保护头文件

## 分支命名 / Branch Naming

- `feature/xxx` -- 新功能
- `fix/xxx` -- Bug 修复
- `docs/xxx` -- 文档更新
- `refactor/xxx` -- 代码重构
