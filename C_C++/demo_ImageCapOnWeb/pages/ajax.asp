<%@ LANGUAGE=VBScript CodePage=65001%> 
<%
private function decodeBase64(base64)  
   dim DM, EL  
   Set DM = CreateObject("Microsoft.XMLDOM")  
   Set EL = DM.createElement("tmp")  
   EL.DataType = "bin.base64" 
   EL.Text = base64  
   decodeBase64 = EL.NodeTypedValue  
end function 


dim data
dim str=request.Form("picData")
dim fileFormat=request.Form("picExt")
str=left(1,len(str)-1)
str=right(1,len(str)-1)
data=decodeBase64(str)


set stm=Server.CreateObject("ADODB.Stream")
'stm.Mode=adSaveCreateOverWrite 'adModeReadWrite
stm.type=1 
stm.open
stm.Write(data) 
stm.SaveToFile Server.MapPath("./") & "/" & "ajax" & fileFormat,2 
stm.Close 
set stm=nothing 
response.setContentType("application/x-json")
response.setCharacterEncoding("utf-8")
response.write "{'savestatus':'ok'}"
response.end

%>