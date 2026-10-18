# 发布到 GitHub 的完整流程

以下命令在本仓库根目录执行，GitHub 用户名为 `Steven927c`，仓库名为 `x-band-phased-array-calibration`。

## 一、发布前确认

1. 阅读 `OPEN_SOURCE_CHECKLIST.md`，确认代码归属和单位授权。
2. 修改 `LICENSE` 中的版权主体。
3. 确认 `README.md` 徽章指向 `Steven927c/x-band-phased-array-calibration`。
4. 再次确认仓库中没有现场数据、参数文件、IP、账号、密钥、内部协议或第三方未授权源码。
5. 本地完成构建和测试。

```bash
cmake -S . -B build -DRCT_BUILD_DEMO=ON -DRCT_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build --output-on-failure -C Release
```

## 二、建立本地 Git 历史

```bash
git init
git branch -M main
git add .
git status
git diff --cached --check
git commit -m "feat: publish X-band phased-array calibration 1.0.0"
```

提交前务必阅读 `git status` 和暂存差异，不要盲目推送。

## 三、在 GitHub 创建仓库

网页方式：

1. 登录 GitHub，点击右上角 `+` → `New repository`。
2. 仓库名填写 `x-band-phased-array-calibration`。
3. 选择 `Public`。
4. 不勾选自动生成 README、LICENSE 和 `.gitignore`，因为本地已经存在。
5. 创建后复制仓库 HTTPS 或 SSH 地址。

命令行方式（已安装并登录 GitHub CLI 时）：

```bash
gh auth status
gh repo create x-band-phased-array-calibration --public --source . --remote origin --push \
  --description "X波段相控阵金属球及太阳法标定"
```

网页创建后，用普通 Git 推送：

```bash
git remote add origin https://github.com/Steven927c/x-band-phased-array-calibration.git
git push -u origin main
```

## 四、仓库设置

在 GitHub 仓库的 `Settings` 中建议完成：

- `General`：启用 Issues；按需启用 Discussions。
- `Branches` 或 `Rules`：为 `main` 建规则，要求 pull request 和 CI 通过。
- `Actions`：确认首次 `build.yml` 执行成功。
- `Security`：启用 Dependabot alerts、secret scanning（账号/组织支持时）。
- `About`：填写说明、网站和 topics，例如 `radar`、`calibration`、`qt`、`cpp`、`meteorology`。

## 五、发布 1.0.0

```bash
git tag -a v1.0.0 -m "X波段相控阵金属球及太阳法标定 1.0.0"
git push origin v1.0.0
```

然后在 GitHub 的 `Releases` → `Draft a new release`：

1. 选择 `v1.0.0`；
2. 标题填写 `X波段相控阵金属球及太阳法标定 1.0.0`；
3. 复制 `CHANGELOG.md` 的 1.0.0 内容；
4. 发布 Release。

## 六、后续维护

- 功能分支 → Pull Request → CI → Review → 合并。
- 版本遵循语义化版本：不兼容 API 为主版本，新功能为次版本，修复为补丁版本。
- 每次发布更新 `CHANGELOG.md`。
- 安全问题按 `SECURITY.md` 私下处理，不要求报告者公开漏洞细节。

