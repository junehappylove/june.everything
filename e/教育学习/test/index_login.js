/**
 * 
 */
var personIdcard;
var certType;
var password;
var VerifyCode;
var username;
var yhdl;
var facerc;
var imageInfo;
var hasCamera=false;
$(function() {
	if(!("ActiveXObject" in window)){
		//alert("请使用IE浏览器");
		if($("#yourFace").length>0){
			var faceobj='<object id="yourFace" type="application/x-itst-activex" '
				+'	style="border:0px;width:320px;height:240px;" '
				+'	clsid="{6C94E32C-B15B-4A66-9FA8-6C1D013D0D19}" '
				+'	progid="FileOperation.CAB#version=1,0,0,1">'
			$("#camera").html(faceobj);
		}
	};
	//$('#commitNot')[0].reset();
	//$('#image').removeAttr("src");
	$('#facefield').val("1");
	$('#gotoLogin').on('click', function() {
		username= $('#username').val();
		password = $('#password').val();
		personIdcard = $('#PAPERSCODE').val();
		VerifyCode = $('#VerifyCode').val();
		certType = $('#CERTTYPE').val();
		yhdl=$("input[name='yhdl']:checked").val();
		if (!verifyMyInfo()) {
			return;
		}
		
		checkPerson();
	});
	
	
	$('#passGoBack').on('click', function() {
		buildPassBackModel();
	});

	
	$('#btn2').on('click', function() {
		beginGetFace();
	});
	$('#btn3').on('click', function() {
		onstart();
	});
	$('#btn4').on('click', function() {
		$('#comparePhoto').window('close');
	});
});
// 判断用户类型
function judgeUserType(val){
	$.ajax({
		url : "/judgeUserType.xhtml?" + new Date().getTime(),
		data : {'userType' : val},
		type : "GET",
		async : true,
		beforeSend : function() {
		},
		success : function(result) {
			if(result=="error"){
			   alert("您登录的用户类型不能学习此类别的教育内容,请点击其它！");
			}else if(result=="notLogin"){
			   alert("您还没有登录，请登录！");
			}else if(result=="success"){
               window.location.href="/showPlay.xhtml";			   
			}
		}
	});
}



