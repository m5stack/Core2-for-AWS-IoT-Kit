"use strict";

var slotValidateDict = {
    0: "none",
    1: "none",
    2: "none",
    3: "none",
    4: "none",
    5: "none",
    6: "none",
    7: "none",
    8: "none",
    9: "none",
    10: "none",
    11: "none",
    12: "none",
    13: "none",
    14: "none",
    15: "none",
    16: "none"
};

function validateSlotOpt(){
    var generateXml = true;
    for(var i=0; i<16; i++){
        if(slotValidateDict[i] == "invalid"){
            //console.log(i);
            alert("Enter valid data in slot " + i);
            generateXml = false;
        }
    }
    return generateXml;
}