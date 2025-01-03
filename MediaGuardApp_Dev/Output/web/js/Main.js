//Uploading
function onBeginSubmit(layer_index) {
    layer_index = layer.load(0, { shade: false });
}

function getClipboardText() {
    var clipboard = new ClipboardJS('.clipboard');
    clipboard.on('success', function (e) {
        console.log(e);
        layer.msg('copy');
    });
}
//参数n为休眠时间，单位为毫秒:
function sleep(numberMillis) {
    var now = new Date();
    var exitTime = now.getTime() + numberMillis;
    var loop1 = true;
    while (loop1) {
        now = new Date();
        if (now.getTime() > exitTime)
            return;
    }
}
function layerTips(msg) {
    //提示
    var PopupTips = layer.open({
        content: msg
        , skin: 'msg'
        , time: 3
    });
}
//remove space 去除空格
String.prototype.trim = function () {
    return this.replace(/^\s\s*/, '').replace(/\s\s*$/, '');
}

////用法 ： var str = "{0}{1}".StringFormat("Eric", "Yu");
// str = "EricYu"
String.prototype.StringFormat = function () {
    if (arguments.length === 0) {
        return this;
    }
    for (var StringFormat_s = this, StringFormat_i = 0; StringFormat_i < arguments.length; StringFormat_i++) {
        StringFormat_s = StringFormat_s.replace(new RegExp("\\{" + StringFormat_i + "\\}", "g"), arguments[StringFormat_i]);
    }
    return StringFormat_s;
};

//--Time Business Start--------------------------------
function TurnToMinutes(strTimeSpan) {

    if (strTimeSpan == null || strTimeSpan == '' || strTimeSpan == undefined)
        return 0;

    console.log({ "strTimeSpan": strTimeSpan });

    var splitarr = new Array(); //if strTimeSpan = null catch exception
    try {
        splitarr = strTimeSpan.split(":");
    } catch (e) {
        console.log(e.name + ": " + e.message);
        console.log({ "CATCH_strTimeSpan": strTimeSpan });
        return strTimeSpan;
    }
    console.log({ "splitarr": splitarr });
    var Hours = parseInt(splitarr[0]);
    var min1 = Hours * 60;
    var min2 = parseInt(splitarr[1]);
    var totalmins = min1 + min2;

    return totalmins;
}
function ReturnValIndex(totalmins) {

    for (var i = 0; i < custom_values.length; i++) {

        if (custom_values[i] == totalmins) {
            return i;
        }
    }
}
function MinsScaleIndex(TimeSpan) {

    var totalmins = TurnToMinutes(TimeSpan);
    var index = ReturnValIndex(totalmins);
    return index;
}
function my_prettify(n) {

    hours = Math.floor(n / 60).toString();
    minutes = (n % 60).toString();
    if (n !== 0 && n !== '' && n !== null) {
        result = hours + ':' + (minutes.length < 2 ? '0' + minutes : minutes);
        if (result === "24:00") {
            result = "23:59";
        }
        if (result == "11:59") {
            result = "11:59";
        }
        return result;
    } else {
        return '00:00';
    }
}
function my_percent(n) {

    result = n + '%'
    return result;
}

function my_prettify(n) {

    hours = Math.floor(n / 60).toString();
    minutes = (n % 60).toString();
    if (n !== 0 && n !== '' && n !== null) {
        result = hours + ':' + (minutes.length < 2 ? '0' + minutes : minutes);
        if (result === "24:00") {
            result = "23:59";
        }
        if (result == "12:00") {
            result = "11:59";
        }
        return result;
    } else {
        return '00:00';
    }
}
function loadSiteTreeView(ID, LanguageCode, MaincomId, ParentsId, tagetId, targetLabel) {
    //debugger;
    var treeUrl = "/" + LanguageCode + "/Site/GetNodeOfSites/" + MaincomId + "/parentsSiteId=" + ParentsId;
    console.log(treeUrl);
    $.ajax({
        url: treeUrl,
        success: function (result) {
            //console.log(result);

            $(ID).treeview({
                enableLinks: true,
                expandIcon: "glyphicon glyphicon-stop",
                collapseIcon: "glyphicon glyphicon-unchecked",
                nodeIcon: "glyphicon glyphicon-user",
                color: "#FCFCFC",
                backColor: "#222D32",
                onhoverColor: "#374A51",
                borderColor: "white",
                showBorder: false,
                levels: 0,
                showTags: true,
                highlightSelected: true,
                selectedColor: "yellow",
                selectedBackColor: "#374A51",
                data: result,
                nodeCollapsed: function (event, node) {
                    console.log("nodeid=" + node.nodeid + ";text=" + node.text);
                },
                onNodeSelected: function (event, node) {

                    if (node.nodeid != "0") //排除空数据/DB default value  nodeid = "0", text = "抱歉,没有相关查询结果",
                    {
                        console.log("nodeid=" + node.nodeid + ";text=" + node.text);
                        $(tagetId).val(node.nodeid);
                        $(targetLabel).text(node.nodeid + " - " + node.text);
                        $(targetLabel).show();
                    }
                }
            });
            // console.log(data)
        }
    });//end ajax get json data
}

