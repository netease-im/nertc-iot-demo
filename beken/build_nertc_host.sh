#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDK_ROOT="${SDK_ROOT:-$ROOT_DIR/bk_aidk}"
PROJECT_DIR="${PROJECT_DIR:-$ROOT_DIR/nertc_host}"
SOC="${SOC:-bk7258}"
JOBS="${JOBS:-24}"
IMAGE="${IMAGE:-bekencorp/armino-idk:1.2}"
USER_ID="$(id -u)"
BUILD_DIR_IN_CONTAINER="../../projects/nertc_host/build_codex"

if [[ ! -d "$SDK_ROOT" ]]; then
    echo "SDK root not found: $SDK_ROOT" >&2
    exit 1
fi

if [[ ! -d "$PROJECT_DIR" ]]; then
    echo "Project dir not found: $PROJECT_DIR" >&2
    exit 1
fi

exec docker run --rm \
    -v "$SDK_ROOT:/armino" \
    -v "$PROJECT_DIR:/armino/projects/nertc_host" \
    -w /armino \
    -u "$USER_ID" \
    "$IMAGE" \
    bash -lc "make $SOC PROJECT=nertc_host PROJECT_DIR=../../projects/nertc_host BUILD_DIR=$BUILD_DIR_IN_CONTAINER -j $JOBS"
