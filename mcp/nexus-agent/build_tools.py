"""Build and source exploration tools for the nexus-planner MCP agent."""

import subprocess
from fnmatch import fnmatch
from pathlib import Path

from nexus_context import REPO_ROOT


def _validate_path(relative_path: str) -> Path | None:
    """Validate that a path stays within REPO_ROOT. Returns None if invalid."""
    resolved = (REPO_ROOT / relative_path).resolve()
    if not str(resolved).startswith(str(REPO_ROOT.resolve())):
        return None
    return resolved


def read_source_file(relative_path: str) -> str:
    """Read a file in the repository.

    Args:
        relative_path: Path relative to repo root (e.g., 'nexus-whitepaper.md')
    """
    path = _validate_path(relative_path)
    if path is None:
        return f"Error: path '{relative_path}' escapes repository root."
    if not path.exists():
        return f"Error: file '{relative_path}' not found."
    if path.stat().st_size > 1_000_000:
        return f"Error: file too large ({path.stat().st_size} bytes). Limit: 1MB."
    return path.read_text()


def list_source_files(pattern: str = "**/*") -> str:
    """List files matching a glob pattern.

    Args:
        pattern: Glob pattern relative to repo root (e.g., '*.md', 'mcp/**/*')
    """
    base = REPO_ROOT
    matches = []
    for path in base.glob(pattern):
        if path.is_file():
            rel = path.relative_to(base)
            # Skip hidden dirs and venvs
            parts = rel.parts
            if any(p.startswith(".") for p in parts):
                continue
            if ".venv" in parts:
                continue
            if "node_modules" in parts:
                continue
            matches.append(str(rel))

    if not matches:
        return f"No files matched pattern: {pattern}"

    return "\n".join(sorted(matches))


def get_project_structure() -> str:
    """Return the directory tree of the project."""
    lines = []

    def _walk(path: Path, prefix: str = "", max_depth: int = 4):
        if len(prefix.split("/")) > max_depth:
            return
        entries = sorted(path.iterdir(), key=lambda p: (not p.is_dir(), p.name))
        for i, entry in enumerate(entries):
            if entry.name.startswith(".") and entry.name != ".gitignore":
                continue
            if entry.name == ".venv" or entry.name == "node_modules":
                continue
            if entry.name == "__pycache__":
                continue
            is_last = i == len(entries) - 1
            connector = "└── " if is_last else "├── "
            lines.append(f"{prefix}{connector}{entry.name}")
            if entry.is_dir():
                extension = "    " if is_last else "│   "
                _walk(entry, prefix + extension, max_depth)

    lines.append(f"{REPO_ROOT.name}/")
    _walk(REPO_ROOT)
    return "\n".join(lines)


def run_make(target: str = "") -> str:
    """Run a make target in the repo root.

    Args:
        target: Makefile target to run. If empty, runs default target.
    """
    makefile = REPO_ROOT / "Makefile"
    if not makefile.exists():
        return "Error: No Makefile found in repo root."

    args = ["make"]
    if target:
        args.append(target)

    result = subprocess.run(
        args,
        cwd=REPO_ROOT,
        capture_output=True,
        text=True,
    )

    output = []
    if result.stdout:
        output.append(result.stdout)
    if result.stderr:
        output.append(result.stderr)
    if result.returncode != 0:
        output.append(f"\nMake exited with code {result.returncode}")

    return "\n".join(output) or "Make completed with no output."
