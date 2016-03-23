// --------------swf 播放函数整理 ---------------
//jxjy_AS3_V01_01_fla.MainTimeline

package jxjy_AS3_V01_01_fla
{
    public dynamic class MainTimeline extends MovieClip 
    {
        public var tTips:TextField;
        public var btn_screenSwap:SimpleButton;
        public var btn_pageNext:SimpleButton;
        public var tfMsg:TextField;
        public var mc_slidebarContent:MovieClip;
        public var btn_saveTime:SimpleButton;
        public var mc_loading:MovieClip;
        public var mc_soundControler:MovieClip;
        public var mc_noInit:MovieClip;
        public var btn_test:MovieClip;
        public var txt_pageNum:TextField;
        public var btn_pagePrev:SimpleButton;
        public var mc_contnetControler:MovieClip;
        public var mc_movieBox:MovieClip;
        public var STATUS_NOMOVIE:String;		// 没有视频
        public var STATUS_LOADING:String;		// 加载
        public var STATUS_PLAYING:String;		// 播放
        public var STATUS_PAUSED:String;		// 暂停
        public var STATUS_TESTING:String;		// 测试（视频学习完成后弹出测试题供学员做）
        public var STATUS_CHECKING:String;		// 检查（检查学员的答题）
        public var STATUS_PLAYDONE:String;		// 播放完毕
        public var TEST_DEFAULT_FIRST_TIME:Number;
        public var TEST_DEFAULT_INTERVAL_TIME:Number;
        public var SAVE_BUTTON:String;
        public var SAVE_PAGE:String;
        public var SAVE_JAVASCRIPT:String;
        public var mCfgMaxTime:Number;
        public var mCfgMaxTimeMultiple:Number;
        public var mCfgRightMenuName:String;
        public var mCfgRightMenuUrl:String;
        public var curPage:Number;		// 页码当前页，从0开始计算的
        public var pages:Array;
        public var totalPage:int;
        public var strBasePath:String;
        public var arrCurProgress:Array;// 当前页数组
        public var eduMallXml:XML;
        public var sysStatus:String;		// 系统状态
        public var isInstructions:Boolean;	// 是否指令
        public var timer:Timer;
        public var testTime:Number;
        public var bTest:Boolean;
        public var firstTestTime:Number;
        public var testTimeInterval:Number;
        public var oldStudyTime:Number;
        public var studyTime:Number;		// 学习时间
        public var pageTime:Number;
        public var maxTime:Number;
        public var clickCount:Number;//不论点击上一页，或者下一页，都会加1
        public var curMovie:MovieClip;

        public function MainTimeline()
        {
            addFrameScript(0, frame1);
        }

		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		// 上传时长函数
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
        public function callSaveTime(_arg_1:String):void
        {	//_arg_1 = SAVE_PAGE
            var _local_2:*;
            var _local_3:String;
            var _local_4:String;
            var _local_5:String;
            var _local_6:String;
            if (isInstructions)
            {
                return;
            };
            switch (sysStatus)
            {
                case STATUS_NOMOVIE:
                case STATUS_LOADING:
                case STATUS_TESTING:
                case STATUS_CHECKING:
                    return;
                case STATUS_PAUSED:
                case STATUS_PLAYING:
                case STATUS_PLAYDONE:	// 播放完成后触发的事件
                    _local_2 = studyTime;//将学习时间给变量2
                    if (_arg_1 == SAVE_BUTTON)
                    {
					// 如果用户点击的是保存按钮的话
					// 则学时就是当前学时加上页面时间，但这里似乎用不到的
                        _local_2 = (_local_2 + pageTime);	
                    };
                    if ((Math.round((_local_2 / 60)) - Math.round((oldStudyTime / 60))) < 1)
                    {//Math.round(x:Number) 返回一个与x接近的整数，例如：0.5 = 1 ，1.3 = 1 ，1.8 = 2
					// 
                        if (_arg_1 == SAVE_BUTTON)
                        {
                            Alert.show("学时保存成功。", "提示");
                        };
                        return;
                    };
                    oldStudyTime = _local_2;		// 然并卵，没什么大用啊
                    _local_3 = encrypt(_local_2);	// 这个重要，将学习时间加密
                    _local_4 = encrypt(clickCount);
                    _local_5 = getFixedLengthStr(("" + _local_4.length), 2, "0");
                    _local_6 = ((_local_5 + _local_3) + _local_4);
                    ExternalInterface.call("swfDo_Save", _local_6, createStrProgress(), _arg_1);
                    return;
                default:
                    Alert.show(("系统状态异常：" + sysStatus));
            };
        }

        public function createStrProgress():String
        {
            var _local_1 = "";
            _local_1 = (_local_1 + getFixedLengthStr(("" + arrCurProgress[0]), 2, "0"));
            _local_1 = (_local_1 + getFixedLengthStr(("" + arrCurProgress[1]), 2, "0"));
            _local_1 = (_local_1 + getFixedLengthStr(("" + arrCurProgress[2]), 2, "0"));
            return ((_local_1 + getFixedLengthStr(("" + arrCurProgress[3]), 3, "0")));
        }

        public function doTest():void
        {
            sysStatus = STATUS_TESTING;
            stage.displayState = StageDisplayState.NORMAL;
            setPageControlerStatus();
            btn_saveTime.enabled = false;
            btn_screenSwap.enabled = false;
            mc_slidebarContent.enabled = false;
            mc_slidebarContent.j_mc.enabled = false;
            mc_contnetControler.swapContentStatus();
            mc_soundControler.startTest();
            curMovie.stop();
            ExternalInterface.call("swfDo_test", 1);
        }

        public function encrypt(_arg_1:Number):String
        {
            var _local_2 = "ArsNDdeBChqOPSTtuvQREF67GH01KfgLIJwxyzM8iVWmXYjklnopUZc234ab59/=@]]]]";
            var _local_3:String = String(ExternalInterface.call("starttime"));// 读入开始学习时间 加密后的32位长度
			// 举例 _local_3 = "z0 m5 VM Q2 vN Nv ow md UH zJ UV QX W0 6f hC H3";
			// var str:String = "Hello from Paris, Texas!!!"; 
			// trace(str.substr(11,15)); // output: Paris, Texas!!! 
			// trace(str.substring(11,15)); // output: Pari
			// 在 substring() 方法中，第二个参数是子字符串的结尾 处字符的位置（该字符未包含在返回的字符串中）
            var _local_4:String = _local_3.substring(14, 15);	//m	从1开始计算
            var _local_5:String = _local_3.substring(25, 26);	//0
			// 搜索字符串，并返回在调用字符串内 startIndex 位置上或之后找到的 value 的第一个匹配项的位置。
			// 此索引从零开始，这意味着字符串中的第一个字符被视为位于索引 0 而不是索引 1 处。
			// 如果未找到 value，该方法会返回 -1。
            var _local_6:Number = _local_2.indexOf(_local_4);	// 43	// 'm' 在字符串中出现的位置 从0开始，不成功返回-1
            var _local_7:String = _local_2.substring((_local_6 + 1), (_local_6 + 2)); // 就是取下一个字符,取'X'
            var _local_8:Number = _local_2.indexOf(_local_5);	// '0'的位置
            var _local_9:String = _local_2.substring((_local_8 + 1), (_local_8 + 2));	// 取'1'
			// 05X216151X3179
            var _local_10:String = String((((_local_6 * 7) + (_local_8 * 3)) * _arg_1));
            var _local_11:String = (((_local_7 + _local_10.substring(0, 1)) + _local_9) + _local_10.substring(1, _local_10.length));
            return (_local_11);
        }

        public function getFixedLengthStr(_arg_1:String, _arg_2:Number, _arg_3:String):String
        {
            if (_arg_2 <= 0)
            {
                return ("");
            };
            if (_arg_2 <= _arg_1.length)
            {
                return (_arg_1.substr((_arg_1.length - _arg_2)));
            };
            if (_arg_3 == "")
            {
                _arg_3 = "0";
            };
            var _local_4 = "";
			// ceil（Math.ceil 方法）. public static ceil(x:Number) : Number. 
			// 返回指定数字或表达式的上限值。数字的上限值是大于等于该数字的最接近的整数。
			// ceil(12.5)=13
            var _local_5:Number = Math.ceil((_arg_2 / _arg_3.length));
            var _local_6:Number = 0;
            while (_local_6 < _local_5)
            {
                _local_4 = (_local_4 + _arg_3);
                _local_6++;
            };
            _arg_1 = (_local_4 + _arg_1);
            return (_arg_1.substr((_arg_1.length - _arg_2)));
        }

        public function init():void
        {
            btn_saveTime.visible = false;
            mc_loading.visible = false;
            txt_pageNum.text = "第0页/共0页";
            sysStatus = STATUS_NOMOVIE;
            isInstructions = false;
            stage.displayState = StageDisplayState.NORMAL;
            btn_pagePrev.addEventListener(MouseEvent.CLICK, onPagePrev);
            btn_pageNext.addEventListener(MouseEvent.CLICK, onPageNext);
            btn_saveTime.addEventListener(MouseEvent.CLICK, onSaveTime);
            btn_screenSwap.addEventListener(MouseEvent.CLICK, onSwapScreen);
            setPageControlerStatus();
            ExternalInterface.addCallback("replay", replay);
            ExternalInterface.addCallback("changeContent", changeContent);
            ExternalInterface.addCallback("faceCheckStart", faceCheckStart);
            ExternalInterface.addCallback("faceCheckDone", faceCheckDone);
            ExternalInterface.addCallback("setHasTest", setHasTest);
            ExternalInterface.addCallback("setFirstTime", setFirstTime);
            ExternalInterface.addCallback("setIntervalTime", setIntervalTime);
            ExternalInterface.addCallback("recoveWindow", recoveWindow);
            ExternalInterface.addCallback("asAlert", myAlert);
            var _local_1:String = this.loaderInfo.parameters["hasTest"];
            if (_local_1 == null)
            {
                bTest = true;
            } else
            {
                _local_1 = _local_1.toLowerCase();
                if (_local_1 == "n")
                {
                    bTest = false;
                } else
                {
                    bTest = true;
                };
            };
            firstTestTime = Number(this.loaderInfo.parameters["firstTime"]);
            if (isNaN(firstTestTime))
            {
                firstTestTime = TEST_DEFAULT_FIRST_TIME;
            } else
            {
                firstTestTime = Math.round(firstTestTime);
                if (firstTestTime <= 0)
                {
                    firstTestTime = TEST_DEFAULT_FIRST_TIME;
                };
            };
            testTimeInterval = Number(this.loaderInfo.parameters["intervalTime"]);
            if (isNaN(testTimeInterval))
            {
                testTimeInterval = TEST_DEFAULT_INTERVAL_TIME;
                firstTestTime = TEST_DEFAULT_FIRST_TIME;
            } else
            {
                testTimeInterval = Math.round(this.loaderInfo.parameters["testTimeInterval"]);
                if (testTimeInterval <= firstTestTime)
                {
                    testTimeInterval = TEST_DEFAULT_INTERVAL_TIME;
                    firstTestTime = TEST_DEFAULT_FIRST_TIME;
                };
            };
            tTips.text = "";
        }

		// 获取当前页信息
        public function initCurProgress():void
        {
            var _local_2:String;
            var _local_1:Array = strBasePath.split("/");
            if (strBasePath.lastIndexOf("/") != (strBasePath.length - 1))
            {
                _local_2 = _local_1[(_local_1.length - 1)];
            } else
            {
                _local_2 = _local_1[(_local_1.length - 2)];
            };
            var _local_3:Array = _local_2.split("_");
            arrCurProgress.push(Number(_local_3[0]));
            arrCurProgress.push(Number(_local_3[1]));
            arrCurProgress.push(Number(_local_3[2]));
            arrCurProgress.push(curPage);	// 从0开始的页码
        }

        public function loadPage():void
        {
            sysStatus = STATUS_LOADING;
            mc_loading.visible = true;
            mc_loading.setPercent(0);
            setPageControlerStatus();	// 设置上一页，下一页 按钮的控制状态
            if (curPage >= pages.length)
            {
                curPage = (pages.length - 1);// 当前页是最后一页
            };
            var _local_1:String = pages[curPage];
            var _local_2:Loader = new Loader();
            _local_2.contentLoaderInfo.addEventListener(Event.COMPLETE, onPageLoaded);
            _local_2.contentLoaderInfo.addEventListener(ProgressEvent.PROGRESS, onPageLoadProgress);
            _local_2.contentLoaderInfo.addEventListener(IOErrorEvent.IO_ERROR, onPageLoadError);
            pageTime = 0;					// 页面时间置0
            var _local_3:URLRequest = new URLRequest((strBasePath + _local_1));
            _local_2.load(_local_3);
        }
		
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		// 取分页信息的时候	重要，基本数据的初始化操作，都在这里进行的
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
        public function loadXml():void
        {
            testTime = 0;
            oldStudyTime = 0;
            studyTime = 0;
            pageTime = 0;
            maxTime = 0;
            clickCount = 0;
            arrCurProgress = new Array();
            initCurProgress();
            pages = new Array();
            var _local_1:URLRequest = new URLRequest((strBasePath + "eduMall.xml"));
            var _local_2:URLLoader = new URLLoader();
            _local_2.addEventListener(Event.COMPLETE, onXmlLoaded);
            _local_2.addEventListener(IOErrorEvent.IO_ERROR, onXmlIOError);
            _local_2.load(_local_1);
        }

        public function moviePlaydone():void
        {
            mc_contnetControler.swapContentStatus();
        }

        public function readConfig():void
        {
            var _local_1:XML = eduMallXml.child(0)[0];
            var _local_2:XML = _local_1.child(0)[0];
            mCfgMaxTime = Number(_local_2.attribute("value").toXMLString());
            var _local_3:XML = _local_1.child(1)[0];
            mCfgMaxTimeMultiple = Number(_local_3.attribute("value").toXMLString());
            var _local_4:XML = _local_1.child(2)[0];
            mCfgRightMenuName = _local_4.attribute("value").toXMLString();
            var _local_5:XML = _local_1.child(3)[0];
            mCfgRightMenuUrl = _local_5.attribute("value").toXMLString();
            this.contextMenu = createContextMenu();
        }

        public function readPages():void
        {
            var _local_3:Array;
            var _local_4:String;
            var _local_1:XMLList = eduMallXml.child(1)[0].child(0).children();
            totalPage = _local_1.length();
            var _local_2:int;
            while (_local_2 < totalPage)
            {
                _local_3 = new Array();
                _local_4 = _local_1[_local_2].attribute("file").toString();
                _local_3.push(_local_4);
                pages.push(_local_3);
                _local_2++;
            };
        }

        public function createContextMenu():ContextMenu
        {
            var _local_1:ContextMenu = new ContextMenu();
            _local_1.hideBuiltInItems();
            var _local_2:ContextMenuItem = new ContextMenuItem(mCfgRightMenuName);
            _local_2.addEventListener(ContextMenuEvent.MENU_ITEM_SELECT, goCompanyWebsite);
            _local_1.customItems.push(_local_2);
            return (_local_1);
        }

		// 设置上一页，下一页 按钮的控制状态
        public function setPageControlerStatus():void
        {
            switch (sysStatus)
            {
                case STATUS_NOMOVIE:
                case STATUS_LOADING:
                case STATUS_TESTING:
                case STATUS_CHECKING:
                    btn_pagePrev.enabled = false;
                    btn_pageNext.enabled = false;
                    return;
                case STATUS_PAUSED:
                case STATUS_PLAYING:
                case STATUS_PLAYDONE:
            };
            if (pages.length <= 1)
            {
                btn_pagePrev.enabled = false;
                btn_pageNext.enabled = false;
            } else
            {
                if (curPage <= 0)
                {
                    btn_pagePrev.enabled = false;
                } else
                {
                    btn_pagePrev.enabled = true;
                };
                if (curPage >= (pages.length - 1))
                {
                    btn_pageNext.enabled = false;
                } else
                {
                    btn_pageNext.enabled = true;
                };
            };
        }

        public function showPageNum():void
        {
            txt_pageNum.text = (((("第" + (curPage + 1)) + "页/共") + pages.length) + "页");
        }

		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		// 页面加载即调用此方法
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
        public function onPageLoaded(_arg_1:Event):void
        {
            var _local_2:LoaderInfo = LoaderInfo(_arg_1.currentTarget);
            var _local_3:MovieClip = MovieClip(_local_2.content);
            if (sysStatus != STATUS_CHECKING)
            {	// 加载即开始播放视频
                sysStatus = STATUS_PLAYING;
            };
            mc_loading.visible = false;
            stage.frameRate = _local_2.frameRate;	// 帧频：12
			// 计算播放最大时间 = 总帧数 / 帧频 
            maxTime = Math.floor((_local_3.mv_mc.totalFrames / _local_2.frameRate));
            timer = new Timer(1000);		// 启动定时器
            timer.addEventListener(TimerEvent.TIMER, onTimer);
            timer.start();
            curMovie = _local_3.mv_mc;		// 当前视频信息
            mc_contnetControler.setMovieclip(curMovie);
            mc_slidebarContent.setMovieClip(curMovie);
            mc_movieBox.changeMovie(curMovie);
            showPageNum();					// 展示页码信息，由此也可以看到，当前页面curPage是从0开始的
            arrCurProgress[3] = curPage;	// 维护好页码数组的最后一个位置，总是指向（当前页码-1）
            setPageControlerStatus();		// 控制翻页按钮的状态
            if (pages[curPage] == "end.swf")// 如果是最后一页，给一个提示
            {
                tTips.text = "本教学内容已经播放完毕，请更换内容继续学习。";
            } else
            {
                tTips.text = (("本页完全播放最大可获得学时" + maxTime) + "秒，完全播放后，翻页才可以记录该学时。");
            };
        }

		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		// 点击下一页按钮事件
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
        public function onPageNext(_arg_1:MouseEvent):void
        {
            switch (sysStatus)
            {
                case STATUS_NOMOVIE:
                case STATUS_LOADING:
                case STATUS_TESTING:
                case STATUS_CHECKING:
                    return;
                case STATUS_PAUSED:
                case STATUS_PLAYING:
                case STATUS_PLAYDONE:
            };
            if (isNaN(curPage))
            {
                return;
            };
            if (curPage >= (pages.length - 1))
            {
                Alert.show("已经是最后一页！");
                return;
            };
            curPage++;					// 点击下一页后，相应的当前页+1
            clickCount++;				// 点击次数累加，这个参数尽量少点击为好
            setPageControlerStatus();	// 维护好按钮状态
            timer.stop();				//  将定时器停止掉
            studyTime = (studyTime + pageTime);	// 计算学习时间
            pageTime = 0;				// 将页面时间清空
            callSaveTime(SAVE_PAGE);	// 调用此方法，将学时上传
            loadPage();					// 完成后加载新页
        }
		
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		// 点击上一页按钮事件
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
        public function onPagePrev(_arg_1:Event):void
        {
            switch (sysStatus)
            {
                case STATUS_NOMOVIE:
                case STATUS_LOADING:
                case STATUS_TESTING:
                case STATUS_CHECKING:
                    return;
                case STATUS_PAUSED:
                case STATUS_PLAYING:
                case STATUS_PLAYDONE:
            };
            if (isNaN(curPage))
            {
                return;
            };
            if (curPage <= 0)
            {
                Alert.show("已经是第一页！");
                return;
            };
            curPage--;			// 上一页
            clickCount++;		// 按钮点击次数累加
            setPageControlerStatus();
            timer.stop();
            studyTime = (studyTime + pageTime);	// ?
            pageTime = 0;
            callSaveTime(SAVE_PAGE);
            loadPage();
        }

        public function onSaveTime(_arg_1:MouseEvent):void
        {
            callSaveTime(SAVE_BUTTON);
        }

        public function onSwapScreen(_arg_1:MouseEvent):void
        {
            if (stage.displayState == StageDisplayState.NORMAL)
            {
                stage.displayState = StageDisplayState.FULL_SCREEN;
            } else
            {
                stage.displayState = StageDisplayState.NORMAL;
            };
        }

        public function onTimer(_arg_1:TimerEvent):void
        {
            if (isInstructions)
            {
                return;
            };
            if (sysStatus != STATUS_PLAYING)
            {
                return;
            };
            pageTime++;
            if (pageTime >= maxTime)//如果页面时间超过了最大时间，定时器就停止了
            {
                timer.stop();
            };
            if (bTest)
            {
                testTime++;
                testTime = (testTime % testTimeInterval);
                if (testTime == firstTestTime)
                {
                    doTest();
                };
            };
        }

		// 读xml数据
        public function onXmlLoaded(_arg_1:Event):void
        {
            var _local_2:URLLoader = URLLoader(_arg_1.target);
            eduMallXml = new XML(_local_2.data);
            readConfig();
            readPages();
            loadPage();
            setPageControlerStatus();
            mc_noInit.visible = false;
        }

        public function changeContent(_arg_1:String, _arg_2:String):void
        {
            switch (sysStatus)
            {
                case STATUS_LOADING:
                    Alert.show("当前有一个载入过程正在进行，请稍后重试。\n如果长时间不能完成载入，请刷新页面后，更换教学内容。");
                    return;
                case STATUS_TESTING:
                    Alert.show("请完成测试。");
                    return;
                case STATUS_CHECKING:
                    Alert.show("请完成人脸识别验证。");
                    return;
                case STATUS_PLAYING:
                case STATUS_PAUSED:
                case STATUS_PLAYDONE:
                case STATUS_NOMOVIE:
                    strBasePath = _arg_1;
                    if (strBasePath.charAt((strBasePath.length - 1)) != "/")
                    {
                        strBasePath = (strBasePath + "/");
                    };
                    curPage = (Number(_arg_2) - 1);
                    if (isNaN(curPage))
                    {
                        curPage = 0;
                    } else
                    {
                        if (curPage < 0)
                        {
                            curPage = 0;
                        };
                    };
                    sysStatus = STATUS_LOADING;
                    mc_contnetControler.swapContentStatus();
                    setPageControlerStatus();
                    loadXml();
                    return;
            };
        }

        public function faceCheckDone():void
        {
            sysStatus = STATUS_PLAYING;
            setPageControlerStatus();
            btn_saveTime.enabled = true;
            btn_screenSwap.enabled = true;
            mc_slidebarContent.enabled = true;
            mc_slidebarContent.j_mc.enabled = true;
            mc_contnetControler.swapContentStatus();
            mc_soundControler.endTest();
            curMovie.play();
        }

        public function faceCheckStart():void
        {
            sysStatus = STATUS_CHECKING;
            setPageControlerStatus();
            btn_saveTime.enabled = false;
            btn_screenSwap.enabled = false;
            mc_slidebarContent.enabled = false;
            mc_slidebarContent.j_mc.enabled = false;
            mc_contnetControler.swapContentStatus();
            mc_soundControler.startTest();
            curMovie.stop();
        }

        public function recoveWindow():void
        {
            stage.displayState = StageDisplayState.NORMAL;
        }

        public function replay():void
        {
            sysStatus = STATUS_PLAYING;
            setPageControlerStatus();
            btn_saveTime.enabled = true;
            btn_screenSwap.enabled = true;
            mc_slidebarContent.enabled = true;
            mc_slidebarContent.j_mc.enabled = true;
            mc_contnetControler.swapContentStatus();
            mc_soundControler.endTest();
            curMovie.play();
        }

        public function setHasTest(_arg_1:String):void
        {
            if (_arg_1.toLowerCase() == "n")
            {
                bTest = false;
            } else
            {
                bTest = true;
            };
        }

        public function setFirstTime(_arg_1:Number):void
        {
            firstTestTime = _arg_1;
            if (testTimeInterval <= firstTestTime)
            {
                testTimeInterval = TEST_DEFAULT_INTERVAL_TIME;
                firstTestTime = TEST_DEFAULT_FIRST_TIME;
            };
        }

        public function setIntervalTime(_arg_1:Number):void
        {
            testTimeInterval = _arg_1;
            if (testTimeInterval <= firstTestTime)
            {
                testTimeInterval = TEST_DEFAULT_INTERVAL_TIME;
                firstTestTime = TEST_DEFAULT_FIRST_TIME;
            };
        }

        public function myAlert(_arg_1:String):void
        {
            Alert.show(("来自JS：" + _arg_1));
        }

        public function onTest(_arg_1:MouseEvent):void
        {
        }

        function frame1()
        {
            STATUS_NOMOVIE = "nomovie";
            STATUS_LOADING = "loading";
            STATUS_PLAYING = "playing";
            STATUS_PAUSED = "paused";
            STATUS_TESTING = "testing";
            STATUS_CHECKING = "checking";
            STATUS_PLAYDONE = "playdone";
            TEST_DEFAULT_FIRST_TIME = 14;
            TEST_DEFAULT_INTERVAL_TIME = 180;
            SAVE_BUTTON = "button";
            SAVE_PAGE = "page";
            SAVE_JAVASCRIPT = "javascript";
            mCfgMaxTime = 0;
            mCfgMaxTimeMultiple = 0;
            mCfgRightMenuName = "";
            mCfgRightMenuUrl = "";
            totalPage = 0;
            stop();
            Security.allowDomain("*");
            Security.allowInsecureDomain("*");
            Alert.init(stage);
            init();
            btn_test.addEventListener(MouseEvent.CLICK, onTest);
        }


    }
}//package jxjy_AS3_V01_01_fla