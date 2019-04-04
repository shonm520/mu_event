#### C语言版muduo，对muduo的部分设计做了优化
#### 关于缓冲池的设计可以参考这篇文章 https://blog.csdn.net/zxm342698145/article/details/88676063

#### 下面是和muduo的部分性能对比测试，测试标准参考陈硕大佬自己的标准 https://blog.csdn.net/solstice/article/details/5864889 


![吞吐量对比](https://github.com/shonm520/mu_event/blob/master/src/testcase/1.png)

#### 连接数为1时，muevent吞吐量没有muduo高，这个以后慢慢找到瓶颈并优化

#### 下图是发送数据量为16384B，不同并发连接下的对比
![并发时吞吐量对比](https://github.com/shonm520/mu_event/blob/master/src/testcase/3.png)

###### 可以看到并发数越大时muevent吞吐量可以和muduo媲美，甚至超越了muduo


#### QQ讨论群：858791125



