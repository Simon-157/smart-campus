void setupOTA() {
    // Set OTA hostname and password
    ArduinoOTA.setHostname("smartnode_sh"); 
    ArduinoOTA.setPassword("smartnode_sh"); 

    // Start OTA
    ArduinoOTA.begin();
    ArduinoOTA.onStart([]() {
        Serial.println("Start updating...");
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
}



const char* serverIndex = 
"<style>"
    "body {"
        "font-family: Arial, sans-serif;"
        "margin: 0;"
        "padding: 20px;"
        "background-color: #f7f7f7;"
    "}"
    "h1 {"
        "text-align: center;"
        "margin-bottom: 20px;"
    "}"
    "form {"
        "text-align: center;"
        "margin-bottom: 20px;"
    "}"
    "input[type=\"file\"] {"
        "display: block;"
        "margin: 0 auto;"
        "margin-bottom: 10px;"
    "}"
    "input[type=\"submit\"] {"
        "background-color: #4CAF50;"
        "color: white;"
        "padding: 10px 20px;"
        "border: none;"
        "border-radius: 4px;"
        "cursor: pointer;"
        "font-size: 16px;"
    "}"
    "input[type=\"submit\"]:hover {"
        "background-color: #45a049;"
    "}"
    "#prg {"
        "text-align: center;"
        "font-size: 18px;"
    "}"
"</style>"
"<h1>File Upload</h1>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
    "<input type='file' name='update'>"
    "<input type='submit' value='Update'>"
"</form>"
"<div id='prg'>Progress: 0%</div>"
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<script>"
    "$('form').submit(function(e){"
        "e.preventDefault();"
        "var form = $('#upload_form')[0];"
        "var data = new FormData(form);"
        "$.ajax({"
            "url: '/update',"
            "type: 'POST',"
            "data: data,"
            "contentType: false,"
            "processData:false,"
            "xhr: function() {"
                "var xhr = new window.XMLHttpRequest();"
                "xhr.upload.addEventListener('progress', function(evt) {"
                    "if (evt.lengthComputable) {"
                        "var per = evt.loaded / evt.total;"
                        "$('#prg').html('Progress: ' + Math.round(per * 100) + '%');"
                    "}"
                "}, false);"
                "return xhr;"
            "},"
            "success: function(d, s) {"
                "console.log('Success!')"
            "},"
            "error: function(a, b, c) {"
                "console.log('Error!')"
            "}"
        "});"
    "});"
"</script>";