//验证人员
function checkPerson(){
	$.ajax({
		url : "/isFaceLogin.xhtml?" + new Date().getTime(),
		data : {
			'PAPERSCODE' : personIdcard,
			'USERNAME' : username,
			'yhdl':yhdl,
			'CERTTYPE' : certType,
			'PASSWORD' : password,
			'rand' : VerifyCode
		},
		type : "POST",
		async : true,
		beforeSend : function() { 
			$("#gotoLogin").text("正在提交数据请稍候......").show();
		},
		success : function(result) {
			$("#gotoLogin").text("登录").show();
			if(result.error=="ISUSER"){
				alert("您的学时违规暂停使用，请联系大连枫升科技电话：400-888-7800");
			}
//			else if(result.error=="yearBefore"){
//				alert("yearBefore");
//				buildYearModel();
//			}else if(result.error=="yearAfter"){
//				alert("yearAfter");
//			}else if(result.error=="NotAccord"){
//				alert("NotAccord");
//			}
			else if(result.error=="N"){
				var logType=$("input[name='yhdl']:checked").val();
				if(logType=="yhm"){
					//if(confirm("确定要缴费吗？")){
			   		//	window.location.href="/zfbPay.xhtml?username="+$('#username').val()+"&password="+ $('#password').val();
			   		//}
					window.location.href="/yinHLogin.xhtml?username="+$('#username').val()+"&password="+ $('#password').val();
				}else if(logType=="sfz"){
					//if(confirm("确定要缴费吗？")){
			   		//	window.location.href="/zfbPay.xhtml?username="+$('#PAPERSCODE').val()+$('#CERTTYPE').val()+"&password="+ $('#password').val();
			   		//}
					window.location.href="/yinHLogin.xhtml?username="+$('#PAPERSCODE').val()+$('#CERTTYPE').val()+"&password="+ $('#password').val();
				}
			}else if(result.error=="continuePay"){
				//alert(result.msg);
//				if(logType=="yhm"){
//					if(confirm(result.msg+",续费成功后需要重新登录,确定要缴费吗？")){
//			   			window.location.href="/zfbPay.xhtml?username="+$('#username').val()+"&password="+ $('#password').val();
//			   		}else {
//			   			window.location.href="/index.xhtml";
//			   		}
//				}else if(logType=="sfz"){
//					if(confirm(result.msg+",续费成功后需要重新登录,确定要缴费吗？")){
//			   			window.location.href="/zfbPay.xhtml?username="+$('#PAPERSCODE').val()+$('#CERTTYPE').val()+"&password="+ $('#password').val();
//			   		}else {
//			   			window.location.href="/index.xhtml";
//			   		}
//				}
			}else if (result.success==""||result.success=="null"||result.success==null) {
				alert(result.error);
			} else if(result.success=="notFace"){
				alert(result.error);
				window.location.href="/index.xhtml";
			}else if(result.success=="isFace"){
				//buildModel();
				$('#comparePhoto').window('open');
			 onstart();
			}
		}
	});
}
//
function buildYearModel() {
	$('#info').css('display', 'block');
	$('#_pass').css('display', 'none');
	$("#buildYearModel").dialog({
		title : "",
		'class' : "mydialog",
		onClose : function() {
			$(this).dialog("close");
		},
		buttons : [ {
			text : "关闭",
			'class' : "btn-primary",
			click : function() {
				$(this).dialog("close");
				$('#buildYearModel').css('display', 'none');
			}
		}, {
			text : "确认",
			'class' : "btn-success",
			click : function() {
				// 开始提交找回密码信息
			    var closeBtn=this;
			    $.ajax({
					url : "/userTakeLogin.xhtml?" + new Date().getTime(),
					data : {
						'PAPERSCODE' : personIdcard,
						'USERNAME' : username,
						'yhdl':yhdl,
						'CERTTYPE' : certType,
						'PASSWORD' : password,
						'rand' : VerifyCode,
						'faceField' : $('#facefield').val()
					},
					type : "POST",
					async : true,
					beforeSend : function() {
						$("#load1").text("正在提交数据请稍候......").show();
					},
					success : function(result) {
						if(result.error=="N"){
							var logType=$("input[name='yhdl']:checked").val();
							if(logType=="yhm"){
								if(confirm("确定要缴费吗？")){
						   			window.location.href="/zfbPay.xhtml?username="+$('#username').val()+"&password="+ $('#password').val();
						   		}
							}else if(logType=="sfz"){
								if(confirm("确定要缴费吗？")){
						   			window.location.href="/zfbPay.xhtml?username="+$('#PAPERSCODE').val()+$('#CERTTYPE').val()+"&password="+ $('#password').val();
						   		}		  
							}
							return;
						}else if (result.error) {
							alert(result.error);
							$("#load1").hide();
							return;
						} else {
							//$('#commitNot')[0].reset();
							$('#image').removeAttr("src");
							$('#facefield').val("1");
							alert(result.success);
							window.location.href="/index.xhtml";
							$(closeBtn).dialog("close");
						}
						$("#load1").hide();
					}
				});
			}
		} ]
	});
	$('#buildYearModel').css('display', 'block');
}




//找回密码
function buildPassBackModel() {
	$('#info').css('display', 'block');
	$('#_pass').css('display', 'none');
	$("#buildPassBackModel").dialog({
		title : "找回密码",
		'class' : "mydialog",
		onClose : function() {
			$(this).dialog("close");
		},
		buttons : [ {
			text : "关闭",
			'class' : "btn-primary",
			click : function() {
				$(this).dialog("close");
				$('#buildPassBackModel').css('display', 'none');
			}
		}, {
			text : "确认",
			'class' : "btn-success",
			click : function() {
				// 开始提交找回密码信息
			    var closeBtn=this;
				$.ajax({
					url : "/goBackPass.xhtml?" + new Date().getTime(),
					data : {
						'PAPERSCODE' : $('#_personIdcard').val(),
						'CERTTYPE' : $('#_certType').val(),
						'CERTCODE' : $('#_certCode').val(),
						'rand' : $('#_randCode').val()
					},
					type : "POST",
					async : true,
					beforeSend : function() {
						$("#load").text("正在提交数据请稍候......").show();
					},
					success : function(result) {
						if (result.error) {
							//alert("1");
							alert(result.error);
							$("#load").hide();
							return;
						} else {
							$('#info').css('display', 'none');
							$('#_pass').css('display', 'block');
							$('.btn-success').css('display', 'none');
							$('#pass').html(result.msg);
							//window.location.href="/index.xhtml";
							//$(closeBtn).dialog("close");
						}
						$("#load").hide();
					}
				});
			}
		} ]
	});
	$('#buildPassBackModel').css('display', 'block');
}

