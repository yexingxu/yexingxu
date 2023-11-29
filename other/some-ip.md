<!--
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-09 13:28:52
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-11-21 03:02:47
-->
1. 明确 client id 的分配问题；同一个 ecu 之间如何分配，不同 ecu 如何分配
hostprefix+processprefix+random

2. session handling 是怎么使用的，什么情况下会是false

3. 对齐是可以进行配置的吗，4 还是 8

4. 00079 的结构体配置项是什么

5. ip options 的使用，发送地址和监听地址怎么分配的？user 的明白了，sd 报文的？

7. Service contract versioning 什么作用？和 service instance version 区别

8. Communication Group 怎么实现？设计两个 service， 但是是两个进程啊，怎么会写成 api 的形式呢？只提供生成代码，实现让 user 去做？
   哪些实现在代码里，哪些实现在 app 里，具体通信流程是怎么样的？

9. Optional Execution Context 在说什么意思？

10. multi-cast-threshold 切换报文使用原来的路径进行通知吗（应该是）？切换的时候会创建 socket 还是之前已有，现在的方案?

11. Static Service Connection 是什么东西，应用场景？不经过服务发现协议进行通信，不发 sd 报文吗？服务怎么下线呢？

12. 怎么实现 data accumulation，类似 dds 的 batch? -- skeleton 和 proxy 负责创建组包器，序列化器要支持 copy 之前获取长度

13. someip 的接受消息的缓存能缓存多少，类似history 的 depth 吗，可以配置？

14. SWS_CM_11270 是说这里的哪些数据是被安全保护的吗？加密？

15. trigger 是一个 event，感觉跟 event 没什么不同啊，单独拿出来意义是什么，场景？

16. someip 回复 message type 的 error 和 error code 的时机。

------------------

17. iam 时权限信息会发给对端还是根据配置内容进行查询？
18. comgount是什么东西？
19. global SMState 是什么？
20. 同一个服务（sid,siid,major version）一样能在一个machine上不同的网口对外提供吗？
一个 machine 上怎么检查重复的服务？查看端口占用？
21. 什么是 optional argument
22. 现在的 error code 和之前版本的定义方式有什么不一样吗
23. service interface mapping 和 service interface element mapping 在讲什么？
24. data type 是怎么进行定义的？
25. SomeipServiceDiscovery 配置内容，ip地址怎么配置的，内容好多啊，IPSecRule，IPSecConfig...
26. figure 11.3 没看懂

//要做的事情：
1 砍掉daemon（发现协议受影响，发送接收受影响），
2 IPC 换共享内存，
3 序列化器支持长度计算（静态反射？TLV）
4 static service，
5 data accumulation，
6 communication group
7 s2s
8

3 5 在 someip 里面。