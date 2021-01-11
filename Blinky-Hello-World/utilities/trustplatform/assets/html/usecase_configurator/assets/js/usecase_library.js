"use strict";

function showTrainingVideos() {
    $(document.getElementById("training")).show();
    window.location.href = "#training";
}

var selectedUseCases = {
    100: {
        name: "secureBoot",
        selected: false,
        ids: ["Slot15"]
    },
    101: {
        name: "secureKeyRotation",
        selected: false,
        ids: ["Slot13", "Slot14"]
    },
    102: {
        name: "ipProtection",
        "selected": false,
        ids: ["Slot5"]
    },
    103: {
        name: "disposable",
        selected: false,
        ids: ["Slot5"]
    },
    104: {
        name: "customPKI",
        selected: false,
        ids: ["Slot10", "Slot12", "custCertPubkey"]
    },
    105: {
        name: "cloud_gcp_tflex",
        selected: false,
        ids: []
    },
    106: {
        name: "disposable_asym_tflx",
        selected: false,
        ids: ["Slot10", "Slot12", "custCertPubkey"]
    },

    200: {
        name: "cloud_aws_tng",
        selected: false,
        ids: []
    },
    201: {
        name: "cloud_gcp_tng",
        selected: false,
        ids: []
    },
    202: {
        name: "cloud_azure_tng",
        selected: false,
        ids: []
    },
};

function toggleUseCase(useCase) {
    var uc = document.getElementById(selectedUseCases[useCase].name);
    var useCaseName = selectedUseCases[useCase].name;
    var item = document.getElementById(useCaseName + "Item");
    var button = document.getElementById(useCaseName + "Btn");
    if (selectedUseCases[useCase].selected === false) {
        selectedUseCases[useCase].ids.forEach(element => {
            var rowElement = document.getElementById(element);
            rowElement.classList.add("danger");
        });
        selectedUseCases[useCase].selected = true;
        $(uc).show();
        item.style.backgroundColor = "#00bb00";
        button.innerHTML = "UNSELECT";
        button.classList.add("use_case_btn_selected");
    } else {
        selectedUseCases[useCase].ids.forEach(element => {
            var rowElement = document.getElementById(element);
            rowElement.classList.remove("danger");
        });
        selectedUseCases[useCase].selected = false;
        $(uc).hide();
        item.style.backgroundColor = "#e40222";
        button.innerHTML = "SELECT";
        button.classList.remove("use_case_btn_selected");
    }
}

function validateUseCaseSlots() {
    //Object.keys(selectedUseCases).length
    var usecaseElements;
    var alertUseCasesNames = "";
    var alertUseCaseSlots = "";
    var alertStatus = false;
    var radioName;

    for (usecaseElements in selectedUseCases) {
        if (selectedUseCases[usecaseElements].selected == true) {
            //console.log(selectedUseCases[usecaseElements].ids);
            for (let i = 0; i < selectedUseCases[usecaseElements].ids.length; i++) {
                var element = selectedUseCases[usecaseElements].ids[i];
                if (element == "Slot10" || element == "Slot12") {
                    radioName = element.toLowerCase() + "certopt";
                    if (getFormRadioValue(formNameMain, radioName) == "MCHPCert") {
                        if (!alertUseCaseSlots.includes(element)) {
                            alertUseCaseSlots += element + "\r\n";
                            alertStatus = true;
                        }
                    }
                } else if (element == "custCertPubkey") {
                    radioName = "slot16" + "dataopt";
                    if (getFormRadioValue(formNameMain, radioName) == "unused") {
                        alertUseCaseSlots += "CA Public key data" + "\r\n";
                        alertStatus = true;
                    }
                } else {
                    radioName = element.toLowerCase() + "dataopt";
                    if (getFormRadioValue(formNameMain, radioName) == "unused") {
                        if (!alertUseCaseSlots.includes(element)) {
                            alertUseCaseSlots += element + "\r\n";
                            alertStatus = true;
                        }
                    }
                }
            }
        }
    }

    if (alertStatus) {
        var alertString = "For the usecases selected, data must be provided in the slots marked in red. \r\n\nData is required in the following slots: \r\n" + alertUseCaseSlots;
        alert(alertString);
    }
    return alertStatus;
}