const char HOME_PAGE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart Object Home</title>
    <link href="https://fonts.googleapis.com/css2?family=Roboto:wght@400;500;700&display=swap" rel="stylesheet">
    <link href="https://cdn.jsdelivr.net/npm/tailwindcss@2.2.19/dist/tailwind.min.css" rel="stylesheet">
    <style>
        /* CSS Styles */
        body {
            font-family: 'Roboto', sans-serif;
            margin: 0;
            padding: 0;
            background-color: #f3f4f6;
        }

        .container {
            max-width: 90%;
            margin: 0 auto;
            padding: 20px;
        }

        .navbar {
            background-color: #4a90e2;
            padding: 10px;
            border-radius: 5px;
            margin-bottom: 20px;
        }

        .navbar ul {
            list-style-type: none;
            padding: 0;
            margin: 0;
            display: flex;
            justify-content: center;
        }

        .navbar li {
            margin-right: 20px;
        }

        .navbar a {
            color: #fff;
            text-decoration: none;
            font-size: 1.2rem;
            font-weight: bold;
        }

        .heading {
            text-align: center;
            font-size: 1rem;
            margin-bottom: 20px;
        }

        .sensor-container {
            display: flex;
            gap: 10px;
            justify-content: center;
            margin-bottom: 20px;
        }

        .sensor-card {
            background-color: #ffffff;
            padding: 20px;
            border-radius: 50%;
            box-shadow: 0px 4px 10px rgba(0, 0, 0, 0.1);
            transition: transform 0.3s ease, box-shadow 0.3s ease;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            text-align: center;
            width: 150px;
            height: 150px;
        }

        .sensor-card:hover {
            transform: scale(1.1);
            box-shadow: 0px 8px 20px rgba(0, 0, 0, 0.1);
        }

        .sensor-title {
            font-size: 1.5rem;
            font-weight: bold;
            margin-bottom: 10px;
        }

        .sensor-value {
            font-size: 1rem;
            color: blue;
        }

        .button-container {
            display: flex;
            justify-content: center;
            flex-wrap: wrap;
            margin-bottom: 20px;
        }

        .button-container>div {
            margin-right: 10px;
        }

        .button {
            color: #fff;
            border: none;
            border-radius: 5px;
            padding: 10px;
            font-size: 1rem;
            text-align: center;
            cursor: pointer;
            margin-bottom: 10px;
            transition: background-color 0.3s ease;
            width: 150px;
            position: relative;
        }

        .button.green {
            background-color: #4CAF50;
        }

        .button.red {
            background-color: #FF5252;
        }

        .button.blue {
            background-color: #414a4c;
        }

        .button.blue2 {
            background-color: #353839;
        }

        .button:hover {
            filter: brightness(90%);
        }

        .loader {
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            border: 3px solid #f3f3f3;
            border-top: 3px solid #3498db;
            border-radius: 50%;
            width: 20px;
            height: 20px;
            animation: spin 1s linear infinite;
            display: none; /* Hide loader by default */
        }

        @keyframes spin {
            0% {
                transform: rotate(0deg);
            }
            100% {
                transform: rotate(360deg);
            }
        }

        .table-container {
            margin-top: 20px;
        }

        table {
            width: 100%;
            border-collapse: collapse;
        }

        th,
        td {
            border: 1px solid #ddd;
            padding: 8px;
            text-align: left;
        }

        th {
            background-color: #f2f2f2;
        }

        /* Toast Styles */
        .toast {
            position: fixed;
            bottom: 20px;
            left: 50%;
            transform: translateX(-50%);
            background-color: #333;
            color: #fff;
            padding: 10px 20px;
            border-radius: 5px;
            display: none;
        }

        .toast.success {
            background-color: #4CAF50;
        }

        .toast.error {
            background-color: #FF5252;
        }
    </style>
</head>

