"use strict";

function customCertificateMsg() {
    alert("Due to the way the certificates are stored/retrieved from the ECC608 device, using Custom certificates will require some knowledge on compressed certificates and certificate templates.");
}

function cert10RadioHandler(){
    var radVal = getFormRadioValue(formNameMain, "slot10certopt");

    if (radVal === 'custCert') {
        $('#10certname').show();
        $('#10certcommonname').show();
        $('#10certyear').show();
        $('#10verify').show();
        $('#device_custcert_notes').show();
        $('#custCertPubkey').show();
        $('#manIdHexId').show();
        $('#cbid_devcertcn').show();
        $('label[for="cbid_devcertcn"]').show();
        document.getElementById("cbid_devcertcn").checked = true;
        change_devcertcn();
        setRadioValue(formNameMain, "slot12certopt", "custCert");
    } else {
        $('#10certname').hide();
        $('#10certcommonname').hide();
        $('#10certyear').hide();
        $('#10verify').hide();
        $('#device_custcert_notes').hide();
        $('#custCertPubkey').hide();
        $('#manIdHexId').hide();
        $('#cbid_devcertcn').hide();
        $('label[for="cbid_devcertcn"]').hide();
        document.getElementById("cbid_devcertcn").checked = false;
        change_devcertcn();
        setRadioValue(formNameMain, "slot12certopt", "MCHPCert");
    }

    radVal = getFormRadioValue(formNameMain, "slot12certopt");

    if (radVal === 'custCert') {
        $('#12certname').show();
        $('#12certcommonname').show();
        $('#12certyear').show();
        $('#12verify').show();
        $('#signer_custcert_notes').show();
        $('#custCertPubkey').show();
        $('#manIdHexId').show();
        setRadioValue(formNameMain, "slot10certopt", "custCert");
    } else {
        $('#12certname').hide();
        $('#12certcommonname').hide();
        $('#12certyear').hide();
        $('#12verify').hide();
        $('#signer_custcert_notes').hide();
        $('#custCertPubkey').hide();
        $('#manIdHexId').hide();
        setRadioValue(formNameMain, "slot10certopt", "MCHPCert");
    }
}

$('#10certname').hide();
$('#10certcommonname').hide();
$('#10certyear').hide();
$('#10verify').hide();
$('#custCertPubkey').hide();
$('#manIdHexId').hide();
$('#device_custcert_notes').hide();
$('#cbid_devcertcn').hide();
$('label[for="cbid_devcertcn"]').hide();


function cert12RadioHandler(){
    var radVal = getFormRadioValue(formNameMain, "slot12certopt");

    if (radVal === 'custCert') {
        $('#12certname').show();
        $('#12certcommonname').show();
        $('#12certyear').show();
        $('#12verify').show();
        $('#signer_custcert_notes').show();
        $('#custCertPubkey').show();
        $('#manIdHexId').show();
        setRadioValue(formNameMain, "slot10certopt", "custCert");
    } else {
        $('#12certname').hide();
        $('#12certcommonname').hide();
        $('#12certyear').hide();
        $('#12verify').hide();
        $('#signer_custcert_notes').hide();
        $('#custCertPubkey').hide();
        $('#manIdHexId').hide();
        setRadioValue(formNameMain, "slot10certopt", "MCHPCert");
    }

    radVal = getFormRadioValue(formNameMain, "slot10certopt");

    if (radVal === 'custCert') {
        $('#10certname').show();
        $('#10certcommonname').show();
        $('#10certyear').show();
        $('#10verify').show();
        $('#device_custcert_notes').show();
        $('#custCertPubkey').show();
        $('#manIdHexId').show();
        $('#cbid_devcertcn').show();
        $('#cbid_devcertcn').show();
        $('label[for="cbid_devcertcn"]').show();
        document.getElementById("cbid_devcertcn").checked = true;
        change_devcertcn();
        setRadioValue(formNameMain, "slot12certopt", "custCert");
    } else {
        $('#10certname').hide();
        $('#10certcommonname').hide();
        $('#10certyear').hide();
        $('#10verify').hide();
        $('#device_custcert_notes').hide();
        $('#custCertPubkey').hide();
        $('#manIdHexId').hide();
        $('#cbid_devcertcn').hide();
        $('label[for="cbid_devcertcn"]').hide();
        document.getElementById("cbid_devcertcn").checked = false;
        change_devcertcn();
        setRadioValue(formNameMain, "slot12certopt", "MCHPCert");
    }
}

