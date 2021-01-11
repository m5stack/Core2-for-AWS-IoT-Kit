"use strict";

var zip = null;

function jsZipInit(){
    zip = new JSZip;
    return 0;
}

function jsZipAddFile(fileName, textData){
    if(zip != null){
        zip.file(fileName, textData);
    }
    else{
        return 1;
    }
    return 0;
}

function jsZipGenerateSave(zipFileName){
    if(zip != null){
        zip.generateAsync({
            type: "blob"
        })
        .then(function (content) {
            saveAsFileWithType((zipFileName + ".zip"), content, "compressed/zip");
        });
    }
    else{
        return 1;
    }
    zip = null;
    return 0;    
}

function jsZipRemovefile(fileName){
    if(zip != null){
        zip.remove(fileName);
    }
    else{
        return 1;
    }
    return 0;
}    

function jszipExample() {
    var zip = new JSZip();
    zip.file("Hello.txt", "Hello World\n");
    zip.generateAsync({
            type: "blob"
        })
        .then(function (content) {
            //saveAsFile("example.zip", content);
            //saveAsFileWithType("sdsa.zip", content, "compressed/zip");
        });
}


