# spdlog日志

spdlog作为系统的高性能日志库，支持按日期、大小切分，支持同步异步输出等，是一个功能强大、高性能的跨平台库。

> 注意：这里spdlog是不需要编译的，直接添加到cmake包含include文件即可。

（1）windows安装

windows和linux都不要编译

（2）linux安装

```shell
git clone https://github.com/gabime/spdlog.git
# 建议使用spdlog-1.1.0版本，否则编译不通过
unzip spdlog-1.1.0.zip
cd spdlog-1.1.0 && mkdir build && cd build
cmake .. 
make -j && make install
```

centos报错：error: ‘put_time’ is not a member of ‘std’

问题原因：g++编译器版本低，升级高版本就行了（5.0以上）

解决方式：

```shell
yum -y install gcc tcl
yum install centos-release-scl-rh
yum install centos-release-scl
#yum install rh-release
yum install scl-utils


#显示: No package devtoolset-4-gcc-c++ available
#显示: No package devtoolset-4-gcc available.
wget https://copr.fedoraproject.org/coprs/hhorak/devtoolset-4-rebuild-bootstrap/repo/epel-7/hhorak-devtoolset-4-rebuild-bootstrap-epel-7.repo -O /etc/yum.repos.d/devtools-4.repo
yum install devtoolset-4-gcc devtoolset-4-binutils devtoolset-4-gcc-c++

scl enable devtoolset-4 bash
#报错:Unable to open /etc/scl/conf/devtoolset-4!
#rpm -q gcc
#gcc-4.8.5-44.el7.x86_64
#rpm -e gcc-4.8.5-44.el7.x86_64
#sudo yum install devtoolset-7-gcc*
#scl enable devtoolset-7 bash
echo "source /opt/rh/devtoolset-4/enable" >> ~/.bashrc
```

spdlog作为系统的高性能日志库，支持按日期、大小切分，支持同步异步输出等，是一个功能强大、高性能的跨平台库。

> 注意：这里spdlog是不需要编译的，直接添加到cmake包含include文件即可。

（1）windows安装

windows和linux都不要编译

（2）linux安装

```shell
git clone https://github.com/gabime/spdlog.git
# 建议使用spdlog-1.1.0版本，否则编译不通过
unzip spdlog-1.1.0.zip
cd spdlog-1.1.0 && mkdir build && cd build
cmake .. 
make -j && make install
```

centos报错：error: ‘put_time’ is not a member of ‘std’

问题原因：g++编译器版本低，升级高版本就行了（5.0以上）

解决方式：

```shell
yum -y install gcc tcl
yum install centos-release-scl-rh
yum install centos-release-scl
#yum install rh-release
yum install scl-utils


#显示: No package devtoolset-4-gcc-c++ available
#显示: No package devtoolset-4-gcc available.
wget https://copr.fedoraproject.org/coprs/hhorak/devtoolset-4-rebuild-bootstrap/repo/epel-7/hhorak-devtoolset-4-rebuild-bootstrap-epel-7.repo -O /etc/yum.repos.d/devtools-4.repo
yum install devtoolset-4-gcc devtoolset-4-binutils devtoolset-4-gcc-c++

scl enable devtoolset-4 bash
#报错:Unable to open /etc/scl/conf/devtoolset-4!
#rpm -q gcc
#gcc-4.8.5-44.el7.x86_64
#rpm -e gcc-4.8.5-44.el7.x86_64
#sudo yum install devtoolset-7-gcc*
#scl enable devtoolset-7 bash
echo "source /opt/rh/devtoolset-4/enable" >> ~/.bashrc
```