$('#12certname').hide();
$('#12certcommonname').hide();
$('#12certyear').hide();
$('#12verify').hide();
$('#custCertPubkey').hide();
$('#manIdHexId').hide();
$('#signer_custcert_notes').hide();

// Function to handle custom certificate data
function certVerify() {
    //console.log(this.name);
    if (this.value == "Verify") {
        var certName = document.getElementById(this.id.replace("verify", "certname")).value;
        //console.log(certName);
        var certDataValid1 = false;
        var certDataValid2 = false;
        var certDataValid3 = false;

        if (certName.length == 0) {
            alert("Error: Org name field cannot be empty");
            certDataValid1 = false;
        } else {
            certDataValid1 = true;
        }

        var certName = document.getElementById(this.id.replace("verify", "certcommonname")).value;
        if (certName.length == 0) {
            alert("Error: Common name field cannot be empty");
            certDataValid3 = false;
        } else {
            certDataValid3 = true;
        }

        if(document.getElementById(this.id.replace("verify", "certyear")) != null){
            var certValidity = document.getElementById(this.id.replace("verify", "certyear")).value;
            //console.log(certValidity);
            if (parseInt(certValidity) <= 31 && parseInt(certValidity) >= 0) {
                certDataValid2 = true;
            } else {
                alert("Error: Certificate validity can be 0 to 31 years, entering 0 will set the certificate expiry date to year 9999");
                certDataValid2 = false;
            }
        }
        else{
            certDataValid2 = true;
        }

        if (certDataValid1 == true && certDataValid2 == true && certDataValid3 == true) {
            //console.log(this.id);
            document.getElementById(this.id.replace("verify", "certname")).disabled = true;
            document.getElementById(this.id.replace("verify", "certyear")).disabled = true;
            document.getElementById(this.id.replace("verify", "certcommonname")).disabled = true;
            this.value = "Modify";
        }
    } else if (this.value == "Modify") {
        document.getElementById(this.id.replace("verify", "certname")).disabled = false;
        document.getElementById(this.id.replace("verify", "certyear")).disabled = false;
        document.getElementById(this.id.replace("verify", "certcommonname")).disabled = false;
        this.value = "Verify";
    }
}

function rootVerify(){
    if (this.value == "Verify") {
        var certName = document.getElementById(this.id.replace("verify", "certname")).value;
        //console.log(certName);
        var certDataValid1 = false;
        var certDataValid2 = false;

        if (certName.length == 0) {
            alert("Error: Org name field cannot be empty");
            certDataValid1 = false;
        } else {
            certDataValid1 = true;
        }

        var certName = document.getElementById(this.id.replace("verify", "certcommonname")).value;
        if (certName.length == 0) {
            alert("Error: Common name field cannot be empty");
            certDataValid2 = false;
        } else {
            certDataValid2 = true;
        }

        if (certDataValid1 == true && certDataValid2 == true) {
            //console.log(this.id);
            document.getElementById(this.id.replace("verify", "certname")).disabled = true;
            document.getElementById(this.id.replace("verify", "certcommonname")).disabled = true;
            this.value = "Modify";
        }
    } else if (this.value == "Modify") {
        document.getElementById(this.id.replace("verify", "certname")).disabled = false;
        document.getElementById(this.id.replace("verify", "certcommonname")).disabled = false;
        this.value = "Verify";
    }
}

// Common function to add input element
function addInputElement(elementTag, elementName, elementType, elementId, parentId) {
    // Creating a input element
    var element = document.createElement(elementTag);

    // Assign attributes to the created element.
    if (elementName == null || elementId == null || parentId == null) {
        console.error("Input cannot be null")
    }

    element.setAttribute("name", elementName);
    element.setAttribute("id", elementId);

    if (elementType != null) {
        element.setAttribute("type", elementType);
    }

    // Adding element to the parent.
    var inpElement = document.getElementById(parentId);
    inpElement.appendChild(element);
}

// Common function to remove an HTML elememt
function removeElement(elementId) {
    var elem = document.getElementById(elementId);
    //console.log(elem);
    if (elem != null) {
        elem.parentNode.removeChild(elem);
    }
}

