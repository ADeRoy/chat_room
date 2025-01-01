# 使用Ubuntu 20.04作为基础镜像
FROM ubuntu:20.04

# 避免在构建过程中出现地理区域配置提示
ENV DEBIAN_FRONTEND=noninteractive

# 安装必要的编译工具和依赖
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    make \
    bash \
    curl \
    vim \
    procps \
    && rm -rf /var/lib/apt/lists/*

# 创建工作目录
RUN mkdir -p /opt/chatroom
# 复制源代码和CMake文件到容器中
COPY . /opt/chatroom/
# 创建构建目录并编译
WORKDIR /opt/chatroom/

# 暴露服务端口（根据你的默认端口配置）
EXPOSE 8000

# 运行服务
CMD ["bash", "-c", "./server"]