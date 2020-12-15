var formNameMain = slotdataform;

var device_org_name_len = 24;
var device_common_name_len = 20;
var signer_org_name_len = 24;
var signer_common_name_len = 33;
var root_org_name_len = 24;
var root_common_name_len = 33;

var keyLoadConfig = {
    0: "noLoad",
    1: "noLoad",
    2: "noLoad",
    3: "noLoad",
    4: "noLoad",
    5: "load",
    6: "load",
    7: "noLoad",
    8: "load",
    9: "load",
    10: "cert",
    11: "noLoad",
    12: "cert",
    13: "load",
    14: "load",
    15: "load"
};

var slotsize = {
    0: "32",
    1: "32",
    2: "32",
    3: "32",
    4: "32",
    5: "32",
    6: "32",
    7: "32",
    8: "416",
    9: "64",
    10: "64",
    11: "64",
    12: "64",
    13: "64",
    14: "64",
    15: "64",
    16: "64"
}

var tflexSlotType = {
    0: "private",
    1: "private",
    2: "private",
    3: "private",
    4: "private",
    5: "secret",
    6: "secret",
    7: "general",
    8: "general",
    9: "secret",
    10: "cert",
    11: "cert",
    12: "cert",
    13: "public",
    14: "public",
    15: "public"
}

var unprettifyDict = {
    0: " ",
    1: "\n",
    2: "\r",
    3: "\t",
    4: "0x",
    5: "0X",
    6: ",",
}

function unprettifyHexData(Hexstring){
    return Hexstring.replaceAll(" ","").replaceAll("\n", "").replaceAll("\r", "").replaceAll("\t", "").replaceAll("0x", "").replaceAll(",","").toUpperCase();
}

function tflex608SlotSize(slotNumber){
    var size = 0;

    if(slotNumber < 8){
        size = 36;
    }
    else if(slotNumber == 8){
        size = 416;
    }
    else if(slotNumber > 8){
        size = 72;
    }
    else{
        size = null;
    }

    return size;
}

function gererateXML() {
    var parser = new DOMParser();
    var xmlStr = document.getElementById('tFlexECC608').innerHTML;
    var xmlDoc = parser.parseFromString(xmlStr, "text/xml");
    processXML(xmlDoc);
}

function setRadioValue(form, radioName, radioSelect){
    var radios = form.elements[radioName];

    for (var i = 0; i < radios.length; i++){
        if (radios[i].value == radioSelect){
            radios[i].checked = true;
        }
        else{
            radios[i].checked = false;
        }
    }

    return null;
}

function getFormRadioValue(form, name){
    var radios = form.elements[name];

    for (var i = 0; i < radios.length; i++){
        if (radios[i].checked == true){
            return radios[i].value;
        }
    }

    return null;
}

function getFormDataSlot(form, slotNumber){
    var radioName = "slot" + slotNumber + "dataopt";
    var status;
    var slotData = null;

    if(null != (status = getFormRadioValue(form, radioName))){
        if(status == "unused"){
            slotData = null;
        }
        else if(status == "hexdata"){
            slotData = document.getElementById(radioName + "id").value;
        }
        else if(status == "pemdata"){
            slotData = document.getElementById(radioName + "id").value;
        }
        else{
            console.error("Unknown radio value");
            slotData = null;
        }
    }
    else{
        console.error("Radio Value fetch error")
    }
    return slotData;
}

function getDataFromSlot(radioName){
    var status;
    var slotData = null;

    if(null != (status = getFormRadioValue(formNameMain, radioName))){
        if(status == "unused"){
            //Do nothing?
            slotData = null;
        }
        else if(status == "hexdata"){
            slotData = document.getElementById(radioName + "id").value;
        }
        else if(status == "pemdata"){
            slotData = null;
        }
        else{
            console.error("Unknown radio value");
            slotData = null;
        }
    }
    else{
        console.error("Radio Value fetch error")
    }
    return slotData;
}

