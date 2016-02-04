/**
 * 
 */

// 弹出框
// 获取数据
var confirm1="开始核实";
var confirm2="正在提交";
var facerc;
var hasCamera=false;

function base64_toimage() {
	beginGetFace();
	var imageField;
	imageField=setText(facerc) ;
	
	// AJAX判断是否存在人脸
	/*$('#image').attr("src",
			"data:image/png;base64," + $.scriptcam.getFrameAsBase64());
	var faceField = $.scriptcam.getFrameAsBase64();
	$('#facefield').val(faceField); */

	$.ajax( {
		url : "commitFaceImage.xhtml?" + new Date().getTime(),
		data : {
			'faceField' : imageField
		},
		type : "POST",
		beforeSend: function(){
			document.getElementById("btn2").disabled="disabled";
			document.getElementById("btn2").value=confirm2;
		},
		success : function(result) {
			document.getElementById("btn2").removeAttribute("disabled"); 
			document.getElementById("btn2").value=confirm1;
			if (result.success) {
				alert(result.success);
				$('#comparePhoto').window('close');
				$('#facefield').val("1");
				$('#image').attr("src", "");
				window.myIframe.getSWF("fla").faceCheckDone();
				//
				$.ajax( {
					url : "checkTime.xhtml?" + new Date().getTime(),
					data : {
						'id' :'0'
					},
					type : "GET",
					success : function(result) {}
				});
				
				
			} else {
				alert(result.error);
			}
		}
	});
};

function onstart() {
	/*var o = document.getElementById("yourFace");
	if (!o.object) {
		alert("控件未安装,需要到下载页面下载控件！");
		return;
	}*/
		var result=yourFace.Start();
		if(result==0)
			alert("摄像头已连接");
			hasCamera=true;
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
	// 设置加密随机数
	str = yourFace.SetRandomText(sr);
	// 向服务器发送
	return str;
}
//启动控件
function beginGetFace() {
	if(hasCamera==false){
		alert("请连接摄像头后，点击‘检测设备’按钮");
		return;
	}
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
				var imageField;//
				imageField=setText(result.msg) ;//获取加密的随机数
				// 提交人脸数据
				$.ajax( {
					url : "commitFaceImage.xhtml?" + new Date().getTime(),
					data : {
						'faceField' : imageField
					},
					type : "POST",
					beforeSend: function(){
						document.getElementById("btn2").disabled="disabled";
						document.getElementById("btn2").value=confirm2;
					},
					success : function(result) {
						document.getElementById("btn2").removeAttribute("disabled"); 
						document.getElementById("btn2").value=confirm1;
						if (result.success) {
							alert(result.success);
							$('#comparePhoto').window('close');
							$('#facefield').val("1");
							$('#image').attr("src", "");
							window.myIframe.getSWF("fla").faceCheckDone();
							// 对时
							$.ajax( {
								url : "checkTime.xhtml?" + new Date().getTime(),
								data : {
									'id' :'0'
								},
								type : "GET",
								success : function(result) {}
							});
							
							
						} else {
							alert(result.error);
						}
					}
				});
			}
		}
	});
}