function onstart() {
	$(".modal-dialog").css("width","720" );
//	alert(DetectActiveX());
 	var o = null;
	var result=0;
	var objecthtml='<object id="yourFace" width=320 height=240  '
					+'	classid="CLSID:6C94E32C-B15B-4A66-9FA8-6C1D013D0D19"  codebase="/upload/camera.cab#version=2,0,0,1" '
					+'></object> ' ;
	if(!o){
		if($("#yourFace").length<=0)
		$("#camera").html(objecthtml);
	}
if("ActiveXObject" in window){
	o = document.getElementById("yourFace")
	if(!o.object) {		
		var cameraAlert="<div  style='width:320px;height:240px;font-size:18px;color:#ff4444;background-color:#ECEBE9;margin:auto;font-weight:700'><br/><br/><br/><br/>您没有安装控件，点击此处<a href='/upload/setup.exe' >下载控件</a>,如何安装控件<a target='_blank' href='jxjycameraHelper.html'>请点我</a></div>"
		$("#camera").html(cameraAlert);
		return;
	} else{
		alert("摄像头连接正常！");		
	} 
	}
	result=yourFace.Start();
	if(result==0){
	//	alert("摄像头已连接");
		hasCamera=true;
		return;
		}
	if(result==-50){
	alert("没有检测到摄像头");
	hasCamera=false;
	return ;
	}
	if(result==-51){
	alert("摄像头被占用");
	hasCamera = false;
	return;
	}
}


function setText(sr) {
	/*var o = document.getElementById("yourFace");
	if (!o.object) {
		alert("控件未安装,需要到下载页面下载控件！");
		return;
	}*/
	// 设置加密随机数
	str = yourFace.SetRandomText(sr);
	$('#imageBase64').val(str);
	// 向服务器发送
	return str;
}

//启动控件
function beginGetFace(obj) {
	// 通过Ajax获取加密的随机数
	$.ajax({
		url : "/initFaceCamera.xhtml?" + new Date().getTime(),
		data : {},
		type : "POST",
		async : false,
		success : function(result) {
			if (result.error) {
				alert(result.error);
			} else {
				facerc = result.msg;
				//onstart();
				//生成图片
				 if (!facerc)
						return;
					imageInfo = setText(facerc);
					userLongin(obj);
				//dealImage(imageInfo);
			}
		}
	});
}

// 弹出采集人脸框
var confirm1="确认登陆";
var confirm2="正在提交..";
function buildModel() {
	$("#confirmUserInfo").dialog({
		title : "验证人脸       (※不能使用照片进行采集人脸，如发现不予审证及扣分)",
		'class' : "mydialog",
		'style':"color: red",
		onClose : function() {
			$(this).dialog("close");
		},
		buttons : [ {
			text : "关闭",
			'class' : "btn-primary",
			click : function() {
				$(this).dialog("close");
				$('#confirmUserInfo').css('display', 'none');
			}
		}, {
		text : "确认",
			'class' : "btn-success",
			'id':"confirm2",
			click : function() {
			 /*   if($('#facefield').val()==1||$('#facefield').val()==''){
			        alert('请先点击红色“生成照片”后再点击绿色“确认”！');
			        return ;
			    } */
				// 开始提交登陆信息
			    var closeBtn=this;
			    //if (!facerc)
				//	return;
				//imageInfo = setText(facerc);
				beginGetFace(closeBtn);
			}
		} ]
	});
	// 显示摄像头框
	//beginGetPhoto();
	$('#confirmUserInfo').css('display', 'block');
}

