#!/bin/bash
set -euo pipefail

usage() {
  echo "Usage:"
  echo "  $0 <version> [message]              # e.g. $0 1.0.2.5 \"Bugfix release\""
  echo "  $0 bump <major|minor|patch> [msg]"
  echo "  $0 -f <release_notes_file>"
  echo "  $0 <version> -f <release_notes_file>"
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

extract_notes_for_version() {
  local file="$1"
  local version="$2"

  if ! [ -f "$file" ]; then
    echo "❌ Error: $file not found." >&2
    return 1
  fi

  local version_pattern="^##[[:space:]]*$version([[:space:]]|$)"
  awk -v pattern="$version_pattern" '
    $0 ~ pattern { in_section=1; next }
    in_section && /^##[[:space:]]*[0-9]+\.[0-9]+\.[0-9]+/ { exit }
    in_section { print }
  ' "$file"
}

extract_latest_notes() {
  local file="$1"
  awk '
    /^##[[:space:]]*[0-9]+\./ && ++section > 1 { exit }
    /^##[[:space:]]*[0-9]+\./ { in_section=1; next }
    in_section { print }
  ' "$file"
}

# --- Parse arguments ---
if [ $# -lt 1 ]; then
  usage
fi

VERSION=""
MESSAGE=""
FILE=""

case "$1" in
  bump)
    [ $# -lt 2 ] && usage
    LATEST=$(get_latest_tag)
    [ -z "$LATEST" ] && LATEST="0.0.0.0"
    VERSION=$(bump_version "$LATEST" "$2")
    shift 2
    MESSAGE="$*"
    ;;
  -f)
    [ $# -lt 2 ] && usage
    FILE="$2"
    LATEST=$(get_latest_tag)
    [ -z "$LATEST" ] && LATEST="0.0.0.0"
    VERSION=$(bump_version "$LATEST" "patch")
    shift 2
    ;;
  *)
    VERSION="$1"
    shift 1
    if [ "${1:-}" = "-f" ]; then
      FILE="$2"
      shift 2
    else
      MESSAGE="$*"
    fi
    ;;
esac

# --- Determine message ---
if [ -n "$FILE" ]; then
  if [ -n "$VERSION" ]; then
    MESSAGE=$(extract_notes_for_version "$FILE" "$VERSION" || true)
  fi
  if [ -z "$MESSAGE" ]; then
    MESSAGE=$(extract_latest_notes "$FILE" || true)
  fi
fi

# If no message provided, default to last commit message
if [ -z "$MESSAGE" ]; then
  MESSAGE=$(git log -1 --pretty=%B)
fi

# Convert literal \n to real newlines
MESSAGE="${MESSAGE//\\n/$'\n'}"
TAG="release-$VERSION"

# Delete local tag if it exists
if git rev-parse "$TAG" >/dev/null 2>&1; then
  echo "Deleting existing local tag $TAG..."
  git tag -d "$TAG"
fi

# Delete remote tag if it exists
if git ls-remote --tags origin | grep -q "refs/tags/$TAG"; then
  echo "Deleting remote tag $TAG..."
  git push --delete origin "$TAG"
fi

# Create and push new annotated tag
echo "Creating annotated tag $TAG with message:"
echo "-----------------------------------------"
echo "$MESSAGE"
echo "-----------------------------------------"

git tag -a "$TAG" -m "$MESSAGE"
git push origin "$TAG"

echo "✅ Done — created $TAG"
