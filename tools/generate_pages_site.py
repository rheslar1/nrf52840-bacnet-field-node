#!/usr/bin/env python3
from __future__ import annotations

import html
import shutil
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SITE = ROOT / "site"


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def first_paragraph(markdown: str) -> str:
    for block in markdown.split("\n\n"):
        stripped = block.strip()
        if stripped and not stripped.startswith("#") and not stripped.startswith("```"):
            return stripped.replace("\n", " ")
    return "nRF52840 BACnet field node project evidence."


def copy_tree(source: Path, destination: Path) -> None:
    if source.exists():
        shutil.copytree(source, destination)


def link_list(paths: list[Path], base: Path) -> str:
    items = []
    for path in paths:
        relative = path.relative_to(base)
        label = relative.as_posix()
        items.append(f'<li><a href="{html.escape(label)}">{html.escape(label)}</a></li>')
    return "\n".join(items)


def diagram_cards() -> str:
    diagrams_dir = ROOT / "docs" / "diagrams"
    cards = []
    for image in sorted(diagrams_dir.glob("*.png")):
        title = image.stem.replace("-", " ").title()
        relative = image.relative_to(ROOT).as_posix()
        drawio = image.with_suffix(".drawio")
        drawio_link = drawio.relative_to(ROOT).as_posix()
        cards.append(
            f"""
            <article class="card">
              <img src="{html.escape(relative)}" alt="{html.escape(title)} diagram">
              <h3>{html.escape(title)}</h3>
              <a href="{html.escape(drawio_link)}">Editable Draw.io source</a>
            </article>
            """
        )
    return "\n".join(cards)


def main() -> None:
    if SITE.exists():
        shutil.rmtree(SITE)
    SITE.mkdir(parents=True)

    copy_tree(ROOT / "docs", SITE / "docs")
    copy_tree(ROOT / "examples", SITE / "examples")

    readme = read_text(ROOT / "README.md")
    summary = first_paragraph(readme)
    docs = sorted((ROOT / "docs").glob("*.md"))
    examples = sorted((ROOT / "examples").glob("*"))

    (SITE / "index.html").write_text(
        f"""<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>nRF52840 BACnet Field Node</title>
  <style>
    :root {{
      color-scheme: dark;
      --bg: #101418;
      --panel: #182027;
      --panel-2: #202a33;
      --text: #edf2f7;
      --muted: #aab7c4;
      --accent: #3fd0a8;
      --line: #34424f;
    }}
    * {{ box-sizing: border-box; }}
    body {{
      margin: 0;
      background: var(--bg);
      color: var(--text);
      font-family: Inter, ui-sans-serif, system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
      line-height: 1.5;
    }}
    header, main {{
      width: min(1120px, calc(100% - 32px));
      margin: 0 auto;
    }}
    header {{
      padding: 48px 0 24px;
      border-bottom: 1px solid var(--line);
    }}
    h1 {{
      margin: 0 0 12px;
      font-size: clamp(2rem, 5vw, 4rem);
      line-height: 1;
    }}
    h2 {{ margin: 0 0 16px; }}
    p {{ color: var(--muted); max-width: 840px; }}
    a {{ color: var(--accent); }}
    main {{ padding: 28px 0 56px; }}
    section {{ margin: 0 0 32px; }}
    .grid {{
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
      gap: 16px;
    }}
    .card {{
      background: var(--panel);
      border: 1px solid var(--line);
      border-radius: 8px;
      padding: 16px;
    }}
    .card img {{
      width: 100%;
      aspect-ratio: 16 / 9;
      object-fit: contain;
      background: var(--panel-2);
      border-radius: 6px;
      border: 1px solid var(--line);
    }}
    .metric {{
      background: var(--panel);
      border: 1px solid var(--line);
      border-radius: 8px;
      padding: 18px;
    }}
    .metric strong {{
      display: block;
      color: var(--accent);
      font-size: 1.6rem;
    }}
    code {{
      background: var(--panel-2);
      border: 1px solid var(--line);
      border-radius: 6px;
      padding: 2px 6px;
    }}
    ul {{
      padding-left: 20px;
      color: var(--muted);
    }}
  </style>
</head>
<body>
  <header>
    <h1>nRF52840 BACnet Field Node</h1>
    <p>{html.escape(summary)}</p>
  </header>
  <main>
    <section class="grid">
      <div class="metric"><strong>C++17</strong>Host-buildable embedded field-node core</div>
      <div class="metric"><strong>CI</strong>CMake, CTest, CLI smoke checks</div>
      <div class="metric"><strong>Static</strong>cppcheck and clang-tidy gates</div>
      <div class="metric"><strong>Deploy</strong>GitHub Pages evidence site</div>
    </section>
    <section>
      <h2>Project Evidence</h2>
      <p>The deployed site packages the architecture notes, commissioning evidence, diagrams, validation plan, and static-analysis workflow used by the repository.</p>
      <ul>
        <li><a href="README.md">Repository README</a></li>
        {link_list(docs, ROOT)}
        {link_list(examples, ROOT)}
      </ul>
    </section>
    <section>
      <h2>Diagrams</h2>
      <div class="grid">
        {diagram_cards()}
      </div>
    </section>
    <section>
      <h2>Automation</h2>
      <p>Pull requests run build, tests, CLI smoke checks, <code>cppcheck</code>, and <code>clang-tidy</code>. Pushes to <code>main</code> deploy this static evidence site after those gates pass.</p>
    </section>
  </main>
</body>
</html>
""",
        encoding="utf-8",
    )

    shutil.copy2(ROOT / "README.md", SITE / "README.md")


if __name__ == "__main__":
    main()