function processXML(xmlObj) {
    // Request finished.
    var xmlDoc = xmlObj;
    // If secret slots are not used then convert it to random slots
    var secretSlots = [5, 6, 9];
    var XMLContainsSecrets = false;

    var isManIdValid = true;
    // Initilize JsZip
    jsZipInit();

    //xmlDoc = x08UpdateLockable(xmlDoc);

    // Updating the interface type in XML object I2C/SWI
    var devInterface = getFormRadioValue(formNameMain, "devIface");

    if (devInterface == 'i2c') {
        xmlDoc.getElementsByTagName("I2CEnable")[0].textContent = '01'; // I2C mode
        xmlDoc.getElementsByTagName("I2CAddress")[0].textContent = '6C'; // I2C Address
    }
    else if (devInterface == 'swi'){
        xmlDoc.getElementsByTagName("I2CEnable")[0].textContent = '00'; // SWI mode
        //xmlDoc.getElementsByTagName("I2CAddress")[0].textContent = '00'; // GPIO Disabled. SCL pin is unused, should be tied low on the board.
        xmlDoc.getElementsByTagName("I2CAddress")[0].textContent = '03'; // GPIO Enabled as Output. SCL may be driven HIGH or LOW. Default state is set to LOW(On powerup).
    }
    else {
        console.error("Invalid interface value")
    }

    if("custCert" == getFormRadioValue(formNameMain, "slot12certopt")){
        xmlDoc.getElementsByTagName("CompressedCerts")[0].getElementsByTagName("CompressedCert")[1].getElementsByTagName("CAPublicKey")[0].textContent = "\n\t\t" + prettyPrintHex(unprettifyHexData(getFormDataSlot(formNameMain, 16)), 32).replaceAll("\n", "\n\t\t");
    }

    if(!is_sn_selected_as_CN())
    {
        xmlDoc.getElementsByTagName("CompressedCerts")[0].getElementsByTagName("CompressedCert")[0].getElementsByTagName("Element")[5].remove();
        xmlDoc.getElementsByTagName("CompressedCerts")[0].getElementsByTagName("CompressedCert")[0].getElementsByTagName("Element")[4].remove();
    }

    // Update the slots with user's data.
    for (var i = 0; i < 16; i++) {
        if (keyLoadConfig[i] == "noLoad") {
        }
        else if (keyLoadConfig[i] == "load") {
            var data = getFormDataSlot(formNameMain, i);
            if(data != null){
                xmlDoc = tflexUpdataXmlSlot(xmlObj, i, data);
            }
        }
        else if (keyLoadConfig[i] == "cert"){
            var radioName = "slot" + i + "certopt";
            // Getting value from selection button
            var certOptValue = getFormRadioValue(formNameMain, radioName);

            if (certOptValue == "MCHPCert") {
                // Form the .c and .h files to download
                if(i == 10){
                    var deviceCertH = document.getElementById('tflxtls_cust_cert_def_device_h').innerHTML;
                    jsZipAddFile("tflxtls_cust_cert_def_device.h", deviceCertH);

                    var deviceCertC =
                        document.getElementById('tflxtls_cust_cert_def_device_c_p1').innerHTML +
                        document.getElementById('tflxtls_cust_cert_def_device_c_hex').innerHTML +
                        document.getElementById('tflxtls_cust_cert_def_device_c_p2').innerHTML +
                        document.getElementById('tflxtls_cust_cert_def_device_c_p3').innerHTML +
                        document.getElementById('tflxtls_cust_cert_def_device_c_p5').innerHTML;
                    jsZipAddFile("tflxtls_cust_cert_def_device.c", deviceCertC);
                }
                else if(i == 12){
                    var signerCertH = document.getElementById('tflxtls_cust_cert_def_signer_h').innerHTML;
                    jsZipAddFile("tflxtls_cust_cert_def_signer.h", signerCertH);
                    var ca_pub_key = xmlDoc.getElementsByTagName("CompressedCerts")[0].getElementsByTagName("CompressedCert")[1].getElementsByTagName("CAPublicKey")[0].textContent;
                    var signerCertC = document.getElementById('tflxtls_cust_cert_def_signer_c_p1').innerHTML + document.getElementById('tflxtls_cust_cert_def_signer_c_hex').innerHTML + document.getElementById('tflxtls_cust_cert_def_signer_c_p2').innerHTML + convertHextoChex(ca_pub_key, 32) + document.getElementById('tflxtls_cust_cert_def_signer_c_p3').innerHTML;
                    jsZipAddFile("tflxtls_cust_cert_def_signer.c", signerCertC);
                }
            }
            else if(certOptValue == "custCert"){
                var xmlslotData = xmlDoc.getElementsByTagName("DataZone")[0].getElementsByTagName("Slot")[i];
                var certMode = xmlslotData.getAttribute("Mode");
                var certCompressedCert = xmlslotData.getAttribute("CompressedCert");
                var certOrgName = document.getElementById(i + "certname").value;
                var certExpYear = document.getElementById(i + "certyear").value;

                xmlDoc = processCertData(xmlDoc, i, certCompressedCert, certOrgName, certExpYear);
            }
        }
        else {
            console.error("Config Error");
        }

        // Code to change mode secret slots to random if not used
        if(secretSlots.includes(i)){
            // Check if slot is unused.
            var radioName = "slot" + i + "dataopt";

            if(null != (status = getFormRadioValue(formNameMain, radioName))){
                if(status == "unused"){
                    xmlDoc.getElementsByTagName("DataZone")[0].getElementsByTagName("Slot")[i].setAttribute("Mode", "Random");
                }
                else{
                    XMLContainsSecrets = true;
                }
            }
        }

        //Remove <Data> tag from all Random slots
        if(xmlDoc.getElementsByTagName("DataZone")[0].getElementsByTagName("Slot")[i].getAttribute("Mode") == "Random")
        {
            xmlDoc.getElementsByTagName("DataZone")[0].getElementsByTagName("Slot")[i].remove();
            newEle = xmlDoc.createElement("Slot");
            newEle.setAttribute("Index", i);
            newEle.setAttribute("Mode", "Random");
            x = xmlDoc.getElementsByTagName("DataZone")[0]
            y = xmlDoc.getElementsByTagName("DataZone")[0].getElementsByTagName("Slot")
            x.insertBefore(newEle,y[i])

            elementBlankNode = xmlDoc.createTextNode("\r\n\t\t")
            x.insertBefore(elementBlankNode,y[i+1])
        }
    }

    var returns;
    returns = validateManIdText();
    isManIdValid = returns.status;

    if(isManIdValid == true){
        xmlDoc.getElementsByTagName("SN8")[0].textContent = returns.manId;
    }

    partNumberString = document.getElementById("partNumberId").value;
    if(partNumberString == "")
        partNumberString = "ATECC608B-MAHxx-p"
    xmlObj.getElementsByTagName("PartNumber")[0].textContent = partNumberString;

    // Process secureboot/persistant latch on slot 0
    var latchStatus;
    latchStatus = getFormRadioValue(formNameMain, "sbootLatchName")
    if (latchStatus == "enabled"){
        xmlDoc.getElementsByTagName("SecureBoot")[0].textContent = "0B F7"
        xmlDoc.getElementsByTagName("ConfigurationZone")[0].getElementsByTagName("KeyConfiguration")[0].textContent = "53 10"
    }
    else {
        //do Nothing
    }

    xmlDoc = x08UpdateWriteLock(xmlDoc);

    var useCaseValid = validateUseCaseSlots();
    var slotDataValidity = validateSlotOpt();

    if (useCaseValid == false && slotDataValidity == true && isManIdValid == true) {
        serializeXmlAndSave(xmlDoc);
        jsZipGenerateSave("TFLXTLS_Provisioning_package");

        // Warn customers about secrets in XML
        if(XMLContainsSecrets){
            alert("Secrets in the generated XML output file are not encrypted. \n\nThe file needs to be encrypted before it can be sent over to Microchip provisioning service.");
        }
    }
}

