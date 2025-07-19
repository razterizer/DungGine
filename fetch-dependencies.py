#!/usr/bin/env python3

import contextlib
import dataclasses
import functools as fn
from pathlib import Path
import os
import subprocess as sp
import sys
from typing import Iterator, NamedTuple
import textwrap

####################################################################################################
#                                   HELPER CLASSES AND FUNCTIONS
#
#                      \/ \/ \/ scroll down for start of the script \/ \/ \/
####################################################################################################

class Colors:
    """A few ANSI color codes that are used in logging."""
    reset = "\033[0m"
    bold = "\033[1m"
    blue = "\033[34m"


# We add extra logs if the `FETCH_DEPENDENCIES_DEBUG` environment variable
# is set to 1
debug_enabled=os.environ.get('FETCH_DEPENDENCIES_DEBUG', '0') == '1'


def debug(*args):
    """Only print `*args` in debug mode."""
    if debug_enabled:
        print(f"{Colors.blue}>>", *args, file=sys.stderr, end=f"{Colors.reset}\n")


def info(*args):
    """Print a message that needs to stand out in bold."""
    print(f"{Colors.bold}::", *args, end=f"{Colors.reset}\n")


@contextlib.contextmanager
def pushd(path: Path):
    """Context manager that changes the current directory to `path` and changes it back to the
    previous directory when going out of scope."""
    old_cwd = Path.cwd()
    os.chdir(path)
    debug('PUSHD', path)
    try:
        yield
    finally:
        os.chdir(old_cwd)
        debug('POPD')


def run(command: list, **kwargs) -> sp.CompletedProcess:
    """Wrapper around `subprocess.run` that logs the command that will be executed in debug
    mode."""
    debug('RUN', command)
    return sp.run(command, **kwargs)


# Convenience function for commands that are run internally by this script
# (and whose output should be not shown to the user)
run_internal = fn.partial(run, capture_output=True, text=True)


def short_commit_info(commit_ish: str):
    """Returns a one-line informtation string on `commit_ish`."""
    result = run_internal(['git', 'show', '--date=human', '--no-patch',
                           '--format=format:%cd [%cN] %s', commit_ish])
    return result.stdout.strip()


def yes_no_question(*args):
    response = input(' '.join([*args, '[y/N] ']))
    return response in ['y', 'Y']


# The following classes are just helper classes to transfer data beteween the different steps of
# script.

@dataclasses.dataclass
class Dependency:
    path: Path
    repo: str
    ref: str
    os: str


class CheckoutStatus(NamedTuple):
    path: Path
    commit: str
    should_fetch: bool


def parse_dependencies(f) -> Iterator[Dependency]:
    """Parses a set of dependency lines. Lines starting with `#` are ignored."""
    for i, d in enumerate(map(str.split, f), 1):
        match d:
            case [] | ['#', *_]:
                pass
            case [path, repo, ref, *os]:
                os = os[0] if os else "any"
                yield Dependency(Path(path), repo, ref, os)
            case _:
                print(f'Error in dependencies file. I do not understand line {i}.', file=sys.stderr)
                sys.exit(1)


####################################################################################################
#                                         SCRIPT STARTS HERE
####################################################################################################

