"""Git tools for the nexus-planner MCP agent.

All operations are scoped to REPO_ROOT.
The agent uses its own git identity: nexus-planner / nexus-planner@nexus.local
"""

import subprocess
from pathlib import Path

from nexus_context import REPO_ROOT

AGENT_NAME = "nexus-planner"
AGENT_EMAIL = "nexus-planner@nexus.local"

BLOCKED_BRANCHES = ("main", "master")


def _git(*args: str, check: bool = True) -> subprocess.CompletedProcess:
    """Run a git command in REPO_ROOT with the agent identity."""
    cmd = [
        "git",
        "-c", f"user.name={AGENT_NAME}",
        "-c", f"user.email={AGENT_EMAIL}",
        *args,
    ]
    return subprocess.run(
        cmd,
        cwd=REPO_ROOT,
        capture_output=True,
        text=True,
        check=check,
    )


def _git_status_files() -> list[str]:
    """Return list of changed files (staged + unstaged + untracked)."""
    result = _git("status", "--porcelain")
    return [
        line[3:] for line in result.stdout.strip().splitlines() if line.strip()
    ]


def _detect_commit_type() -> str:
    """Detect conventional commit prefix based on changed files."""
    files = _git_status_files()
    if not files:
        return "chore"

    has_md = any(f.endswith(".md") for f in files)
    has_mcp = any("mcp/" in f for f in files)
    has_new = any(f.startswith("??") for f in _git("status", "--porcelain").stdout.splitlines() if f.startswith("??"))

    # Check for new files
    status_result = _git("status", "--porcelain")
    has_new = any(line.startswith("??") for line in status_result.stdout.splitlines())

    if has_new:
        return "feat"
    if has_md and not any(not f.endswith(".md") for f in files):
        return "docs"
    if has_mcp:
        return "chore"
    return "chore"


def git_status() -> str:
    """Get the git status of the repository."""
    result = _git("status")
    return result.stdout or "No changes."


def git_diff() -> str:
    """Get the unstaged diff."""
    result = _git("diff")
    return result.stdout or "No unstaged changes."


def git_diff_staged() -> str:
    """Get the staged diff."""
    result = _git("diff", "--staged")
    return result.stdout or "No staged changes."


def git_log(count: int = 10) -> str:
    """Show recent git log."""
    result = _git("log", f"--oneline", f"-{count}", check=False)
    if result.returncode != 0:
        return "No commits yet."
    return result.stdout or "No commits yet."


def git_show_commit(commit_hash: str) -> str:
    """Show a specific commit's diff."""
    result = _git("show", commit_hash, check=False)
    if result.returncode != 0:
        return f"Error: {result.stderr}"
    return result.stdout


def git_branch_list() -> str:
    """List all branches."""
    result = _git("branch", "-a")
    return result.stdout or "No branches."


def git_branch_create(branch_name: str) -> str:
    """Create a new branch from current HEAD."""
    result = _git("branch", branch_name, check=False)
    if result.returncode != 0:
        return f"Error creating branch: {result.stderr}"
    return f"Created branch: {branch_name}"


def git_branch_delete(branch_name: str) -> str:
    """Delete a branch."""
    if branch_name in BLOCKED_BRANCHES:
        return f"BLOCKED: Cannot delete protected branch '{branch_name}'."
    result = _git("branch", "-d", branch_name, check=False)
    if result.returncode != 0:
        return f"Error deleting branch: {result.stderr}"
    return f"Deleted branch: {branch_name}"


def git_preview_commit(message: str) -> str:
    """Show what would be committed for user confirmation.

    Returns the staged diff, file list, and proposed commit message.
    The user must confirm before calling git_commit.
    """
    # First stage tracked changes and new files
    _git("add", "-A")

    # Get staged diff
    diff_result = _git("diff", "--staged", "--stat")
    diff_full = _git("diff", "--staged")

    # Get commit type
    commit_type = _detect_commit_type()

    # Build conventional commit message
    if ":" not in message:
        full_message = f"{commit_type}: {message}"
    else:
        full_message = message

    output = []
    output.append("=== PROPOSED COMMIT ===")
    output.append(f"Commit message: {full_message}")
    output.append("")
    output.append("=== FILES TO BE COMMITTED ===")
    output.append(diff_result.stdout or "No files staged.")
    output.append("")
    output.append("=== DIFF ===")
    output.append(diff_full.stdout or "No diff (binary or empty).")
    output.append("")
    output.append("Call git_commit with this exact message to proceed.")
    return "\n".join(output)


def git_commit(message: str) -> str:
    """Stage all changes and create a commit with the agent identity.

    Uses conventional commit prefix if not provided in message.
    """
    # Stage tracked changes and new files
    _git("add", "-A")

    # Build conventional commit message
    commit_type = _detect_commit_type()
    if ":" not in message:
        full_message = f"{commit_type}: {message}"
    else:
        full_message = message

    result = _git("commit", "-m", full_message, check=False)
    if result.returncode != 0:
        return f"Error committing: {result.stderr}"
    return f"Committed: {full_message}\n{result.stdout}"


def git_push(branch: str, force: bool = False) -> str:
    """Push to origin. Blocks main/master by default.

    Args:
        branch: Branch name to push (required, no default).
        force: If True, allows pushing to main/master. Use with caution.
    """
    if not branch:
        return "Error: branch parameter is required."

    if branch in BLOCKED_BRANCHES and not force:
        return (
            f"BLOCKED: Cannot push to '{branch}' branch.\n"
            f"To override, call git_push with force=True.\n"
            f"This is a safety measure to protect the main branch."
        )

    cmd_args = ["push", "origin", branch]
    if force:
        cmd_args.insert(1, "--force-with-lease")

    result = _git(*cmd_args, check=False)
    if result.returncode != 0:
        return f"Error pushing: {result.stderr}"
    return f"Pushed '{branch}' to origin.\n{result.stdout}"


def git_pull() -> str:
    """Pull from origin with rebase."""
    result = _git("pull", "--rebase", check=False)
    if result.returncode != 0:
        return f"Error pulling: {result.stderr}"
    return result.stdout


def git_stash(message: str = "") -> str:
    """Stash current changes."""
    args = ["stash"]
    if message:
        args.extend(["push", "-m", message])
    result = _git(*args, check=False)
    if result.returncode != 0:
        return f"Error stashing: {result.stderr}"
    return result.stdout or "Changes stashed."


def git_stash_pop() -> str:
    """Pop the most recent stash."""
    result = _git("stash", "pop", check=False)
    if result.returncode != 0:
        return f"Error popping stash: {result.stderr}"
    return result.stdout


def git_checkout(branch_name: str) -> str:
    """Switch to a branch."""
    result = _git("checkout", branch_name, check=False)
    if result.returncode != 0:
        return f"Error checking out: {result.stderr}"
    return f"Switched to branch: {branch_name}\n{result.stdout}"