function ascii_to_hexa(str)
{
	var arr1 = [];
	for (var n = 0, l = str.length; n < l; n ++)
     {
		var hex = Number(str.charCodeAt(n)).toString(16);
		arr1.push(hex);
	 }
	return arr1.join('');
}

function hex_to_ascii(str1)
 {
	var hex  = str1.toString();
	var str = '';
	for (var n = 0; n < hex.length; n += 2) {
		str += String.fromCharCode(parseInt(hex.substr(n, 2), 16));
	}
	return str;
 }

function padString(string, len){
    var initLen = string.length;

    if(string.length < len){
        for(var y = 0; y < (len - initLen); y++){
            string += " ";
        }
    }
    else if(string.length > len){
        console.error("string padding error, invalid length");
        return null;
    }

    return string;
}

function processCertData(xmlObj, slotNumber, compressedCert, orgName, validyears){
    var xmlDoc = xmlObj;

    // Find the compressed certificate template in XML
    var allCerts = xmlDoc.getElementsByTagName("CompressedCerts")[0];
    // Getting no of certs attribute
    var noOfCerts = allCerts.getAttribute("CompressedCerts");
    var certlist = allCerts.getElementsByTagName("CompressedCert");
    var template;

    // Loop through and find the right the correct compressedCert template
    for (var i = 0; i < noOfCerts; i++){
        template = certlist[i];
        // Should have the correct template by end of the loop
        if(template.getAttribute("Index") == compressedCert){
            break;
        }
        if(i == (noOfCerts - 1)){
            console.error("Certificate template not found")
        }
    }


    var templateData = template.getElementsByTagName("TemplateData")[0];
    templateDataHex = unprettifyHexData(templateData.childNodes[0].textContent);
    var updatedCert;

    if(slotNumber == 10){
        var issuerOrgName = document.getElementById("12certname").value;
        var issuerCommonName = document.getElementById("12certcommonname").value;

        var userOrgName = document.getElementById("10certname").value;
        var userCommonName = document.getElementById("10certcommonname").value;

        updatedCert = templateDataHex.slice(0, 56*2)
            + unprettifyHexData(ascii_to_hexa(padString(issuerOrgName, signer_org_name_len)))
            + templateDataHex.slice(80*2, 91*2)
            + unprettifyHexData(ascii_to_hexa(padString(issuerCommonName, signer_common_name_len)))
            + templateDataHex.slice(124*2, 171*2)
            + unprettifyHexData(ascii_to_hexa(padString(userOrgName, device_org_name_len)))
            + templateDataHex.slice(195*2, 206*2)
            + unprettifyHexData(ascii_to_hexa(padString(userCommonName, device_common_name_len)))
            + templateDataHex.slice(226*2);
    }
    else if(slotNumber == 12){
        var issuerOrgName = document.getElementById("16certname").value;
        var issuerCommonName = document.getElementById("16certcommonname").value;

        var userOrgName = document.getElementById("12certname").value;
        var userCommonName = document.getElementById("12certcommonname").value;

        updatedCert = templateDataHex.slice(0, 56*2)
            + unprettifyHexData(ascii_to_hexa(padString(issuerOrgName, root_org_name_len)))
            + templateDataHex.slice(80*2, 91*2)
            + unprettifyHexData(ascii_to_hexa(padString(issuerCommonName, root_common_name_len)))
            + templateDataHex.slice(124*2, 171*2)
            + unprettifyHexData(ascii_to_hexa(padString(userOrgName, signer_org_name_len)))
            + templateDataHex.slice(195*2, 206*2)
            + unprettifyHexData(ascii_to_hexa(padString(userCommonName, signer_common_name_len)))
            + templateDataHex.slice(239*2);
    }

    //var name = padString(orgName, 24);
    // Storing the orgName inside the XMLDOC object
    //var updatedCert = templateDataHex.replaceAll("4d6963726f6368697020546563686e6f6c6f677920496e63".toUpperCase(), unprettifyHexData(ascii_to_hexa(name)));

    // Updating the valid years attribute

    if(parseInt(validyears) == 0){
        // Need to replace the expiry year to 23:59:59 12_31_9999Z
        // This definition has no ExpireDate specified because ValidYears=0 indicates no expiration.
        // notAfter should be set in the template to the max date value 99991231235959Z (GeneralizedTime)
        // per RFC5280 section 4.1.2.5
        updatedCert = updatedCert.replaceAll("32303436313130383035303030305a".toUpperCase(), unprettifyHexData(ascii_to_hexa("99991231235959Z")));
    }
    else if(validyears > 0){
        // Do nothing
    }

    xmlDoc.getElementsByTagName("CompressedCerts")[0].getElementsByTagName("CompressedCert")[compressedCert].getElementsByTagName("TemplateData")[0].textContent = "\n\t\t" + prettyPrintHex(updatedCert, 32).replaceAll("\n", "\n\t\t")+"\n\t\t";
    xmlDoc.getElementsByTagName("CompressedCerts")[0].getElementsByTagName("CompressedCert")[compressedCert].setAttribute("ValidYears", validyears);

    var sn_selection = is_sn_selected_as_CN();
    // Form the .c and .h files to download
    if(slotNumber == 10){
        var deviceCertH = document.getElementById('tflxtls_cust_cert_def_device_h').innerHTML;
        jsZipAddFile("tflxtls_cust_cert_def_device.h", deviceCertH);

        var deviceCertC = document.getElementById('tflxtls_cust_cert_def_device_c_p1').innerHTML
            + convertHextoChex(updatedCert, 32)
        if(sn_selection)
            deviceCertC += document.getElementById('tflxtls_cust_cert_def_device_c_p2').innerHTML;
        deviceCertC += document.getElementById('tflxtls_cust_cert_def_device_c_p3').innerHTML;
        if(sn_selection)
            deviceCertC += document.getElementById('tflxtls_cust_cert_def_device_c_p5').innerHTML;
        else
            deviceCertC += document.getElementById('tflxtls_cust_cert_def_device_c_p4').innerHTML;
        jsZipAddFile("tflxtls_cust_cert_def_device.c", deviceCertC);
    }
    else if(slotNumber == 12){
        var signerCertH = document.getElementById('tflxtls_cust_cert_def_signer_h').innerHTML;
        jsZipAddFile("tflxtls_cust_cert_def_signer.h", signerCertH);

        var ca_pub_key = xmlDoc.getElementsByTagName("CompressedCerts")[0].getElementsByTagName("CompressedCert")[1].getElementsByTagName("CAPublicKey")[0].textContent;
        var signerCertC = document.getElementById('tflxtls_cust_cert_def_signer_c_p1').innerHTML + convertHextoChex(updatedCert, 32) + document.getElementById('tflxtls_cust_cert_def_signer_c_p2').innerHTML + convertHextoChex(ca_pub_key, 32) + document.getElementById('tflxtls_cust_cert_def_signer_c_p3').innerHTML;
        jsZipAddFile("tflxtls_cust_cert_def_signer.c", signerCertC);
    }

    // // add .crt files to zip
    // cert_templates = get_certs();

    // if (cert_templates[0] != undefined){
    //     jsZipAddFile("tflxtls_cust_cert_root.crt", cert_templates[0]);
    // }
    // if (cert_templates[1] != undefined){
    //     jsZipAddFile("tflxtls_cust_cert_signer.crt", cert_templates[1]);
    // }
    // if (cert_templates[2] != undefined){
    //     jsZipAddFile("tflxtls_cust_cert_device.crt", cert_templates[2]);
    // }

    return xmlDoc;
}



