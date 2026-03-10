#!/usr/bin/env python3

from pathlib import Path
import re


ROOT = Path(__file__).resolve().parent.parent
DOCS = ROOT / "docs"
WIKI = DOCS / "residualanalysis.wiki"

SOURCE_FILES = {
    "WikiHome.md": "residualanalysis.md",
    "L2Residual.md": "L2Residual.md",
    "JER.md": "JER.md",
    "L3Residual.md": "L3Residual.md",
    "Systematics.md": "Systematics.md",
    "Batch.md": "Batch.md",
}


def rewrite_for_wiki(text: str) -> str:
    def replace_link(match: re.Match[str]) -> str:
        label = match.group(1)
        target = match.group(2)
        if target.startswith("http://") or target.startswith("https://"):
            return match.group(0)
        if target.endswith(".md") or ".md#" in target:
            return match.group(0)
        if target.startswith("../"):
            return f"[{label}](../-/blob/HEAD/{target[3:]})"
        return match.group(0)

    text = re.sub(r"\[([^\]]+)\]\(([^)]+)\)", replace_link, text)
    return "<!-- Generated from docs/. Edit docs/*.md and run docs/sync_wiki.py. -->\n\n" + text


def main() -> None:
    for src_name, dst_name in SOURCE_FILES.items():
        src = DOCS / src_name
        dst = WIKI / dst_name
        dst.write_text(rewrite_for_wiki(src.read_text()))


if __name__ == "__main__":
    main()