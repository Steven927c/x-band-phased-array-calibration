# Contributing

感谢参与。提交改动前请：

1. 先建立 Issue 描述问题、数据来源和预期行为。
2. 从 `main` 创建短生命周期分支。
3. 保持设备协议在适配层，核心算法不得依赖全局命令缓冲区。
4. 新增或修改算法必须补测试，并说明单位、坐标系和无效值语义。
5. 不提交真实雷达数据、现场参数、内部协议或任何凭据。
6. 运行构建、测试和格式检查后再发 Pull Request。

提交信息推荐采用 Conventional Commits，例如：

```text
feat: add configurable RHI peak association tolerance
fix: unwrap azimuth samples around north
docs: clarify servo correction sign
```

提交 Pull Request 即表示你有权贡献代码，并同意贡献内容按本仓库 MIT 许可证发布。