function tflexUpdataXmlSlot(xmlObj, slotNumber, data){
    // Request finished.
    var xmlDoc = xmlObj;
    var slotType = tflexSlotType[slotNumber];
    var frbytes = "00 00 00 00\n";
    var tempData = data.replaceAll(" ","").replaceAll("\n", "").replaceAll("\r", "").replaceAll("\t", "").replaceAll("0x", "").replaceAll(",","").toUpperCase();
    var temp;
    var currentSlotSize = slotsize[slotNumber];

    switch(slotType){
        case "private":
            // Do nothing currently
            break;
        case "secret":
            // Secret slot, store data and pad zeros at end
            temp = "\n" + prettyPrintHex(tempData, 32);
            for(var i = 0; i < (tflex608SlotSize(slotNumber) - tempData.length/2); i++){
                temp += "00 "
            }
            temp += "\n";
            xmlDoc.getElementsByTagName("DataZone")[0].getElementsByTagName("Slot")[slotNumber].getElementsByTagName("Data")[0].textContent = temp.replaceAll("\n", "\n\t\t\t");
            break;
        case "general":
            // Secret slot, store data and pad zeros at end
            temp = prettyPrintHex(tempData, 32);
            for(var i = 0; i < (tflex608SlotSize(slotNumber) - tempData.length/2); i++){
                temp += "00 "
            }
            temp += "\n";
            xmlDoc.getElementsByTagName("DataZone")[0].getElementsByTagName("Slot")[slotNumber].getElementsByTagName("Data")[0].textContent = temp.replaceAll("\n", "\n\t\t\t");
            break;
        case "cert":
            // Do nothing currently
            break;
        case "public":
            // 4 bytes zero pad, 32 bytes x component, 4 bytes zero pad and 32 bytes y component.
            temp = "";
            temp = "\n" + frbytes + prettyPrintHex(tempData.substring(0, 64), 32) + frbytes + prettyPrintHex(tempData.substring(64, 128), 32);
            xmlDoc.getElementsByTagName("DataZone")[0].getElementsByTagName("Slot")[slotNumber].getElementsByTagName("Data")[0].textContent = temp.replaceAll("\n", "\n\t\t\t");
            break;
        default:
            console.error("Switch Case input error");
    }
    return xmlDoc;
}

