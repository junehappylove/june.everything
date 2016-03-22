/**
 * author fensheng13
 */
;
$(document).ready(function() {
	$(document).bind("contextmenu", function(e) {
		return false;
	});
	$(document).bind("selectstart", function(e) {
		return false;
	});

});

document.onkeydown = function(e) {
	var isie = (document.all) ? true : false;
	var key;
	var ev;
	if (isie) {// IE浏览器
		key = window.event.keyCode;
		ev = window.event;
	} else {
		// 火狐浏览器
		key = e.which;
		ev = e;
	}
	if (key == 9 || key == 13) {// IE浏览器
		if (isie) {
			ev.keyCode = 0;
			ev.returnValue = false;
		} else {// 火狐浏览器
			ev.which = 0;
			ev.preventDefault();
		}
	}
};

function getQuestion() {
	var subject = "";
	var sub_title = "";
	var sub_type = "";
	var optA = "";
	var optA = "";
	var optC = "";
	var optD = "";
	var test = $('#test');
	var result = "";
	var html_content = "";
	// 获取题目
	$.ajax({
		type : "POST",
		url : "/getOurQuestion.xhtml",
		data : "",
		// 禁止异步返回数据
		async : false,
		success : function(text) {
			if (text != "error") {
				subject = eval(text);
				sub_title = subject[0]["eTitle"];
				optA = subject[0]["optionA"];
				optB = subject[0]["optionB"];
				optC = subject[0]["optionC"];
				optD = subject[0]["optionD"];
				sub_type = subject[0]["eType"];
				$("#answer").focus();
				if (sub_type == 1) {
					sub_type = "判断题:";
					html_content = sub_type + '<br/>' + sub_title + '<br/>'
							+ optA + '<br/>' + optB + '<br/> '
					/*
					 * + '选择答案:<input type="button" value=\"' + optA.substr(0,
					 * 1) + '\" class="answer_button blue"
					 * onclick="answerQuestion(this.value)"><input
					 * type="button" value=\"' + optB.substr(0, 1) + '\"
					 * class="answer_button blue"
					 * onclick="answerQuestion(this.value)">'
					 */;

				} else {
					sub_type = "单选题:";
					html_content = sub_type + '<br />' + sub_title + '<br/>'
							+ optA + '<br/>' + optB + '<br/> ' + optC
							+ '<br/> ' + optD + '<br/>'
					/*
					 * + '选择答案:<input type="button" class="answer_button blue"
					 * value=\"' + optA.substr(0, 1) + '\"
					 * onclick="answerQuestion(this.value)"><input
					 * type="button" value=\"' + optB.substr(0, 1) + '\"
					 * class="answer_button blue"
					 * onclick="answerQuestion(this.value)">' + '<input
					 * type="button" value=\"' + optC.substr(0, 1) + '\"
					 * class="answer_button blue"
					 * onclick="answerQuestion(this.value)"><input
					 * type="button" value=\"' + optD.substr(0, 1) + '\"
					 * class="answer_button blue"
					 * onclick="answerQuestion(this.value)">'
					 */;
				}
			} else {
				subject = [ '无法获取题目' ];
			}
		}
	});
	return html_content;
}

// 答题
function answerQuestion() {
	var result = false;
	// 点击提交后异步提交答案，答案正确关闭层，否则重新加载题目
	$.ajax({
		type : "POST",
		url : "/answerOurQuestion.xhtml",
		data : "answer=" + $.trim($('#answer')[0].value) + '&sugar='
				+ $.trim($('#sugar')[0].value),
		// 禁止异步返回数据
		async : false,
		success : function(text) {
			if ($.trim(text) == 'right') {
				alert("答案正确");
				//startPlay();
				//$('#isPlayV').val("1");
				result = true;
				window.myIframe.getSWF("fla").replay();
				layer.closeAll();
			} else {
				// layer.closeAll();
				alert('答案错误，继续答题');
				$('#submitSub')[0].reset();
				// var question = getQuestion();
				var question = $("<img src='/getQuestion.xhtml?"
						+ Math.random() + "' />");
				$('#question_content').html(question);
				$("#answer").focus();
				result = false;
			}
		},
		error : function(XMLHttpRequest, status) {
			if (status == 'timeout') {// 超时,status还有success,error等值的情况
				/* ajaxTimeoutTest.abort(); */
				alert("超时");
			}
			if (status == 'error') {// 超时,status还有success,error等值的情况
				/* ajaxTimeoutTest.abort(); */
				alert("服务器请求错误");
			}
		}
	});
	return result;
}
// 先弹框版本
function PopSubject1(suv) {
	var title = "填写正确答案后继续";
	$
			.layer({
				type : 1,
				title : title,
				shade : [ 0.5, '#000' ],
				area : [ '420px', '260px' ],
				closeBtn : [ 0 ],
				shift : 'left', // 从左动画弹出
				page : {
					html : '<div id="content" class="content" style="text-align:left;"><form id="submitSub">'
							+ '<div id="test"><div id="question_content"></div>请填写您的答案：<input id="answer"  autocomplete="off" required="" pattern=".{1,}" maxlength="1" style="width:20px;margin-right:10px">'
							+ '<input id="YourAnswer" class="answer_button yellow" type="button" value="提交"><input id="sugar" type="hidden" name="sugar" value=\"'
							+ suv + '\"/> </form></div></div>'
				},
				close : function(index) {
					layer.msg('您获得了子窗口标记：'
							+ layer.getChildFrame('#name', index).val(), 1, 1);
				},
				success : function(layero) {
					// 获取题目
					var result = false;
					var answer = $('#answer');

					// var question = getQuestion();
					var question = $("<img src='/getQuestion.xhtml?"
							+ Math.random() + "' />");
					$('#question_content').append(question);
					$('#question_content').css({
						padding : "10px"
					});
					// 点击提交后异步提交答案，答案正确关闭层，否则重新加载题目
					$("#answer").keyup(function() {
						if (($('#answer')[0].value)) {
							result = answerQuestion();
						}
					});

					$("#answer").focus();
					$("#YourAnswer").bind("click", function() {
						result = answerQuestion();
					});
					layer.shift('left');
				}
			});
}