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
function swfDo_Save(studyTime, progress, saveType) {
	// 通过“上一个”“下一页”按钮 saveType=page
	// 通过JavaScript调用 javascript
	// 通过“保存学时”按钮 button
	//alert(studyTime+","+progress+","+saveType);
	// 举个栗子：
	// ctinfo=06X412448X11137&	-- 这个貌似是加密后数据，赢表示学习了多长时间
	// pos=010102002&		-- 这个大概根据目录名称就可以组合出来，表示当前所在哪个小节上
	// starttime=z0m5VMQ2vNNvowmdUHzJUVQXW06fhCH3  -- 这个是加密后数据，解码后应该是开始时间
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
				window.parent.ExpandPage();
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