"""nexus-planner: MCP agent for the Nexus programming language project.

Run: python3 server.py
Transport: stdio (spawned by MCP client)
"""

from mcp.server.fastmcp import FastMCP

from nexus_context import SYSTEM_PROMPT
from git_tools import (
    git_status,
    git_diff,
    git_diff_staged,
    git_log,
    git_show_commit,
    git_branch_list,
    git_branch_create,
    git_branch_delete,
    git_preview_commit,
    git_commit,
    git_push,
    git_pull,
    git_stash,
    git_stash_pop,
    git_checkout,
)
from build_tools import (
    read_source_file,
    list_source_files,
    get_project_structure,
    run_make,
)

mcp = FastMCP(
    "nexus-planner",
    instructions=SYSTEM_PROMPT,
)

# ── Git Tools ──────────────────────────────────────────────


@mcp.tool()
def git_status_tool() -> str:
    """Get the git status of the Nexus repository."""
    return git_status()


@mcp.tool()
def git_diff_tool() -> str:
    """Get unstaged changes in the repository."""
    return git_diff()


@mcp.tool()
def git_diff_staged_tool() -> str:
    """Get staged changes in the repository."""
    return git_diff_staged()


@mcp.tool()
def git_log_tool(count: int = 10) -> str:
    """Show recent git commits.

    Args:
        count: Number of commits to show (default 10)
    """
    return git_log(count)


@mcp.tool()
def git_show_commit_tool(commit_hash: str) -> str:
    """Show a specific commit and its diff.

    Args:
        commit_hash: The commit hash or ref to show
    """
    return git_show_commit(commit_hash)


@mcp.tool()
def git_branch_list_tool() -> str:
    """List all branches in the repository."""
    return git_branch_list()


@mcp.tool()
def git_branch_create_tool(branch_name: str) -> str:
    """Create a new branch from current HEAD.

    Args:
        branch_name: Name of the new branch
    """
    return git_branch_create(branch_name)


@mcp.tool()
def git_branch_delete_tool(branch_name: str) -> str:
    """Delete a branch.

    Args:
        branch_name: Name of the branch to delete
    """
    return git_branch_delete(branch_name)


@mcp.tool()
def git_preview_commit_tool(message: str) -> str:
    """Preview what will be committed. Shows diff, files, and proposed commit message.

    Call this BEFORE git_commit to confirm what will be committed.
    The user must review this output before proceeding.

    Args:
        message: Commit message (conventional prefix auto-added if missing)
    """
    return git_preview_commit(message)


@mcp.tool()
def git_commit_tool(message: str) -> str:
    """Stage all changes and commit with the agent identity.

    Always call git_preview_commit first to show the user what will be committed.
    Uses conventional commit prefix (feat:, fix:, docs:, chore:) if not provided.

    Args:
        message: Commit message
    """
    return git_commit(message)


@mcp.tool()
def git_push_tool(branch: str, force: bool = False) -> str:
    """Push a branch to origin.

    Blocks pushing to main/master by default for safety.
    Use force=True to override (uses --force-with-lease).

    Args:
        branch: Branch name to push (required)
        force: Allow pushing to protected branches (default False)
    """
    return git_push(branch, force)


@mcp.tool()
def git_pull_tool() -> str:
    """Pull from origin with rebase."""
    return git_pull()


@mcp.tool()
def git_stash_tool(message: str = "") -> str:
    """Stash current changes.

    Args:
        message: Optional stash message
    """
    return git_stash(message)


@mcp.tool()
def git_stash_pop_tool() -> str:
    """Pop the most recent stash."""
    return git_stash_pop()


@mcp.tool()
def git_checkout_tool(branch_name: str) -> str:
    """Switch to a branch.

    Args:
        branch_name: Branch to switch to
    """
    return git_checkout(branch_name)


# ── Build / Source Tools ────────────────────────────────────


@mcp.tool()
def read_source_file_tool(relative_path: str) -> str:
    """Read a file from the repository.

    Args:
        relative_path: Path relative to repo root (e.g., 'nexus-whitepaper.md')
    """
    return read_source_file(relative_path)


@mcp.tool()
def list_source_files_tool(pattern: str = "**/*") -> str:
    """List files matching a glob pattern.

    Args:
        pattern: Glob pattern (e.g., '*.md', 'mcp/**/*', '*.c')
    """
    return list_source_files(pattern)


@mcp.tool()
def get_project_structure_tool() -> str:
    """Get the full directory tree of the project."""
    return get_project_structure()


@mcp.tool()
def run_make_tool(target: str = "") -> str:
    """Run a make target.

    Args:
        target: Makefile target (empty for default)
    """
    return run_make(target)


if __name__ == "__main__":
    mcp.run(transport="stdio")
