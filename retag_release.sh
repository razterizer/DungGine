#!/bin/bash
set -euo pipefail

usage() {
  echo "Usage:"
  echo "  $0 <version> [message]          # e.g. $0 1.0.2.5 \"Bugfix release\""
  echo "  $0 bump <major|minor|patch> [message]"
  exit 1
}

get_latest_tag() {
  git fetch --tags >/dev/null 2>&1
  git tag | grep '^release-' | sed 's/release-//' | sort -V | tail -n 1
}

bump_version() {
  local current="$1"
  local part="$2"

  IFS='.' read -r major minor patch build <<<"$current"

  case "$part" in
    major) ((major++)); minor=0; patch=0 ;;
    minor) ((minor++)); patch=0 ;;
    patch) ((patch++)) ;;
    *) echo "Unknown bump type: $part"; exit 1 ;;
  esac

  ((build++))  # always increment build number

  echo "$major.$minor.$patch.$build"
}

if [ $# -lt 1 ]; then
  usage
fi

if [ "$1" = "bump" ]; then
  if [ $# -lt 2 ]; then
    usage
  fi
  LATEST=$(get_latest_tag)
  if [ -z "$LATEST" ]; then
    echo "No existing release tags found. Starting from 0.0.0.0"
    LATEST="0.0.0.0"
  fi
  VERSION=$(bump_version "$LATEST" "$2")
  shift 2
else
  VERSION="$1"
  shift 1
fi

# Remaining args are the message
MESSAGE="$*"

# If no message provided, default to last commit message
if [ -z "$MESSAGE" ]; then
  MESSAGE=$(git log -1 --pretty=%B)
fi

# Convert literal \n to real newlines
MESSAGE="${MESSAGE//\\n/$'\n'}"

TAG="release-$VERSION"

# Delete local tag if it exists
if git rev-parse "$TAG" >/dev/null 2>&1; then
  echo "Deleting local tag $TAG..."
  git tag -d "$TAG"
fi

# Delete remote tag if it exists
if git ls-remote --tags origin | grep -q "refs/tags/$TAG"; then
  git push --delete origin "$TAG"
fi

# Create and push new annotated tag
echo "Creating annotated tag $TAG with message:"
echo "$MESSAGE"
git tag -a "$TAG" -m "$MESSAGE"
git push origin "$TAG"

echo "✅ Done — created $TAG"