function addRadioListners(elementName) {
    // Get all radio instances
    var dataOptRadios = document.getElementsByName(elementName);

    // Loop through and add listners to all the radio buttons
    for (var i = dataOptRadios.length; i--;) {
        dataOptRadios[i].onchange = radioEventHandler;
    }
}

// RadioEvent handler
function radioEventHandler() {
    var slotNumberInt = parseInt(this.name.replaceAll("slot", ""), 10);

    if (this.value == "unused") {
        removeElement(this.name + "id");
        removeElement(this.name + "idverify");
        removeElement(this.name + "idButton");

        // Disable validate fo this slot
        slotValidateDict[slotNumberInt] = "none";
    } else if (this.value == "hexdata") {
        removeElement(this.name + "id");
        removeElement(this.name + "idverify");
        removeElement(this.name + "idButton");

        addInputElement("textarea", this.name + "name", null, this.name + "id", this.name + "span");
        var newInputObject = document.getElementById(this.name + "id");
        newInputObject.className += "slotTextArea";
        newInputObject.placeholder = "Input hex data here:\nSlot expects " + slotsize[parseInt(this.name.replace("slot", ""), 10)] + " bytes of data." + "\n\nExample: \n42 94 A8 92 20 CB 2C 7A\n....  .... .... .... .... .... .... .... ....";
        newInputObject.rows = "8";

        addInputElement("input", this.name + "nameverify", "button", this.name + "idverify", this.name + "verify");
        var newButtonObject = document.getElementById(this.name + "idverify");
        newButtonObject.value = "Verify";
        newButtonObject.className += "opt1button";
        newButtonObject.onclick = verifyButtonHandler;

        // Enable validate fo this slot
        slotValidateDict[slotNumberInt] = "invalid";
    } else if (this.value == "pemdata") {
        removeElement(this.name + "id");
        removeElement(this.name + "idverify");
        removeElement(this.name + "idButton");

        addInputElement("input", this.name + "name", "button", this.name + "idButton", this.name + "span");
        var newInputObject = document.getElementById(this.name + "idButton");
        newInputObject.className += "opt1button";
        newInputObject.onchange = pemKeyHandler;
        newInputObject.setAttribute("type", "file");
        newInputObject.accept = ".pem";
        newInputObject.multiple = false;

        addInputElement("textarea", this.name + "name", null, this.name + "id", this.name + "span");
        var newInputObject = document.getElementById(this.name + "id");
        newInputObject.className += "slotTextArea";
        newInputObject.rows = "8";
        newInputObject.style.display = "none";

        // Enable validate fo this slot
        slotValidateDict[slotNumberInt] = "invalid";
    } else {
        console.error("Unknown radio value");
    }
}

function stripPemHeadnFoot(text) {
    var strippedPem;

    if (text.length > 30 ||
        text.replaceAll("\n", "").startsWith("-----BEGIN") ||
        text.replaceAll("\n", "").endsWith("-----") ||
        text.match("-----BEGIN(.*)-----") != null ||
        text.match("-----END(.*)-----") != null) {

        var header = text.match("-----BEGIN(.*)-----");
        var footer = text.match("-----END(.*)-----");
        var headerContent = header[0].replaceAll("-----", "").replaceAll("BEGIN", "");
        var footerContent = footer[0].replaceAll("-----", "").replaceAll("END", "");
        if (headerContent == footerContent) {
            // Getting raw BASE64 encoded data as string
            strippedPem = text.replaceAll(header[0], "").replaceAll(footer[0], "");
            strippedPem = strippedPem.replaceAll("\n", "").replaceAll("\t", "").replaceAll(" ", "");
        } else {
            // Invalid pem file
            strippedPem = null;
        }
    } else {
        //console.log("Not a valid pem file");
        strippedPem = null;
    }

    return strippedPem;
}

function getPemBeginName(text){
    if (text.length > 30 ||
        text.replaceAll("\n", "").startsWith("-----BEGIN") ||
        text.replaceAll("\n", "").endsWith("-----") ||
        text.match("-----BEGIN(.*)-----") != null ||
        text.match("-----END(.*)-----") != null) {

        var header = text.match("-----BEGIN(.*)-----");
        var footer = text.match("-----END(.*)-----");
        var headerContent = header[0].replaceAll("-----", "").replaceAll("BEGIN", "");
        var footerContent = footer[0].replaceAll("-----", "").replaceAll("END", "");
        if (headerContent == footerContent) {
            return headerContent;
        } else {
            // Invalid pem file
            return null;
        }
    } else {
        //console.log("Not a valid pem file");
        return null;
    }
}

