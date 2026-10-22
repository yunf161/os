#!/usr/bin/env bash
set -euo pipefail

# 每个实验阶段独立构建；不使用并行 make，避免共享编译命令竞争。
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
stage="${1:-lab8/src/7}"
stage_dir="$(realpath -- "$root/$stage")"
case "$stage_dir" in
  "$root"/lab*) ;;
  *) echo "Stage must be an experiment directory inside this repository" >&2; exit 2 ;;
esac
test -f "$stage_dir/build/makefile" || { echo "Missing build/makefile" >&2; exit 2; }
mkdir -p -- "$stage_dir/run"
# 只在不存在时创建镜像，不截断用户已存在的镜像。
if [[ ! -e "$stage_dir/run/hd.img" ]]; then
  truncate -s 16M -- "$stage_dir/run/hd.img"
fi
make -C "$stage_dir/build"
