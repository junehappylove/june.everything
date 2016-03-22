$(function() {
	var url = $('#url').val();
	var page = $('#page').val();
	//getSWF("fla").changeContent(url, page);
});

setTimeout("myplay()",1000);

function myplay(){
	var url = $('#url').val();
	var page = $('#page').val();
	getSWF("fla").changeContent(url, page);
}
// 保存学时   -- 应该是flash中调用此方法提交上传学时
function swfDo_Save(studyTime, progress, saveType) {	//saveType=page
	// 通过“上一个”“下一页”按钮 saveType=page
	// 通过JavaScript调用 javascript
	// 通过“保存学时”按钮 button
	// alert(studyTime+","+progress+","+saveType);
	// 举个栗子：
	// ctinfo=06X412448X11137&	-- 这个貌似是加密后数据，应该表示学习了多长时间
	// pos=010102002&			-- 这个大概根据目录名称就可以组合出来，表示当前所在哪个小节上
	// starttime=z0m5VMQ2vNNvowmdUHzJUVQXW06fhCH3  -- 这个是加密后数据，解码后应该是开始时间
	// ctinfo=0518184012160&pos=020101000&starttime=5X7aTEGGq8ryTe0wIGIVnFQXW06fhCH3
	// ctinfo=05121730015120&pos=020101001&starttime=5X7aTEGGq8ryTe0wIGIVnFQXW06fhCH3
	// ctinfo=061412900111040&pos=020101003&starttime=5X7aTEGGq8ryTe0wIGIVnFQXW06fhCH3
	// ctinfo=061514600111820&pos=020101004&starttime=5X7aTEGGq8ryTe0wIGIVnFQXW06fhCH3
	// ctinfo=061710200121860&pos=020101000&starttime=5X7aTEGGq8ryTe0wIGIVnFQXW06fhCH3
	// ctinfo=061816840131900&pos=020101004&starttime=5X7aTEGGq8ryTe0wIGIVnFQXW06fhCH3
	
	// ctinfo=05V111098V3158&pos=020102000&starttime=0xvz4sh5R4OAq%2FiF5j5cNFQXW06fhCH3
	// ctinfo=06V319380V11074&pos=020102002&starttime=0xvz4sh5R4OAq%2FiF5j5cNFQXW06fhCH3
	// ctinfo=06V613008V11790&pos=020102004&starttime=0xvz4sh5R4OAq%2FiF5j5cNFQXW06fhCH3
	// ctinfo=06V719476V21148&pos=020102005&starttime=0xvz4sh5R4OAq%2FiF5j5cNFQXW06fhCH3
	
	// ctinfo=05V113604V7116&pos=020203001&starttime=0xvz4sh5R4MaeFi12GwzvFQXW06fhCH3
	// ctinfo=06V313652V11432&pos=020203003&starttime=0xvz4sh5R4MaeFi12GwzvFQXW06fhCH3
	// ctinfo=06G319674G11673&pos=020203004&starttime=0xvz4sh5R4OFrv7omj4XYVQXW06fhCH3
	// ctinfo=05J91966J6104&pos=040102001&starttime=0xvz4sh5R4Pny%40IpblfoOFQXW06fhCH3
	// ctinfo=05J311408J9106&pos=040102002&starttime=0xvz4sh5R4Pny%40IpblfoOFQXW06fhCH3
	// ctinfo=06J418320J11510&pos=040102004&starttime=0xvz4sh5R4Pny%40IpblfoOFQXW06fhCH3
	// ctinfo=06J614628J11812&pos=040102005&starttime=0xvz4sh5R4Pny%40IpblfoOFQXW06fhCH3
	// ctinfo=06J812748J21114&pos=040102006&starttime=0xvz4sh5R4Pny%40IpblfoOFQXW06fhCH3
	// ctinfo=06J919660J21416&pos=040102007&starttime=0xvz4sh5R4Pny%40IpblfoOFQXW06fhCH3
	// 0xvz4sh5R4PKvaCDtoVlGVQXW06fhCH3
	// 0xvz4sh5R4PKvaCDtoVlGVQXW06fhCH3
	// 0xvz4sh5R4PKvaCDtoVlGVQXW06fhCH3
	// AEiDitYYsrUZsVZh@STnSVQXW06fhCH3
	// AEiDitYYsrUZsVZh@STnSVQXW06fhCH3
	// AEiDitYYsrWj4VGq2FBHN1QXW06fhCH3
	$.ajax( {
		url : "/saveEduTime.xhtml?" + new Date().getTime(),
		data : {
			'ctinfo' : studyTime,
			'pos' : progress,
			'starttime' : $('#starttime').val()
		},
		type : "POST",
		success : function(result) {
			if (result.success == "showFace") {
				getSWF("fla").recoveWindow();
				getSWF("fla").faceCheckStart();
				window.parent.ExpandPage();// 打开人脸，在jquery.menu-page.js中定义
			} else {
				if (saveType == "button") {
					getSWF("fla").recoveWindow();
					alert(result.msg);
				} else if (saveType == "page") {
					if(result.success=="fullTime"){
						getSWF("fla").recoveWindow();
						alert(result.msg);
					}else if(result.success=="endBeforeStrat"){
						getSWF("fla").recoveWindow();
						alert(result.msg);
					}
				}
			}
		}
	});

}

function setToUrl(url) {
	alert(url);
}

// 弹题
function swfDo_test(isPlay) {
	getSWF("fla").recoveWindow();
	window.parent.ExpandExamPage();
	// $('#isPlayV').val('2');
	// PopSubject1(suv);
	// $('#comparePhoto').window('open');
}

/*
function getSWF(movieName) {
	var e = document.getElementById(movieName);
	return (navigator.appName.indexOf("Microsoft") != -1) ? e : e
			.getElementsByTagName("embed")[0];

}
*/
function getSWF( swfID ) {
	if (window.document[ swfID ]&&navigator.appName.indexOf("Microsoft")==0) {
      return window.document[ swfID ];
    } else if (navigator.appName.indexOf("Microsoft") == -1) {
      if (document.embeds && document.embeds[ swfID ]) {
        return document.embeds[ swfID ];
      }
    } else {
      return document.getElementById( swfID );
    }
  }

// flash 调用的方法
function starttime() {
	return $('#starttime').val();
}