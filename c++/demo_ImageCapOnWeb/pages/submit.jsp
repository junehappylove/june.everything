<%@ page contentType="text/html; charset=utf-8" language="java" import="java.sql.*,java.io.*" errorPage="" %>
<%
String savePath=config.getServletContext().getRealPath("/")+"//";

		File tmp_path=new File(savePath);
		tmp_path.mkdirs();
		System.out.println("照片数据保存路径:"+savePath);


String pic_base_64_data=request.getParameter("picData");

//如果下面的代码输出true则说明需要调整服务器软件工作参数，解决接受post数据的大小限制问题,例如
//tomcat的话需要在server.xml中配置maxPostSize="0"来解除上传数据的大小限制   <Connector port="8080" protocol="HTTP/1.1" 
//               connectionTimeout="20000" 
//               redirectPort="8443" maxPostSize="0"/>
// 
System.out.println(null==pic_base_64_data);

System.out.println("base64 string length:"+pic_base_64_data.length());
String fileFormat=request.getParameter("picExt");
sun.misc.BASE64Decoder decode=new sun.misc.BASE64Decoder();

byte[] datas=decode.decodeBuffer(pic_base_64_data);

String filename=String.valueOf(System.currentTimeMillis())+fileFormat;
File file=new File(savePath+filename);
OutputStream fos=new FileOutputStream(file);
System.out.println("图片文件名称:"+filename);
fos.write(datas);
fos.close();

out.print("<a href='" + request.getContextPath()+"/"+ filename + "'>click here</a>");
out.flush();
out.close();
%>