function userLongin(o){
	$.ajax({
					url : "/userTakeLogin.xhtml?" + new Date().getTime(),
					data : {
						'PAPERSCODE' : personIdcard,
						'USERNAME' : username,
						'yhdl':yhdl,
						'CERTTYPE' : certType,
						'PASSWORD' : password,
						'rand' : VerifyCode,
						'faceField' : imageInfo          //fixed
					},
					type : "POST",
					async : true,
					beforeSend : function() {
						//$("#confirm2").attr("disabled","true");
						document.getElementById("confirm2").disabled="disabled";
						document.getElementById("confirm2").value=confirm2;
						//$("#confirm2").html(confirm2);
					},
					success : function(result) {
						//$("#confirm2").removeAttr("disabled");
						//$("#confirm2").html(confirm1);
						document.getElementById("confirm2").removeAttribute("disabled"); 
						document.getElementById("confirm2").value=confirm1;
						if(result.error=="N"){
							var logType=$("input[name='yhdl']:checked").val();
							if(logType=="yhm"){
								if(confirm("确定要缴费吗？")){
						   			window.location.href="/zfbPay.xhtml?username="+$('#username').val()+"&password="+ $('#password').val();
						   		}
							}else if(logType=="sfz"){
								if(confirm("确定要缴费吗？")){
						   			window.location.href="/zfbPay.xhtml?username="+$('#PAPERSCODE').val()+$('#CERTTYPE').val()+"&password="+ $('#password').val();
						   		}		  
							}
							return;
						}
						else if(result.error=="yearBefore"){
//							if(confirm(result.msg)){
//								window.location.href="/zfbPay.xhtml?username="+$('#username').val()+"&password="+ $('#password').val();      
//							}else {
//								$('#image').removeAttr("src");
//								$('#facefield').val("1");
//								alert(result.success);
//								window.location.href="/index.xhtml";
//								$(closeBtn).dialog("close");
//							}
							$('#image').removeAttr("src");
							$('#facefield').val("1");
							//alert(result.success);
							window.location.href="/index.xhtml";
							//$(closeBtn).dialog("close");
							$('#comparePhoto').window('close');
						}else if(result.error=="yearAfter"){
							//alert("yearAfter");
							$('#image').removeAttr("src");
							$('#facefield').val("1");
							//alert(result.success);
							window.location.href="/index.xhtml";
							//$(closeBtn).dialog("close");
							$('#comparePhoto').window('close');
						}else if(result.error=="NotAccord"){
							alert("您的从业信息有误，请联系客服给您修改!");
							//$('#commitNot')[0].reset();
							$('#image').removeAttr("src");
							$('#facefield').val("1");
							//alert(result.success);
							window.location.href="/index.xhtml";
							//$(closeBtn).dialog("close");
							$('#comparePhoto').window('close');
						}					
						else if (result.error) {
							alert(result.error);
							$("#load1").hide();
							return;
						} else {
							//$('#commitNot')[0].reset();
							$('#image').removeAttr("src");
							$('#facefield').val("1");
							//alert(result.success);
							window.location.href="/index.xhtml";
							//$(closeBtn).dialog("close");
							$('#comparePhoto').window('close');
						}
						$("#load1").hide();
					}
				});
	
}
// 获取数据
function base64_toimage() {
	// AJAX判断是否存在人脸
	$('#image').attr("src",
			"data:image/png;base64," + $.scriptcam.getFrameAsBase64());
	var faceField = $.scriptcam.getFrameAsBase64();
	$('#facefield').val(faceField);

};

// 信息验证
function verifyMyInfo() {
	if ("yhm"==yhdl&&!username) {
		alert("请填写用户名");
		return false;
	}
	
	if ("sfz"==yhdl&&!personIdcard) {
		alert("请填写身份证号");
		return false;
	}
	if ("sfz"==yhdl&&!certType) {
		alert("请选择从业类型");
		return false;
	}
	if (!password) {
		alert("请填写密码");
		return false;
	}
	if (!VerifyCode) {
		alert("请填写验证码");
		return false;
	}
	return true;
}

// 拍照
function beginGetPhoto() {
	$("#webcam").scriptcam({
		path : '/view/ContinuingEducationWeb/js/photo/',
		showMicrophoneErrors : false,
		onError : onError,
		cornerRadius : 20,
		cornerColor : 'e3e5e2',
		onWebcamReady : onWebcamReady,
		onPictureAsBase64 : base64_tofield_and_image
	});
}

function base64_tofield() {
	$('#formfield').val($.scriptcam.getFrameAsBase64());
};

function base64_tofield_and_image(b64) {
	$('#facefield').val(b64);
	$('#image').attr("src", "data:image/png;base64," + b64);
};
function changeCamera() {
	$.scriptcam.changeCamera($('#cameraNames').val());
}
function onError(errorId, errorMsg) {
	$("#btn1").attr("disabled", true);
	$("#btn2").attr("disabled", true);
	alert(errorMsg);
}
function onWebcamReady(cameraNames, camera, microphoneNames, microphone, volume) {
	$.each(cameraNames, function(index, text) {
		$('#cameraNames').append($('<option></option>').val(index).html(text))
	});
	$('#cameraNames').val(camera);
}
