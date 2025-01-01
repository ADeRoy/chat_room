#!/bin/bash
# build.sh

# 读取当前版本号
VERSION=$(cat version.txt 2>/dev/null || echo "0.0.0")

# 增加版本号
MAJOR=$(echo $VERSION | cut -d. -f1)
MINOR=$(echo $VERSION | cut -d. -f2)
PATCH=$(echo $VERSION | cut -d. -f3)
PATCH=$((PATCH + 1))
NEW_VERSION="$MAJOR.$MINOR.$PATCH"

# 如果存在旧版本，先记录旧版本号，以便后续删除
OLD_VERSION=$VERSION

# 构建新版本
docker build -t chatroom:$NEW_VERSION .
docker tag chatroom:$NEW_VERSION chatroom:latest

# 保存新版本号
echo $NEW_VERSION > version.txt

# 如果旧版本存在且不是 0.0.0，则删除旧版本镜像
if [ "$OLD_VERSION" != "0.0.0" ]; then
    docker rmi chatroom:$OLD_VERSION || true
fi

echo "Built version $NEW_VERSION"