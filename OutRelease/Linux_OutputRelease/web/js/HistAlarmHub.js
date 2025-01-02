"use strict";

var connection = new signalR.HubConnectionBuilder()
    .withUrl("/HistAlarmHub")
    .withAutomaticReconnect([2000, 4000, 6000, 8000, 10000, 12000, 14000, 16000, 16000, 16000, 16000, 16000, 16000, 16000, 16000, 16000, 16000, 16000, 16000, 16000, 16000, 16000, 16000, 16000, 16000, 16000, 16000])
    .build();
connection.onclose(function (error) {
    document.getElementById("connectionState").textContent = "Disconnected. Please refrech web page...";
    layerTips("Disconnected. Please refrech web page...");
});

connection.onreconnecting(function (error) {

    document.getElementById("connectionState").textContent = "Reconnecting...";
    layerTips("Reconnecting...ok...");
});
connection.onreconnected(function (connectionId) {
    document.getElementById("connectionState").textContent = "Connected";
    layerTips("Connected  ok...");
});

connection.start().then(function () {
    document.getElementById("connectionState").textContent = "Connected";
}).catch(function (err) {
    return alert("請重新整理頁面")
});

connection.on("HistAlarmHub_NewRec", GetAlarmItemDetails);

function GetAlarmItemDetails(newRecord) {
    console.log("GetAlarmItemDetails:");
    console.log(newRecord);
    layerTips("GetAlarmItemDetails");
    console.log(newRecord);

    const tbody = document.querySelector("table.records tbody")
    let tr = document.createElement('tr')
    layerTips(newRecord.histAlarmId);
    var histAlarmId = newRecord.histAlarmId;
    renderNewItem(histAlarmId);
}
function renderNewItem(histAlarmId) {

    if (language == undefined || language == "") { //default value
        language = 'zh-HK';
    }
    var renderItemUrl = "/" + language + "/HistAlarm/ItemDetails/" + histAlarmId;

    console.log({ "renderItemUrl": renderItemUrl });
    $.ajax({
        url: renderItemUrl,
        type: "GET",
        error: function (data) {
            console.log("FUNC::Alarm RenderNewItem ::>ERROR");
            console.log(data);
        },
        success: function (html) {

            console.log("RESPONSE：-----------------------------------------------------------------------------");
            console.log({ "RESPONSE": html });
            //const tbody = document.querySelector("#AlarmListTbody");
            //let tr = document.createElement('tr');
            //tr.innerHTML(html);
            //tr.classList.add("slideIn");
            //tbody.prepend(tr);
            var x = document.getElementById('AlarmListTbody').insertRow(0);
            x.innerHTML = html;

            ExcTableRowRefreshEffect(x);
        }
    });
}

//執行TD Refresh效果
async function ExcTableRowRefreshEffect(tr) {
    //debugger;
    const start = new Date().getTime();
    console.log("ExcTableRowRefreshEffect Start", start);

    for (let i = 0; i < 3; i++) {

        console.log("hist_alarm_tr_border_style1");

        $(tr).addClass("hist_alarm_tr_border_style1");
        await sleep(2000);
        $(tr).removeClass("hist_alarm_tr_border_style1");

        $(tr).addClass("hist_alarm_tr_border_style2");
        await sleep(2000);
        $(tr).removeClass("hist_alarm_tr_border_style2");

        $(tr).addClass("hist_alarm_tr_border_style3");
        await sleep(2000);
        $(tr).removeClass("hist_alarm_tr_border_style3");
    }
}