function serializeXmlAndSave(xmlObject) {
    // Serialize XML
    let xmlString = "<?xml version='1.0' encoding='utf-8'?> \n" + new XMLSerializer().serializeToString(xmlObject);
    // Sending XML string data to be saved as .xml file.
    //saveAsFile("ECC608_TFLXTLS", xmlString);
    jsZipAddFile(xmlObject.getElementsByTagName("PartNumber")[0].textContent+".xml", xmlString.replace(/^\s*[\r\n]/gm, ""));
}

function convertHextoChex(hexString, sepDist){
    // unprettify recieved data
    var rawHex = unprettifyHexData(hexString);

    var cHex = "";
    for(var i = 0; i < rawHex.length; i++){
        if((!isEven(i)) && ((i+1)%sepDist == 0)){
            cHex += rawHex[i];
            if(i+1 != rawHex.length){
                cHex += ",\n";
            }
        }
        else if(isEven(i)){
            cHex += "0x"
            cHex += rawHex[i];
        }
        else{
            cHex += rawHex[i];
            if(i+1 != rawHex.length){
                cHex += ", ";
            }
        }
    }
    return cHex;
}

function saveAsFile(filename, data) {
    var blob = new Blob([data], { type: 'application/xml' });
    if (window.navigator.msSaveOrOpenBlob) {
        window.navigator.msSaveBlob(blob, filename);
    }
    else {
        var elem = window.document.createElement('a');
        elem.href = window.URL.createObjectURL(blob);
        elem.download = filename;
        document.body.appendChild(elem);
        elem.click();
        document.body.removeChild(elem);
    }
}

