# chat_server简介
基于muduo库实现的nginx负载均衡环境集群聊天服务器及客户端
技术栈:muduo,json,mysql,redis,nginx,cmake

# 编译方式
1.自动编译,进入build目录运行编译脚本
2.手动编译,进入build目录,清空编译文件,使用cmake编译文件
  cd build
  rm -rf *
  cmake ..&&make

# 注意!!!
该项目需要用户自行修改mysql,redis用户信息以使用自己的数据库,nginx需要自行配置
需要修改的地方:/src/server/redis/redis.cpp -56
              /src/server/mysql.cpp -7,8
              