<body>
    <div class="container">
        <div class="navbar">
            <ul>
                <li><a href="/">Home</a></li>
                <li><a href="/data">Data</a></li>
                <li><a href="/settings">Settings</a></li>
                <li><a href="/setupwifi">AP Set</a></li>
            </ul>
        </div>
        <h1 class="heading">Smart Node : <span id="uniqueID"></span></h1>
        <div class="sensor-container">
            <div class="sensor-card bg-blue-500 hover:bg-blue-600">
                <h3 class="sensor-title">Temp</h3>
                <p id="temperature" class="sensor-value"> </p>
            </div>
            <div class="sensor-card bg-green-500 hover:bg-green-600">
                <h3 class="sensor-title">Hum</h3>
                <p id="humidity" class="sensor-value"> </p>
            </div>
        </div>
        <div class="button-container">
            <div>
                <button id="startFan" class="button green">Start Fan <span class="loader"></span></button>
            </div>
            <div>
                <button id="stopFan" class="button red">Stop Fan <span class="loader"></span></button>
            </div>
            <div>
                <button id="showLDRData" class="button blue">ldr readings <span class="loader"></span></button>
            </div>
            <div>
                <button id="clear" class="button blue2">clear memory <span class="loader"></span></button>
            </div>
        </div>
        <div class="flex items-center justify-center">
          <div class="fun-state mr-4" id="funState"></div> 
          <div class="active-mode mr-4" id="activeMode"></div>
        </div>
        <div class="table-container">
            <h1 class="heading">Latest LDR Readings</h1>
            <table>
                <thead>
                    <tr>
                        <th>ReadingId</th>
                        <th>LDRValue</th>
                        <th>Timestamp</th>
                    </tr>
                </thead>
                <tbody id="ldrData">
                    <!-- LDR sensor data will be displayed here ReadingID, r.Timestamp, r.LightIntensity, n.NodeName, l.LocationName, l.LocationType, r.Protocol -->
                </tbody>
            </table>
        </div>
        <div id="toast" class="toast"></div> <!-- Toast container -->
    </div>
    <script>
        window.onload = function () {
            document.getElementById("uniqueID").innerText = "{uniqueID}";

            // Check server-provided states for fun and active mode
            const isManualOverrideOn = true; // Replace with server value
            const isAPModeActive = true; // Replace with server value
            const isStatModeActive = true; // Replace with server value

            let funState = '';
            if (isManualOverrideOn) {
                funState = "Manual Control: ON 🎉";
            } else {
                funState = "Manual Control: OFF 🚫";
            }

            document.getElementById("funState").innerText = funState;

            let activeMode = '';
            if (isAPModeActive && isStatModeActive) {
                activeMode = "Active Mode: AP📡 and STAT 🌐";
            } else if (isAPModeActive) {
                activeMode = "Active Mode: AP Mode 📡";
            } else if (isStatModeActive) {
                activeMode = "Active Mode: STAT Mode 📶";
            }

            document.getElementById("activeMode").innerText = activeMode;
        };

        function showToast(message, type) {
            const toast = document.getElementById("toast");
            toast.textContent = message;
            toast.classList.add("toast", type);
            toast.style.display = "block";
            setTimeout(() => {
                toast.style.display = "none";
                toast.classList.remove(type);
            }, 3000); // 3 seconds timeout
        }

        function updateSensorValues() {
            // Function to update sensor values
            var xhr1 = new XMLHttpRequest();
            xhr1.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("temperature").innerText = "" + this.responseText + "°C";
                }
            };
            xhr1.open("GET", "/temperature", true);
            xhr1.send();

            var xhr2 = new XMLHttpRequest();
            xhr2.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("humidity").innerText = "" + this.responseText + "%";
                }
            };
            xhr2.open("GET", "/humidity", true);
            xhr2.send();
        }

        setInterval(updateSensorValues, 5000); // Update sensor values every 5 seconds

        document.getElementById("startFan").onclick = function () {
            // Function to start AC fan manually
            var button = this;
            var loader = button.querySelector('.loader');
            loader.style.display = 'inline-block'; // Show loader
            button.disabled = true; 
            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function () {
                if (this.readyState == 4) {
                    loader.style.display = 'none'; 
                    button.disabled = false;
                    if (this.status == 200) {
                        showToast("Fan started successfully", "success");
                    } else {
                        showToast("Failed: enable manual control", "error");
                    }
                }
            };
            xhr.open("POST", "/fan", true);
            xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
            xhr.send("action=start");
        };

        document.getElementById("stopFan").onclick = function () {
            // Function to stop AC fan manually
            var button = this;
            var loader = button.querySelector('.loader');
            loader.style.display = 'inline-block'; 
            button.disabled = true; 
            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function () {
                if (this.readyState == 4) {
                    loader.style.display = 'none'; 
                    button.disabled = false; 
                    if (this.status == 200) {
                        showToast("Fan stopped successfully", "success");
                    } else {
                        showToast("Failed: enable manual control", "error");
                    }
                }
            };
            xhr.open("POST", "/fan", true);
            xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
            xhr.send("action=stop");
        };

        document.getElementById("clear").onclick = function () {
            var button = this;
            var loader = button.querySelector('.loader');
            loader.style.display = 'inline-block'; // Show loader
            button.disabled = true; // Disable button while processing
            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function () {
                if (this.readyState == 4) {
                    loader.style.display = 'none'; // Hide loader
                    button.disabled = false; // Enable button after processing
                    if (this.status == 200) {
                        showToast("Memory cleared", "success");
                    } else {
                        showToast("Failed to clear memory", "error");
                    }
                }
            };
            xhr.open("POST", "/clear", true);
            xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
            xhr.send("action=stop");
        };

        document.getElementById("showLDRData").onclick = function () {
            // Function to show last 25 recorded LDR sensor data
            var button = this;
            var loader = button.querySelector('.loader');
            loader.style.display = 'inline-block'; // Show loader
            button.disabled = true; // Disable button while processing
            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function () {
                if (this.readyState == 4) {
                    loader.style.display = 'none'; // Hide loader
                    button.disabled = false; // Enable button after processing
                    if (this.status == 200) {
                        var dataArray = JSON.parse(this.responseText);
                        var ldrData = document.getElementById("ldrData");
                        ldrData.innerHTML = ""; // Clear previous data
                        dataArray.forEach(function (record) {
                            var row = document.createElement("tr");
                            row.innerHTML = "<td>" + record.ReadingID + "</td>" +
                                "<td>" + record.ReadingValue + "</td>" +
                                "<td>" + record.Timestamp + "</td>";
                            ldrData.appendChild(row);
                        });
                    }
                }
            };
            xhr.open("GET", "/ldrData", true);
            xhr.send();
        };
    </script>
</body>

</html>


)=====";