function saveAsFileWithType(filename, data, fileType) {
    var blob = new Blob([data], { type: fileType });
    if (window.navigator.msSaveOrOpenBlob) {
        window.navigator.msSaveBlob(blob, filename);
    }
    else {
        var elem = window.document.createElement('a');
        elem.href = window.URL.createObjectURL(blob);
        elem.download = filename;
        document.body.appendChild(elem);
        elem.click();
        document.body.removeChild(elem);
    }
}

function x08GetSlotLockValue(){
    var i = 0;
    var slotLockId = "";
    var isChecked;
    var slotLockBytes = "";
    var functionStatus = 0;

    for(i=0; i<16; i++){
        slotLockId = "slotlock" + i.toString();
        try {
            isChecked = document.getElementById(slotLockId);
            if(isChecked != null){
                isChecked = isChecked.checked;
            }
            else{
                isChecked = false;
            }
        }
        catch(e){
            console.error(e);
            functionStatus = 1;
        }
        finally{
            if(isChecked == true){
                slotLockBytes = slotLockBytes + '0';
            }
            else{
                slotLockBytes = slotLockBytes + '1';
            }
        }
    }
    slotLockBytes = slotLockBytes.split("").reverse().join("")
    slotLockBytes = parseInt(slotLockBytes, 2).toString(16).toUpperCase();
    slotLockBytes = slotLockBytes.slice(0, 2) + " " + slotLockBytes.slice(2, 4);

    return {
        bytes: slotLockBytes,
        status: functionStatus,
    };
}



String.prototype.replaceAll = function(search, replacement) {
    var target = this;
    return target.replace(new RegExp(search, 'g'), replacement);
};