function b64ToArrayBuffer(b64) {
    return new Promise((res, rej) => {
        var xhr = new XMLHttpRequest();
        xhr.open('GET', 'data:application/octet-stream;base64,' + b64);
        xhr.responseType = 'arraybuffer';
        xhr.addEventListener('load', e => res(xhr.response));
        xhr.addEventListener('error', e => rej(xhr));
        xhr.send();
    });
}

function _base64ToArrayBuffer(base64) {
    var binary_string =  window.atob(base64);
    var len = binary_string.length;
    var bytes = new Uint8Array( len );
    for (var i = 0; i < len; i++)        {
        bytes[i] = binary_string.charCodeAt(i);
    }
    return bytes.buffer;
}

function buf2hex(buffer) {
    // buffer is an ArrayBuffer ...... Uint8Array
    return Array.prototype.map.call(new Uint8Array(buffer), x => ('00' + x.toString(16)).slice(-2)).join('');
}

function handlePemPrivate(text){
    var decodedText = buf2hex(_base64ToArrayBuffer(text)).toUpperCase();

    // console.log(ascii_to_hexa(decodedText));
    // Checking of 0x04 Uncompressed identifier, 0x02 is compressed
    if(parseInt(decodedText[decodedText.length - 134], 16) == 0x04){
        //console.log(decodedText.slice(decodedText.length - 128, decodedText.length));
        return decodedText.slice(decodedText.length - 128, decodedText.length);
    }
    else{
        return null;
    }
}

function handlePemSymmetric(text){
    var decodedText = buf2hex(_base64ToArrayBuffer(text)).toUpperCase();
    //console.log(decodedText);
    // http://oid-info.com/get/1.2.840.10060.3.1
    // https://tools.ietf.org/html/rfc6031#page-3
    // http://oid-info.com/get/1.2.840.113549.1.9.16.1.25
    var identifier = "300906072A8648CE4C030103";
    //console.log(identifier);
    var key;

    if (decodedText.includes(identifier)) {
        key = decodedText.slice(28, decodedText.length);
        //console.log(key);
        var keySize = parseInt(key.slice(0,2), 16) - 2;

        var isCompressed = parseInt(key.slice(4, 4 + 2), 16);
        //console.log(isCompressed);

        if(isCompressed == 0x04){
            //console.log(key.slice(6, 6 + (keySize * 2)));
            return key.slice(6, 6 + (keySize * 2));
        }
    }
}

function loadPemKeyTextArea(data, elementId){
    document.getElementById(elementId).value = data;
}

function pemKeyHandler(event) {
    var currentSlotNumber = parseInt(event.target.name.replace("slot", ""), 10);
    var input = event.target;
    var reader = new FileReader();
    var radioName = "slot" + currentSlotNumber + "dataopt";
    var textBoxId = radioName + "id";

    //console.log(currentSlotNumber);

    reader.onload = function () {
        var text = reader.result;
        var strippedPem = stripPemHeadnFoot(text);
        var pemData = null;

        //console.log(text);
        var pemName = getPemBeginName(text);
        //console.log(pemName);

        if(pemName != null){
            if(getPemBeginName(text).toUpperCase().includes("PRIVATE") == true){
                pemData = handlePemPrivate(strippedPem);
            }
            else if(getPemBeginName(text).toUpperCase().includes("PUBLIC") == true){
                pemData = handlePemPrivate(strippedPem);
            }
            else if(getPemBeginName(text).toUpperCase().includes("SYMMETRIC") == true){
                pemData = handlePemSymmetric(strippedPem);
            }
            else{
                console.error("Undefined pem name");
            }

            if(pemData != null){
                //console.log(textBoxId);
                loadPemKeyTextArea(pemData, textBoxId);

                // Validate this slot
                slotValidateDict[currentSlotNumber] = "valid";
            }
        }
        else{
            console.error("invalid pem");
        }

    };

    reader.readAsText(input.files[0]);
}

