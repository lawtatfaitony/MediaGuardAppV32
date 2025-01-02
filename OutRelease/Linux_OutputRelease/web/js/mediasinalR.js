"use strict";

var connectionMedia = new signalR.HubConnectionBuilder().withUrl("/ServerHub").build();
 
connectionMedia.on("ReceiveMediaMessage", function (user, message) {
    var sp = document.getElementById("ReceiveMediaMessageSpan");
    sp.textContent = `${user} ${message}`;
    console.log(`${user} says ${message}`);
});
 
 