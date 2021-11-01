var global_app = app;

var EXPORT_LAYERS_CSXS_EVENT_ID = "liebao.browser.aet.exportlayersevent";
var TUTORIAL_AUTOMATION_UUID = "267c8093-d35c-4fb7-b0ae-7b224c7fc1ce";

var dodo = function (info) {
    var tmp_w = UnitValue("300 px");
    var tmp_h = UnitValue("200 px");
    app.documents.add(tmp_w, tmp_h, 72); 
}

var createdoc = function() {
    app.documents.add();
}

function ESPSExportLayers() {
    // 能否在这之前先加载好8li
    // 或者点击liebao browser的时候：1.判断一下 文件-自动-test_auto 是否可点击(当前是否有文档被打开) 2.可点击则触发点击
    //↓ 1.当前是否有文档被打开 2.文件-自动-test_auto 的点击动作
    if (app.documents.length !== 0) {
        //createdoc();
        // 2.
        //var idfile = charIDToTypeID();

    }
    

    //app.doAction();runMenuItem

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