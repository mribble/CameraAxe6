var request;
window.onload = updateState;

function updateState() {
    setTimeout(updateState, 1000);
    document.getElementById("id1000").innerHTML = "Done";
    server = "status/99";
    request = new XMLHttpRequest();
    request.onreadystatechange = updateStatus;
    request.open("GET", server, true);
    request.send();
}

function updateStatus() {
    if ((request.readyState == 4) && (request.status == 200))
    {
        result = request.responseText;
        document.getElementById("id1000").innerHTML = result;
    }
}

function sendButton(id){
    document.getElementById("id1000").innerHTML = "ButtonClick";
    server = "Button/" + id;
    request = new XMLHttpRequest();
    request.open("GET", server, true);
    request.send();
}

function sendInt(id, value){
    valNum = "valueInt" + id;
    document.getElementById(valNum).innerHTML=value;
    document.getElementById("id1000").innerHTML = "Processing Int";
    server = "int/" + id + "/" + value;
    request = new XMLHttpRequest();
    request.onreadystatechange = updateAsyncInt;
    request.open("GET", server, true);
    request.send();
}
function updateAsyncInt(){
    if ((request.readyState == 4) && (request.status == 200))
    {
        result = request.responseText;
        singleset = result.split(",");
        pinType = singleset[0];
        pinNum = singleset[1];
        pinVal = parseInt(singleset[2]);
        intNum = "int" + PinNum;
        valNum = "valueInt" + PinNum;
        document.getElementById(intNum).value = pinVal;
        document.getElementById(valNum).innerHTML = pinVal;
        document.getElementById("id1000").innerHTML = result;
    }
}