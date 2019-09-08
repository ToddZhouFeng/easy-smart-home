# easy-smart-home 轻智居
 This project aims to provide an easy solution for setting up  smart home in a affordable price.

该项目致力于提供便宜且易于设置的智能家居解决方案。



**Supported Device :**(for now)

**支持设备：**（目前）

* Arduino



# Structure 框架

* **Definition:**

  * **Master**：device that **can send instruction**

    **主机**：**可发送**指令的设备

  * **Slave**：device that **can's send instruction** but can receives instruction  

    **从机**：**不可发送**指令的设备，但可接收指令的设备



* **Single Line 单线：**

（如果显示不了，请前往[toddzhoufeng.github.io](https://toddzhoufeng.github.io)）

单主机单从机：

~~~sequence
Title: One Master One Slave
Master->Slave: Control
Slave-->Furniture: Control
~~~

* **Multiple Line 多线：**

单主机多从机：

```sequence
Title: One Master Multiple Slaves
Master->Slave1: Control
Master->Slave2: Control
Master->Slave3: Control
Slave3-->Furniture: Control
Slave2-->Furniture: Control
Slave1-->Furniture: Control 
```

多主机单从机：

```sequence
Title: Multiple Masters One Slave

participant Master1
participant Master2

Master1->Slave: Control
Master2->Slave: Control
Slave-->Furniture: Control
```

多主机多从机：

```sequence
Title: Multiple Masters Multiple Slave

participant Master1
participant Master2

Master1->Slave1: Control
Master2->Slave1: Control
Master1->Slave2: Control

Master2->Slave2: Control
Slave1-->Furniture: Control
Slave2-->Furniture: Control
```



# IR Protocol 红外编码

红外接收相比于蓝牙和网络的优势在于：

* 价格更便宜：红外接收管的价格为￥0.2/个
* 使用的引脚更少：仅需要1个
* 耗电量更小
* 等等

缺点在于：

* 对指向有一定要求
* 要求设备有开孔
* 无法做到远程操控
* 等等

目前来讲，红外比较适合受到自然条件（光、温）影响的智能设备，比如灯、窗帘、空调等，因为这些设备的位置比较固定，并且对远程操控的需求较小。

对于空调、电视一类的可受红外控制的设备，则采用厂家的编码协议；对于无红外的设备，则需要定义新的编码协议。新协议的目标如下：

1. 尽可能但又有区别地兼容现有的红外编码协议（如 NEC  以及 常见电器设备的红外编码协议）

2. 支持多种信息交换方式，比如单播（unicast）、多播（multicast）、广播（broadcast）
    ![](https://img-blog.csdn.net/20160927224929898 "单播（unicast）") 
    ![](https://img-blog.csdn.net/20160927225038734 "多播（multicast）")

  ![](https://img-blog.csdn.net/20160927224707535 "广播（broadcast）")

3. 尽可能减小信号失真的危害
4. 有一定的加密能力 
5. 易于编程与理解

定义 0、1 编码如图：

![](https://www.sbprojects.net/knowledge/ir/necmodulation.png)

一个完整的编码由以下5部分组成：



## 起始位

9ms 高 4.5ms 低

## 主机地址

8位主机类型码 + 8位地址

## 加密位

16位的加密

## 重复起始位/从机地址/数据

重复起始位：9ms 高 4.5ms 低

从机地址：8位地址 + 8位地址的反码，对于长度超出8位的地址，可以将反码也用为地址

数据：8位数据+8位数据的反码，对于长度超出8位的数据，可以将反码也用为数据



![](https://www.sbprojects.net/knowledge/ir/nectrain.png)

这一部分是为了与NEC协议兼容。

## 终止位

 15ms 高




> Inspired by TCP/IP
>
> 受TCP/IP启发