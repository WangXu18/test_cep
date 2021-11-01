var global_app = app;

var EXPORT_LAYERS_CSXS_EVENT_ID = "liebao.browser.aet.exportlayersevent";
var TUTORIAL_AUTOMATION_UUID = "267c8093-d35c-4fb7-b0ae-7b224c7fc1ce";

var dodo = function (info) {
    app.documents.add();
}

function ESPSExportLayers() {
    // The library might not exist in some Photoshop versions.
    alert(3333333333333333333333)
    app.documents.add();
    try {
        var xLib = new ExternalObject("lib:\PlugPlugExternalObject");
    } catch (e) {
        alert(e);
        return;
    }

    if (xLib) {
        var eventObj = new CSXSEvent();
        eventObj.type = EXPORT_LAYERS_CSXS_EVENT_ID;
        eventObj.data = true;
        eventObj.dispatch();
        return;
    }
    return;
}


function ESPSActivateAutomationPlugin() {
    var idPlugin = stringIDToTypeID(TUTORIAL_AUTOMATION_UUID);
    executeAction(idPlugin, undefined, DialogModes.NO);

    return;
}