#### C语言版muduo，对muduo的部分设计做了优化
#### 之前用buffer队列实现的缓冲区，效果不是很好，后来使用了环形缓冲区

#### 下面是和muduo的部分性能对比测试，测试标准参考陈硕大佬自己的标准 https://blog.csdn.net/solstice/article/details/5864889 


#### 下面是各数据时的吞吐量对比
![吞吐量对比1](https://upload-images.jianshu.io/upload_images/9444378-b2ae69351584481a.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)


#### 通过使用环形缓冲区，使得吞吐量大大提高，并超过了muduo。我想除了学习到了muduo的精华，还因为目前我的程序结构比较简单，没有其他杂项的缘故

#### 下图是发送数据量为16384B，不同并发连接下的对比
![并发时吞吐量对比2](https://upload-images.jianshu.io/upload_images/9444378-63e04366068867a7.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

###### 可以看到多连接时（高并发时）muevent吞吐量也超过了muduo


#### QQ讨论群：858791125