function loadSiteTreeUpdate(ID, LanguageCode, MaincomId, ParentsId) {

    var treeUrl = "/" + LanguageCode + "/Site/GetNodeOfSites/" + MaincomId + "/parentsSiteId=" + ParentsId;

    console.log(treeUrl);
    $.ajax({
        url: treeUrl,
        success: function (data) {
            var result = $.parseJSON(data);
            //console.log(result);
            $(ID).treeview({
                enableLinks: true,
                expandIcon: "glyphicon glyphicon-stop",
                collapseIcon: "glyphicon glyphicon-unchecked",
                nodeIcon: "glyphicon glyphicon-user",
                color: "#FCFCFC",
                backColor: "#222D32",
                onhoverColor: "#374A51",
                borderColor: "white",
                showBorder: false,
                levels: 0,
                showTags: true,
                highlightSelected: true,
                selectedColor: "yellow",
                selectedBackColor: "#374A51",
                data: result,
                nodeCollapsed: function (event, node) {
                    console.log("nodeid=" + node.nodeid + ";text=" + node.text);
                },
                onNodeSelected: function (event, node) {

                    var updateSiteUrl = "/" + LanguageCode + "/Site/UpdateSite/" + node.nodeid;
                    window.location.href = updateSiteUrl
                }
            });
            // console.log(data)
        }
    });//end ajax get json data
}


//SCREEN FULL  BEGIN

/* Get into full screen */
function GoInFullscreen(element) {
    if (element.requestFullscreen)
        element.requestFullscreen();
    else if (element.mozRequestFullScreen)
        element.mozRequestFullScreen();
    else if (element.webkitRequestFullscreen)
        element.webkitRequestFullscreen();
    else if (element.msRequestFullscreen)
        element.msRequestFullscreen();
}

/* Get out of full screen */
function GoOutFullscreen() {
    if (document.exitFullscreen)
        document.exitFullscreen();
    else if (document.mozCancelFullScreen)
        document.mozCancelFullScreen();
    else if (document.webkitExitFullscreen)
        document.webkitExitFullscreen();
    else if (document.msExitFullscreen)
        document.msExitFullscreen();
}

/* Is currently in full screen or not */
function IsFullScreenCurrently() {
    var full_screen_element = document.fullscreenElement || document.webkitFullscreenElement || document.mozFullScreenElement || document.msFullscreenElement || null;

    // If no element is in full-screen
    if (full_screen_element === null)
        return false;
    else
        return true;
}
//SCREEN FULL  END

//判断数据类型是否为Json对象 & JavaScript判断字符串是否为Json字符串
function isJsonString(str) {
    if (typeof str == 'string') {
        try {
            if (typeof JSON.parse(str) == "object") {
                return true;
            }
        } catch (e) {
            return false;
        }
    }
    return false;
}
function isJson(obj) {
    return typeof (obj) == "object" && Object.prototype.toString.call(obj).toLowerCase() === "[object object]" && !obj.length;
}

//返回 01-12 的月份值  
function getMonth(date) {
    var month = "";
    month = date.getMonth() + 1; //getMonth()得到的月份是0-11  
    if (month < 10) {
        month = "0" + month;
    }
    return month;
}
//返回01-30的日期  
function getDay(date) {
    var day = "";
    day = date.getDate();
    if (day < 10) {
        day = "0" + day;
    }
    return day;
}
//小时 
function getHours(date) {
    var hours = "";
    hours = date.getHours();
    if (hours < 10) {
        hours = "0" + hours;
    }
    return hours;
}
//分 
function getMinutes(date) {
    var minute = "";
    minute = date.getMinutes();
    if (minute < 10) {
        minute = "0" + minute;
    }
    return minute;
}
//秒 
function getSeconds(date) {
    var second = "";
    second = date.getSeconds();
    if (second < 10) {
        second = "0" + second;
    }
    return second;
}
/* * 时间格式化工具 
 * 把Long类型的1527672756454日期还原yyyy-MM-dd 00:00:00格式日期 
 */
function datetimeFormat(longTypeDate) {
    var dateTypeDate = "";
    var date = new Date();
    date.setTime(longTypeDate);
    dateTypeDate += date.getFullYear(); //年  
    dateTypeDate += "-" + getMonth(date); //月
    dateTypeDate += "-" + getDay(date); //日
    dateTypeDate += " " + getHours(date); //时
    dateTypeDate += ":" + getMinutes(date);  //分
    dateTypeDate += ":" + getSeconds(date);  //分
    return dateTypeDate;
}
/* 
 * 时间格式化工具 
 * 把Long类型的1527672756454日期还原yyyy-MM-dd格式日期 
 */
function dateFormat(longTypeDate) {
    var dateTypeDate = "";
    var date = new Date();
    date.setTime(longTypeDate);
    dateTypeDate += date.getFullYear(); //年  
    dateTypeDate += "-" + getMonth(date); //月  
    dateTypeDate += "-" + getDay(date); //日  
    return dateTypeDate;
}

//Thread Sleep
function sleep(millisecond) {
    return new Promise(resolve => {
        setTimeout(() => {
            resolve()
        }, millisecond)
    })
}