// 拍照
function beginGetPhoto() {
	$("#webcam").scriptcam( {
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

$(function() {
	loadBody();
	loadSurplusTtme();
	satisfyHour();
	surplusHour();
	if(!("ActiveXObject" in window)){
	//	alert("请使用IE浏览器");
		if($("#yourFace").length>0){
			var faceobj='<object id="yourFace" type="application/x-itst-activex" '
				+'	style="border:0px;width:320px;height:240px;" '
				+'	clsid="{6C94E32C-B15B-4A66-9FA8-6C1D013D0D19}" '
				+'	progid="FileOperation.CAB#version=1,0,0,1">'
			$("#camera").html(faceobj);
		}
	};
	// var p=${cp}1;
	$('#faceImageField')[0].reset();
	// 需要判断用户是否已经登录,如果已经登录则显示下面内容
	var menutitle = $('#menu-title');
	var playType=$('#playType').val();
	$('#cc>.panel>.panel-header').css("padding", "0").html(menutitle);
	$('#cc').layout('resize', {});
	$('#tt').tree(
			{
				url : '/contentMenu.xhtml',//用户学习的菜单
				idField : 'id',
				treeField : 'text',
				animate : true,
				collapsible : true,
				formatter : function(node) {
					var s = node.text;
					if (node.children) {
						s = '&nbsp;<a style=\'color:#333333\' title="'
								+ node.text + '" >' + node.text+'</a>';
					}
					return s;
				},
				onSelect : function(node) {
					var url = node.url;
					if (url) {
						var parent1 = $('#tt').tree('getParent', node.target);
						var parent2 = $('#tt').tree('getParent', parent1.target);
						//$('#fla>param[name=flashvars]')[0].value = "varbp="
						//		+ url;
						//var s = $('#fla>embed').attr('flashvars',
						//		"varbp=" + url);
						$('#neirong').html(node.text);	//内容
						$('#jie').html(parent1.text);	//节
						$('#danyuan').html(parent2.text);//单元
						// TODO 更换教育内容，参数：（内容的URL，页码）
						//window.myIframe.getSWF("fla").changeContent(url, 1);
						
						var page=1;
						if(playType=="before"){
							page=$('#cp').val();
							if(selectTimes==2)
								$('#playType').val("");
						}
						$('#myIframe').attr('src','/reInitPlay.xhtml?contentId='+node.id+"&url="+url+"&page="+page);
						playType="";
//						$.ajax( {
//							url : "/reInitPlay.xhtml",
//							type : "POST",
//							data : {
//								"contentId" : node.id
//							},
//							success : function(result) {
//								//alert("see");
//								//alert(result.success);
//							}
//						});
					}
				},
				onLoadSuccess : function(node, data) {
					// TODO 需要传一个教学内容的ID
					var contentId = $('#contentId').val();
					expandTo(contentId);
				}
			});

	var typePlay=$('#playType').val();
	if(typePlay=="laws"){ 
		$('#ac').accordion('select','│法律法规(不计时)');
		//$('#ac').accordion(
		//	'onSelect':function(title,index){
		//	$('#ac').accordion('getPanel',title).panel({iconCls:'icons-ok'}); 
		//});
	}
	
	$('#menu-title').on('click', function() {
		$('#cc').layout('collapse', 'west');
	});
	$('#tt').tree( {
		onClick : function(node) {
			$('#tt').tree('toggle', $(node.target));
		}
	});

	$('#comparePhoto').window( {
		onOpen : function() {
			beginGetPhoto();
		}
	})
});
var selectTimes=0;
function expandTo(tid) {
	var node = $('#tt').tree('find', tid);
	var playType=$('#playType').val();
	if(playType=="before"){
		if (node) {
			selectTimes++;
			var page=$('#cp').val();
			var mainFrame = document.getElementById("myIframe");
			$('#tt').tree('expandTo', node.target).tree('select', node.target);
			mainFrame.contentWindow.getSWF("fla").changeContent(node.url, page);
		}
	}
}

function toZfb() {
	if (confirm("确定要续费吗？")) {
		window.location.href = "/zfbPay.xhtml?username=" + $("#username").val()
				+ "&password=" + $("#password").val();
	}
}
function loadBody() {
	$.ajax( {
		type : "GET",
		url : "/time.xhtml?"+ new Date().getTime(),
		data : "",
		async : true,
		beforeSend : function() {
			// $("#load").text("正在提交数据请稍候......").show();
	},
	success : function(text) {
		// var str1 =text.split(",") ;
		$('#timeLong').text(text);
		// $('#timeLongM').text(str1[0]);
	}
	});
}
function loadSurplusTtme() {
	$.ajax( {
		type : "GET",
		url : "/surplusTtme.xhtml?"+ new Date().getTime(),
		data : "",
		async : true,
		beforeSend : function() {
			// $("#load").text("正在提交数据请稍候......").show();
	},
	success : function(text) {
		// var str1 =text.split(",") ;
		$('#surplusTtme').text(text);
		// $('#surplusTtmeM').text(str1[0]);
	}
	});
}


function satisfyHour() {
	$.ajax( {
		type : "GET",
		url : "/satisfyHour.xhtml?"+ new Date().getTime(),
		data : "",
		async : true,
		beforeSend : function() {
			// $("#load").text("正在提交数据请稍候......").show();
	},
	success : function(text) {
		// var str1 =text.split(",") ;
		$('#satisfyHour').text(text);
		// $('#timeLongM').text(str1[0]);
	}
	});
}

function surplusHour() {
	$.ajax( {
		type : "GET",
		url : "/surplusHour.xhtml?"+ new Date().getTime(),
		data : "",
		async : true,
		beforeSend : function() {
			// $("#load").text("正在提交数据请稍候......").show();
	},
	success : function(text) {
		// var str1 =text.split(",") ;
		$('#surplusHour').text(text);
		// $('#surplusTtmeM').text(str1[0]);
	}
	});
}




//弹题
function ExpandExamPage() {
	//弹题
	var suv=$('#garlic').val();
	PopSubject1(suv);
}

//人脸
function ExpandPage() {
	//人脸
	$('#comparePhoto').window('open');
}
