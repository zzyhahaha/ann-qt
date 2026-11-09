"""把 修改记录.md 转成同目录下的 修改记录.html（本地打开即可，图片用相对路径）。

用法：python docs/build_html.py
依赖：markdown-it-py、mdit-py-plugins（Anaconda 自带）。用它而不是 python-markdown，
是因为记录里有嵌在列表项里的表格和代码块（CommonMark 的 2 格缩进），python-markdown 只认 4 格。
"""
import html
import pathlib
import re

from markdown_it import MarkdownIt
from mdit_py_plugins.anchors import anchors_plugin

HERE = pathlib.Path(__file__).resolve().parent
SRC = HERE / "修改记录.md"
OUT = HERE / "修改记录.html"

CSS = """
:root { --fg:#1f2328; --muted:#59636e; --bg:#ffffff; --line:#d1d9e0; --soft:#f6f8fa; --accent:#0969da; }
@media (prefers-color-scheme: dark) {
  :root { --fg:#e6edf3; --muted:#9198a1; --bg:#0d1117; --line:#3d444d; --soft:#151b23; --accent:#4493f8; }
}
* { box-sizing: border-box; }
body { margin: 0; background: var(--bg); color: var(--fg);
       font: 15px/1.75 "Microsoft YaHei UI", "Microsoft YaHei", "PingFang SC", system-ui, sans-serif; }
main { max-width: 1060px; margin: 0 auto; padding: 24px 16px 80px; }
h1 { font-size: 28px; margin: 8px 0 12px; }
h2 { margin-top: 2.4em; padding-bottom: .3em; border-bottom: 1px solid var(--line); }
h3 { margin-top: 2em; }
h4 { margin-top: 1.6em; font-size: 16px; }
a { color: var(--accent); }
hr { border: 0; border-top: 1px solid var(--line); margin: 2em 0; }
table { border-collapse: collapse; margin: 12px 0; display: block; overflow-x: auto; max-width: 100%; }
th, td { border: 1px solid var(--line); padding: 6px 10px; vertical-align: top; }
th { background: var(--soft); text-align: left; }
table.shots { display: table; table-layout: fixed; width: 100%; }
table.shots td img { width: 100%; }
img { max-width: 100%; height: auto; border: 1px solid var(--line); border-radius: 4px; vertical-align: top; }
code { background: var(--soft); padding: .1em .35em; border-radius: 4px;
       font-family: Consolas, "Cascadia Mono", "Microsoft YaHei UI", "Microsoft YaHei", monospace; font-size: 90%; }
a.zoom { cursor: zoom-in; }
pre { background: var(--soft); padding: 12px 14px; border-radius: 6px; overflow-x: auto; line-height: 1.5; }
pre code { padding: 0; background: none; font-size: 13px; }
nav.toc { background: var(--soft); border: 1px solid var(--line); border-radius: 6px; padding: 10px 18px; margin: 16px 0 8px; }
nav.toc b { color: var(--muted); font-weight: normal; }
nav.toc ol { margin: 4px 0; padding-left: 1.4em; columns: 2; column-gap: 32px; }
@media (max-width: 640px) { nav.toc ol { columns: 1; } }
"""


def main() -> None:
    md = MarkdownIt("commonmark", {"html": False}).enable("table")
    anchors_plugin(md, min_level=2, max_level=3)
    tokens = md.parse(SRC.read_text(encoding="utf-8"))

    # 含截图的表格加 class="shots"：固定列宽、图片按列宽缩放
    table_open = None
    for tok in tokens:
        if tok.type == "table_open":
            table_open = tok
        elif tok.type == "table_close":
            table_open = None
        elif table_open is not None and tok.type == "inline" and any(c.type == "image" for c in tok.children or []):
            table_open.attrSet("class", "shots")

    # 目录：所有二级标题
    toc = []
    for i, tok in enumerate(tokens):
        if tok.type == "heading_open" and tok.tag == "h2":
            toc.append((tok.attrGet("id"), tokens[i + 1].content))
    toc_html = "".join(f'<li><a href="#{html.escape(i)}">{html.escape(t)}</a></li>' for i, t in toc)

    body = md.renderer.render(tokens, md.options, {})
    # 截图可点击，在新标签页里看原图（表格里缩小后字比较小）
    body = re.sub(r'(<img src="([^"]+)"[^>]*>)', r'<a class="zoom" href="\2" target="_blank">\1</a>', body)
    # 目录放在第一个 <hr> 之前（标题和基本信息之后）
    body = body.replace("<hr />", f'<nav class="toc"><b>目录</b><ol>{toc_html}</ol></nav>\n<hr />', 1)

    page = (
        "<!doctype html>\n<html lang=\"zh-CN\">\n<head>\n<meta charset=\"utf-8\">\n"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
        "<title>ANN 修改记录</title>\n<style>" + CSS + "</style>\n</head>\n<body>\n<main>\n"
        + body + "</main>\n</body>\n</html>\n"
    )
    OUT.write_text(page, encoding="utf-8")
    print(f"wrote {OUT} ({len(page)} chars, {len(toc)} sections)")


if __name__ == "__main__":
    main()
