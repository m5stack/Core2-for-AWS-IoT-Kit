"use strict";

function validateManIdText(){
    var manIdHexString;
    var manIdHexStringLength;
    var alertMessage = "";
    var isManIdValid = true;
    manIdHexString = document.getElementById("manIdHexId").value;

    if(manIdHexString == "")
        manIdHexString = "0x01"

    if(manIdHexString != null){
        var formatedString = manIdHexString.replaceAll(" ", "").replaceAll("\n", "").replaceAll("\r", "").replaceAll("\t", "").replaceAll("0x", "").replaceAll(",", "").toUpperCase();

        manIdHexStringLength = formatedString.length;

        if(!is_hex(formatedString)){
            alertMessage = alertMessage + "Invalid MAN-ID, Non-hexadecimal characters found \n\n"
        }

        if(manIdHexStringLength == 1 || manIdHexStringLength > 2){
            alertMessage = alertMessage + "Invalid MAN-ID, MAN-ID is a 1 byte hexadecimal number \n"
        }

        if(alertMessage != ""){
            isManIdValid = false;
            alert(alertMessage);
        }
    }

    return {
        status: isManIdValid,
        manId: formatedString,
    };
}