function verifyButtonHandler() {
    var slotNumberInt = parseInt(this.name.replace("slot", ""), 10);

    if (this.value == "Verify") {
        var slotData = getDataFromSlot(this.name.replace("nameverify", ""));
        var currentSlotSize = slotsize[parseInt(this.name.replace("slot", ""), 10)];
        var slotDataValid = false;

        // Check the entered data
        slotDataValid = checkTextAreaString(slotData, currentSlotSize);

        if (slotDataValid == true) {
            //console.log(this.id);
            document.getElementById(this.id.replace("verify", "")).disabled = true;
            this.value = "Modify";

            // Validate this slot
            slotValidateDict[slotNumberInt] = "valid";
        }
    } else if (this.value == "Modify") {
        document.getElementById(this.id.replace("verify", "")).disabled = false;
        this.value = "Verify";
    }

}

function checkTextAreaString(stringData, expectedLength) {
    var formatedString = stringData.replaceAll(" ", "").replaceAll("\n", "").replaceAll("\r", "").replaceAll("\t", "").replaceAll("0x", "").replaceAll(",", "").toUpperCase();
    var slotDataValid = false;

    if ((expectedLength * 2) == formatedString.length) {
        if (is_hex(formatedString)) {
            var string = "Data valid,"
            string += " entered values are:\n"
            string += prettyPrintHex(formatedString, 32);
            alert(string);
            slotDataValid = true;
        } else {
            alert("Error: Data contains non-hex characters");
            slotDataValid = false;
        }
    } else {
        alert("The slot expects: " + expectedLength + "bytes" + "\nYou have entered: " + (formatedString.length / 2) + "bytes");
        slotDataValid = false;
    }
    return slotDataValid;
}

function formatedPrettyPrintHex(StringData, sepDist) {
    var hexStringData = StringData.replaceAll(" ", "").replaceAll("\n", "").replaceAll("\r", "").replaceAll("\t", "").replaceAll("0x", "").replaceAll(",", "").toUpperCase();
    var prettyHex = "";

    for (var i = 0; i < hexStringData.length; i++) {
        if ((!isEven(i)) && ((i + 1) % sepDist == 0)) {
            prettyHex += hexStringData[i];
            prettyHex += "\n";
        } else if (isEven(i)) {
            prettyHex += hexStringData[i];
        } else {
            prettyHex += hexStringData[i];
            prettyHex += " ";
        }
    }
    return prettyHex;
}

function prettyPrintHex(hexStringData, sepDist) {
    var prettyHex = "";
    for (var i = 0; i < hexStringData.length; i++) {
        if ((!isEven(i)) && (!((i + 1) % sepDist == 0)) && ((i + 1) % 16 == 0)) {
            prettyHex += hexStringData[i];
            prettyHex += "  ";
        } else if ((!isEven(i)) && ((i + 1) % sepDist == 0)) {
            prettyHex += hexStringData[i];
            prettyHex += "\n";
        } else if (isEven(i)) {
            prettyHex += hexStringData[i];
        } else {
            prettyHex += hexStringData[i];
            prettyHex += " ";
        }
    }
    return prettyHex;
}

function isEven(n) {
    return n == parseFloat(n) ? !(n % 2) : void 0;
}

function is_hex(str) {
    var regexp = /^[0-9a-fA-F]+$/;
    return regexp.test(str);
}

function invokeRadioListners() {
    addRadioListners("slot5dataopt");
    addRadioListners("slot6dataopt");
    addRadioListners("slot8dataopt");
    addRadioListners("slot9dataopt");
    addRadioListners("slot13dataopt");
    addRadioListners("slot14dataopt");
    addRadioListners("slot15dataopt");
    addRadioListners("slot16dataopt");

    // Get all radio instances
    var cert10OptRadios = document.getElementsByName("slot10certopt");

    // Loop through and add listners to all the radio buttons
    for (var i = cert10OptRadios.length; i--;) {
        cert10OptRadios[i].onchange = cert10RadioHandler;
    }

    var cert12OptRadios = document.getElementsByName("slot12certopt");

    // Loop through and add listners to all the radio buttons
    for (var i = cert10OptRadios.length; i--;) {
        cert12OptRadios[i].onchange = cert12RadioHandler;
    }

    change_devcertcn();
    document.getElementById("10verify").addEventListener("click", certVerify);
    document.getElementById("12verify").addEventListener("click", certVerify);
    document.getElementById("16verify").addEventListener("click", rootVerify);
    document.getElementById("id_version_license").innerHTML = TPDS_VERSION_STRING + TPDS_LICENSE;
}

String.prototype.replaceAll = function (search, replacement) {
    var target = this;
    return target.replace(new RegExp(search, 'g'), replacement);
};