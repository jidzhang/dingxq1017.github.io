---
layout: post
title: "创建块和块参照"
category: 
---

在讲解如何创建块/块参照前，先普及下AutoCAD图形数据库的基本知识。

通常我打开或新建的一张dwg图纸就是一个图形数据库（Graphic Database），这是一个实实在在的数据库，由表（Table）和记录（TableRecord）组成。大家用AutoCAD画图，除了画线画实体外，还几乎必须用到图层表、标注样式表和线型表等，这正是在直接操作AutoCAD图形数据库的各种表。

具体来说，AutoCAD图形数据库由 9 个符号表和一个命名字典构成，每个表（table）下面是一条一条的记录（record）。这 9 个表分别是：
	
Lint type table
Dim type table
Layer table
Block table
。。。。。

通常进行的绘图操作，从编程角度来说是这样一个流程：

	打开表 --> 从表里找到记录或创建新的纪录 --> 往记录里面添加实体或者修改里面的实体

今天要做的演示是：选择实体后创建一个命名块，然后利用这个块插入块参照。

程序流程是这样：

1. 选择实体，得到一个选择集（SelectionSet）
2. new 一个块表记录指针（BlockTableRecord * pBlockRecord)
3. 提示用户输入块的名字，并赋给上面的块表记录
4. 把 块表记录 添加到块表（BlockTable）中，同时记下与块表记录关联的 ObjectID
5. 利用上面块表记录的ID（ObjectID），向块表记录中添加选择集中的实体（Entity）
6. 利用块表记录的ID随意创建一个块参照（BlockReference）
7. 提示用户输入块参照的插入位置，随后插入块参照

简化的源程序如下，完整程序请到我们的 [Github 主页](https://github.com/dingxq1017/dingxq1017.github.io/tree/master/ArxProject)上获取。


	

	
	