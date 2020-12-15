"use strict";

var slotConfigLockable = {
    2: "85 20",
    3: "85 20",
    4: "85 20",
    5: "8F 46",
    6: "8F 0F",
    8: "0F 0F",
    10: "0F 0F",
    11: "0F 0F",
    12: "0F 0F",
    13: "0F 0F",
    15: "0F 0F"
};

var keyConfigLockable = {
    2: "73 00",
    3: "73 00",
    4: "73 00",
    5: "38 00",
    6: "7C 00",
    8: "3C 00",
    10: "3C 00",
    11: "30 00",
    12: "3C 00",
    13: "30 00",
    15: "30 00"
};

var slotConfigPermanent = {
    2: "85 A0",
    3: "85 A0",
    4: "85 A0",
    5: "8F C6",
    6: "0F 8F",
    8: "0F 8F",
    10: "0F 8F",
    11: "0F 8F",
    12: "0F 8F",
    13: "0F 8F",
    15: "0F 8F"
};

var keyConfigPermanent = {
    2: "53 00",
    3: "53 00",
    4: "53 00",
    5: "18 00",
    6: "5C 00",
    8: "1C 00",
    10: "1C 00",
    11: "10 00",
    12: "1C 00",
    13: "10 00",
    15: "10 00"
};

function getKeyConfigLockable(slotNumber, isLockable){
    var keyConfig = '';
    if(isLockable == true){
        keyConfig = keyConfigLockable[slotNumber];
    }
    else if(isLockable == false){
        keyConfig = keyConfigPermanent[slotNumber];
    }
    return keyConfig;
}

function getSlotConfigLockable(slotNumber, isLockable){
    var slotConfig ='';
    if(isLockable == true){
        slotConfig = slotConfigLockable[slotNumber];
    }
    else if(isLockable == false){
        slotConfig = slotConfigPermanent[slotNumber];
    }
    return slotConfig;
}

function x08UpdateLockable(xmlObj){
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
                console.log(isChecked);
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
                xmlDoc.getElementsByTagName("SlotConfiguration")[i].textContent = getSlotConfigLockable(i, isChecked);
                xmlDoc.getElementsByTagName("KeyConfiguration")[i].textContent = getKeyConfigLockable(i, isChecked); 
            }
        }        
    }

    return xmlDoc;
}

//x08UpdateLockable(3);