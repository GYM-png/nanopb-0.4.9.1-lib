#!/usr/bin/env bash
# =============================================================================
# nanopb 自动生成脚本（仅 Windows / Git Bash）
# 使用 generator-bin-win 目录
# =============================================================================

# 颜色定义（无加粗）
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# 固定路径（仅 Windows）
PROTO_SRC="$SCRIPT_DIR/nanopb_proto"
OUTPUT_DIR="$SCRIPT_DIR/nanopb_gen"
GENERATOR_BIN="$SCRIPT_DIR/generator-bin-win"
NANOPB_PROTO_DIR="$GENERATOR_BIN/proto"

# 检查必要目录和文件
[ ! -d "$GENERATOR_BIN" ] && { echo -e "${RED}错误: 未找到 generator-bin-win 目录: $GENERATOR_BIN${NC}"; exit 1; }
[ ! -d "$PROTO_SRC" ] && { echo -e "${RED}错误: 未找到 nanopb_proto 目录！${NC}"; exit 1; }
[ ! -f "$NANOPB_PROTO_DIR/nanopb.proto" ] && { echo -e "${RED}错误: 未找到 nanopb.proto！路径: $NANOPB_PROTO_DIR/nanopb.proto${NC}"; exit 1; }

mkdir -p "$OUTPUT_DIR"

# 检查 protoc
if ! command -v protoc &> /dev/null; then
    echo -e "${RED}错误: 未找到 protoc 命令，请安装 Protocol Buffers 编译器${NC}"
    echo "    下载: https://github.com/protocolbuffers/protobuf/releases"
    exit 1
fi

# 查找插件（优先 .exe）
PLUGIN_PATH=""
if [ -f "$GENERATOR_BIN/protoc-gen-nanopb.exe" ]; then
    PLUGIN_PATH="$GENERATOR_BIN/protoc-gen-nanopb.exe"
elif [ -f "$GENERATOR_BIN/protoc-gen-nanopb" ]; then
    PLUGIN_PATH="$GENERATOR_BIN/protoc-gen-nanopb"
elif command -v protoc-gen-nanopb &> /dev/null; then
    PLUGIN_PATH="protoc-gen-nanopb"
else
    echo -e "${RED}错误: 未找到 protoc-gen-nanopb 插件！${NC}"
    echo "    期望路径:"
    echo "      $GENERATOR_BIN/protoc-gen-nanopb"
    echo "      $GENERATOR_BIN/protoc-gen-nanopb.exe"
    exit 1
fi

echo -e "${GREEN}开始编译 .proto 文件...${NC}"
echo "  generator-bin: $GENERATOR_BIN"
echo "  输入目录: $PROTO_SRC"
echo "  输出目录: $OUTPUT_DIR"
echo "  插件: $PLUGIN_PATH"
echo ""

# 获取所有 .proto 文件
PROTO_FILES=("$PROTO_SRC"/*.proto)
if [ ${#PROTO_FILES[@]} -eq 0 ] || [ "${PROTO_FILES[0]}" = "$PROTO_SRC/*.proto" ]; then
    echo -e "${YELLOW}警告: nanopb_proto 目录中未找到 .proto 文件！${NC}"
    exit 0
fi

SUCCESS_COUNT=0
FAIL_COUNT=0

for proto in "${PROTO_FILES[@]}"; do
    filename=$(basename "$proto")
    echo -e "${YELLOW}正在编译: $filename${NC}"

    protoc \
        --plugin="protoc-gen-nanopb=$PLUGIN_PATH" \
        --nanopb_out="$OUTPUT_DIR" \
        -I"$PROTO_SRC" \
        -I"$NANOPB_PROTO_DIR" \
        "$proto"

    if [ $? -eq 0 ]; then
        echo -e "${GREEN}成功: $filename → ${filename%.proto}.pb.c/h${NC}"
        ((SUCCESS_COUNT++))
    else
        echo -e "${RED}失败: $filename${NC}"
        ((FAIL_COUNT++))
    fi
done

echo ""
echo -e "${GREEN}编译完成！成功: $SUCCESS_COUNT, 失败: $FAIL_COUNT${NC}"
[ $SUCCESS_COUNT -gt 0 ] && echo "生成文件位于: $OUTPUT_DIR/"

exit $FAIL_COUNT