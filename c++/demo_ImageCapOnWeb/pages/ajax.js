$(function() {
			$('#btnUpload1').click(function() {
				ajax_post_1();
			})
});

function getServerUrl(){
  if ($('#asp').attr("checked")==true) {
	  return "./ajax.asp";
  }else if ($('#jsp').attr("checked")==true) {
	  return "http://localhost:8080/pages/ajax.jsp";
  }else if ($('#php').attr("checked")==true) {
 	  return "./ajax.php";
 }else if ($('#aspnet').attr("checked")==true) {
	 return "./ajax.aspx";
  } else{
      alert('请选择服务器端技术类型!');
	  return null;
   }
}


function ajax_post_1() {
	var base64_data = document.getElementById('cap1').jpegBase64Data;
	alert("data length:"+base64_data.length);
//	var s=getServerUrl();
//	alert(s);
	$.ajax({
				url : getServerUrl(),
				type : 'POST',
				dataType : 'jason',
				data : {
					picData : "'" + base64_data + "'",
					picExt:".jpg"
				},
				timeout : 1000,
				success : callbackfun1
			});
}


function callbackfun1(data) {
	var obj = eval('(' + data + ')');

	if ('ok' == obj.savestatus) {
		alert('success!');
	}

}