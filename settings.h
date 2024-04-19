const char SETTINGS_PAGE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Settings</title>
    <link href="https://cdn.jsdelivr.net/npm/tailwindcss@2.2.19/dist/tailwind.min.css" rel="stylesheet">
    <link href="https://fonts.googleapis.com/css2?family=Roboto:wght@400;500;700&display=swap" rel="stylesheet">

    <style>
        body {
            font-family: 'Roboto', sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f4f4f4;
            position: relative;
        }

        .container {
            max-width: 600px;
            margin: 0 auto;
            background-color: #fff;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
        }

        h1 {
            text-align: center;
            margin-bottom: 30px;
            color: #333;
        }

        label {
            display: block;
            margin-bottom: 10px;
            font-weight: bold;
            color: #333;
        }

        input[type="text"],
        input[type="password"],
        select {
            width: calc(100% - 24px);
            padding: 12px;
            margin-bottom: 20px;
            border: 1px solid #ccc;
            border-radius: 4px;
            box-sizing: border-box;
            font-size: 16px;
        }

        input[type="button"] {
            width: calc(100% - 24px);
            background-color: #4CAF50;
            color: white;
            padding: 15px 0;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 16px;
            transition: background-color 0.3s;
        }

        input[type="button"]:hover {
            background-color: #45a049;
        }

        .method-radio {
            display: inline-block;
            margin-right: 20px;
        }

        .method-label {
            margin-right: 10px;
            color: #555;
        }

        .checkbox-label {
            display: block;
            margin-bottom: 15px;
            color: #555;
        }

        .loader {
            border: 4px solid #f3f3f3;
            border-top: 4px solid #3498db;
            border-radius: 50%;
            width: 30px;
            height: 30px;
            animation: spin 2s linear infinite;
            margin: 0 auto;
            margin-top: 20px;
        }

        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }

        .toast-container {
            position: fixed;
            bottom: 20px;
            right: 20px;
            z-index: 9999;
        }

        .toast {
            visibility: hidden;
            min-width: 250px;
            background-color: #333;
            color: #fff;
            text-align: center;
            border-radius: 5px;
            padding: 16px;
            position: relative;
            font-size: 17px;
            transition: visibility 0.5s ease-in-out;
        }
       

          .toast.success {
              background-color: #4CAF50;
          }

          .toast.error {
              background-color: #FF5252;
          }
        .toast.show {
            visibility: visible;
            animation: fadein 0.5s, fadeout 0.5s 2.5s;
        }

        @keyframes fadein {
            from { bottom: -50px; opacity: 0; }
            to { bottom: 20px; opacity: 1; }
        }

        @keyframes fadeout {
            from { bottom: 20px; opacity: 1; }
            to { bottom: -50px; opacity: 0; }
        }

    </style>
    <script>
         window.onload = function() {
            // Set initial settings values to input fields
            document.getElementById("uniqueID").value = "{uniqueID}";
            document.getElementById("triggerTemp").value = "{triggerTemperature}";
            var postingMethod = "{postingMethod}";
            if (postingMethod === "post") {
                document.getElementById("postMethod").checked = true;
            } else if (postingMethod === "mqtt") {
                document.getElementById("mqttMethod").checked = true;
            }
            // Set the checked attribute for the manual override checkbox based on the value from the server
            var manualOverrideChecked = {manualOverrideChecked};
            if (manualOverrideChecked) {
                document.getElementById("manualOverride").checked = true;
            }
        };


        function saveSettings() {
            var formData = new FormData(document.getElementById("settingsForm"));
            var xhr = new XMLHttpRequest();
            document.getElementById("loader").style.display = "block"; // Show loader
            xhr.open("POST", "/configurations");
            xhr.onload = function () {
                document.getElementById("loader").style.display = "none"; // Hide loader
                if (xhr.status === 200) {
                    console.log("Settings saved successfully");
                    showSuccessToast("Settings updated successfully");
                } else {
                    console.error("Failed to save settings. Status code:", xhr.status);
                    showErrorToast("Failed to save settings");
                }
            };
            xhr.onerror = function () {
                document.getElementById("loader").style.display = "none"; // Hide loader on error
                console.error("Error saving settings");
                showErrorToast("Error saving settings");
            };
            xhr.send(formData);
        }

        function showSuccessToast(message) {
            var toast = document.getElementById("successToast");
            toast.innerText = message;
            toast.classList.add("show", "success"); // Add success class
            setTimeout(function () { toast.classList.remove("show", "success"); }, 3000);
        }

        function showErrorToast(message) {
            var toast = document.getElementById("errorToast");
            toast.innerText = message;
            toast.classList.add("show", "error"); // Add error class
            setTimeout(function () { toast.classList.remove("show", "error"); }, 3000);
        }

    </script>
</head>

<body>
    <div class="container">
        <nav class="navbar bg-blue-500 rounded-lg mb-8 p-4">
            <ul class="flex justify-center">
                <li><a href="/" class="text-white font-bold text-lg">Home</a></li>
                <li><a href="/data" class="text-white font-bold text-lg ml-4">Data Page</a></li>
                <li><a href="/settings" class="text-white font-bold text-lg ml-4">Settings Page</a></li>
            </ul>
        </nav>
        <h1>Settings</h1>
        <form id="settingsForm">
            <label for="uniqueID">Unique Name:</label>
            <input type="text" id="uniqueID" name="uniqueID" placeholder="Enter unique name" required>

            <label>Posting Method:</label>
            <div class="method-radio">
                <input type="radio" id="postMethod" name="postingMethod" value="post">
                <label class="method-label" for="postMethod">HTTP POST</label>
            </div>
            <div class="method-radio">
                <input type="radio" id="mqttMethod" name="postingMethod" value="mqtt">
                <label class="method-label" for="mqttMethod">MQTT</label>
            </div>

              <div class="flex items-center mb-4">
              <label class="checkbox-label mr-2" for="manualOverride">Manual Override:</label>
                  <input type="checkbox" id="manualOverride" name="manualOverride" class="align-middle" onchange="updateValue()">
              </div>
              <input type="hidden" id="valueField" name="valueField">



            <label for="triggerTemp">Trigger Temperature:</label>
            <input type="text" id="triggerTemp" name="triggerTemp" placeholder="Enter trigger temperature" required>

            <input type="button" value="Save Settings" onclick="saveSettings()">
            <div id="loader" class="loader" style="display: none;"></div> <!-- Loader element -->
        </form>
    </div>
    <!-- Toasts -->
    <div class="toast-container">
        <div id="successToast" class="toast"></div>
        <div id="errorToast" class="toast"></div>
    </div>

       <script>
                  function updateValue() {
                      var checkbox = document.getElementById("manualOverride");
                      var valueField = document.getElementById("valueField"); 
                      if (checkbox.checked) {
                          valueField.value = "on"; 
                      } else {
                          valueField.value = ""; 
                      }
                  }
              </script>
</body>

</html>

)=====";