if __name__ == '__main__':
    # We will probably have to switch to argparse at some point, but this does the job for now.
    force_yes = False
    
    args = sys.argv[1:]  # Exclude the script name

    # Check for '-y' flag in the arguments
    if '-y' in args:
        force_yes = True
        args.remove('-y')

    # Check for help options
    if set(args) & {'-h', '--help', '-help', '/?'}:
        print(f"usage: {sys.argv[0]} [-y] <dependencies-file>")
        print(textwrap.dedent("""
            Downloads dependencies listed in <dependencies-file>. The paths listed in the
            dependencies file are relative to the current working directory.
            Options:
              -y    Automatically answer 'yes' to prompts.
        """))
        sys.exit(0)

    # Ensure exactly one positional argument remains (the dependencies file)
    if len(args) != 1:
        print(f"usage: {sys.argv[0]} [-y] <dependencies-file>", file=sys.stderr)
        sys.exit(2)

    deps_spec = args[0]
    deps_spec_path = Path(deps_spec)

    
    # Use current working directory as the root for relative paths in the dependencies
    projects_dir = Path.cwd()

    with open(deps_spec_path) as f:
        deps = list(parse_dependencies(f))

    # All the dependencies are checked and added to the following lists depending on what to do with
    # them.
    clone = []
    checkout = []
    do_nothing = []
    error = []

    for d in deps:
        skip = False;
        # os = 'nt' | 'posix' | 'any'
        match d.os:
            case 'win':
                if not os.name == 'nt':
                    skip = True;
            case 'posix':
                if not os.name == 'posix':
                    skip = True;
        if skip:
          continue;
    
        dep_wc = (projects_dir / d.path).absolute()

        # If the dependency working copy does not exist yet, we can just clone it.
        if not dep_wc.exists():
            clone.append(d)
            continue

        # If a file with the same path as the working copy already exists, check if it is the root
        # of a git working copy. If something does not look right, do not try anything that we
        # might regret.

        if not dep_wc.is_dir():
            error.append((d.path, 'This path exists but is not a directory.'))
            continue

        with pushd(dep_wc):
            rev_parse_result = run_internal(['git', 'rev-parse', '--show-toplevel'])
            if rev_parse_result.returncode != 0:
                error.append((d.path, 'git rev-parse failed. Is this a git working copy?'))
                continue

        if not dep_wc.samefile(Path(rev_parse_result.stdout.strip())):
            error.append((d.path, 'This path is inside a subdirectory of an existing git working copy.'))
            continue
        
        # If the working copy looks like a directory, we will try and checkout the right revision,
        # and fetch commits from the default remote if needed.
        with pushd(dep_wc):
            rev_parse_result = run_internal(['git', 'rev-parse', 'HEAD'])
            if rev_parse_result.stdout.strip() == d.ref:
                do_nothing.append(d.path)
                continue

            status_result = run_internal(['git', 'status', '--porcelain=v1'], check=True)
            if status_result.stdout:
                error.append((d.path, 'This git working copy is not clean. I will not switch it.'))
            else:
                cat_file_result = run_internal(['git', 'cat-file', '-e', f'{d.ref}^0']) 
                checkout.append(CheckoutStatus(path=d.path,
                                               commit=d.ref,
                                               should_fetch=(cat_file_result.returncode != 0)))

    # Describe the changes that will be made, and ask if the script should proceed.

    if clone:
        info("I will clone these dependencies:")
        print()
        for d in clone:
            print(f"     {d.path} - {d.repo} ({d.ref})")
        print()

    if checkout:
        info("These dependencies are already cloned but I will switch their working copy:")
        print()
        for path, ref, fetch in checkout:
            with pushd(path):
                # [2024-04-27 thibaut] yes, the formatting is intentional...
                print(    f"    {path} - {ref}")
                if fetch:
                    print( "        [commit not found locally]")
                else:
                    print(f"        {short_commit_info('HEAD')}")
                    print(f"          -> {short_commit_info(ref)}")
            print()

    if error:
        info("These dependencies confuse me. I will not do anything:")
        print()
        for path, err_msg in error:
            print(f"     {path} - {err_msg}")
        print()

    if do_nothing:
        info("These dependencies are already checked out at the right revision:")
        print()
        for path in do_nothing:
            print(f"     {path}")
        print()
    

    if not(clone or checkout):
        # nothing to do, exit
        sys.exit(0)

    if not force_yes and not yes_no_question('Should I continue?'):
        sys.exit(0)
    print()

    # Do the actual work.

    for d in clone:
        info('Dependency', f"'{d.path}'", '[clone]')
        run(['git', 'clone', '--recurse-submodules', d.repo, d.path])

        with pushd(d.path):
            run(['git', 'checkout', '--recurse-submodules', '--detach', d.ref])
        print()

    for path, ref, fetch in checkout:
        info('Dependency', f"'{d.path}'", '[switch]')
        with pushd(path):
            if fetch:
                run(['git', 'fetch'])
            run(['git', 'checkout', '--recurse-submodules', '--detach', ref])

####################################################################################################
#                                        SCRIPT ENDS HERE
#
#                                  (in case you did not notice)
####################################################################################################

