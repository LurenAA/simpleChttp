读完tcp/ip网络编程后一个简单的HTTP实现

2019/6/9: 更改Transfer-Encoding为chunked，能够边读边传。Content-length不能随意的传个较大的值，浏览器会报错Content-length mismatch。
