var global_cs = new CSInterface();

var DONE_CSXS_EVENT_ID = "liebao.browser.aet.doneevent";

function docum() {
  var cs = new CSInterface();
  cs.evalScript("dodo()");
  return;
}

global_cs.addEventListener(DONE_CSXS_EVENT_ID,
  function(event) {
      AlertDialog(event.data);
      return;
  });

  function AlertDialog(msg) {
    var cs = new CSInterface();
    cs.evalScript("alert(\"" + msg + "\");");
    return;
}



function ExportLayersCB() {
  var cs = new CSInterface();
  // 能否在这之前先加载好8li
  // ...
  //cs.loadBinAsync("‪E:\\Softs\\Ps2019\\Adobe Photoshop CC 2019\\Plug-ins\\tutorial_automation.bin",function () { alert("aaaaaaaaaaaaa")});
  
  cs.evalScript("ESPSExportLayers();");
  return;
}

global_cs.evalScript("ESPSActivateAutomationPlugin();");