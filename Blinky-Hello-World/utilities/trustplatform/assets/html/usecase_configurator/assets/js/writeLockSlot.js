"use strict";

function x08UpdateWriteLock(xmlObj){
    var xmlDoc = xmlObj;
    var slotLockId = "";
    var isChecked;
    var i = 0;
    var keyConfig = "";
    var slotConfig = "";

    for(i=0; i<16; i++){
        slotLockId = "slotlock" + i.toString();
        try {
            isChecked = document.getElementById(slotLockId);
            if(isChecked != null){
                isChecked = isChecked.checked;
                //console.log(isChecked);
            }
            else{
                isChecked = false;
            }
        }
        catch(e){
            console.error(e);
        }
        finally{
            if(isChecked){
                var slotConfig = xmlDoc.getElementsByTagName("SlotConfiguration")[i].textContent;
                slotConfig = unprettifyHexData(slotConfig);
                slotConfig = slotConfig.split('');
                slotConfig[2] = "8";
                slotConfig = slotConfig.join('');
                var slotConfigP1 = slotConfig.slice(0, 2);
                var slotConfigP2 = slotConfig.slice(2, 4);
                slotConfig = slotConfigP1 + " " + slotConfigP2;

                xmlDoc.getElementsByTagName("SlotConfiguration")[i].textContent = slotConfig;
            }
        }
    }

    return xmlDoc;
}