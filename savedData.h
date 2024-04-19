const char SAVED_DATA_PAGE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Saved Data</title>
    <link href="https://cdn.jsdelivr.net/npm/tailwindcss@2.2.19/dist/tailwind.min.css" rel="stylesheet">
    <style>
        /* Custom styles */
        body {
            font-family: 'Roboto', sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f4f4f4;
            position: relative;
        }
        .container {
            max-width: 90%;
            margin: 0 auto;
            padding: 20px;
            display: flex;
            flex-direction: column;
            align-items: center; /* Center contents horizontally */
        }

        table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 20px;
        }

        th, td {
            border: 1px solid #e2e8f0;
            padding: 12px;
            text-align: left;
        }

        /* Additional humidity table style */
        .humidity-table {
            margin-top: 20px;
            width: 100%;
        }

        .humidity-table th, .humidity-table td {
            border: 1px solid #e2e8f0;
            padding: 12px;
            text-align: left;
        }

        /* Media query for responsive layout */
        @media (max-width: 768px) {
            .container {
                padding: 10px;
            }

            th, td {
                padding: 8px;
            }

            th, td {
                font-size: 14px;
            }
        }
    </style>
</head>
<body>
    <div class="container mx-auto">
        <div class="navbar bg-blue-500 rounded-lg mb-8 p-4">
            <ul class="flex justify-center">
                <li><a href="/" class="text-white font-bold text-lg">Home</a></li>
                <li><a href="/data" class="text-white font-bold text-lg ml-4">Data Page</a></li>
                <li><a href="/settings" class="text-white font-bold text-lg ml-4">Settings Page</a></li>
            </ul>
        </div>
        <h1 class="text-3xl font-bold mb-4">Saved Data</h1>
        <table class="divide-y divide-gray-200">
            <thead class="bg-gray-50">
                <tr>
                    <th class="px-4 py-2 text-sm font-medium text-gray-500 uppercase tracking-wider">Index</th>
                    <th class="px-4 py-2 text-sm font-medium text-gray-500 uppercase tracking-wider">Timestamp</th>
                    <th class="px-4 py-2 text-sm font-medium text-gray-500 uppercase tracking-wider">Temperature (°C)</th>
                    <th class="px-4 py-2 text-sm font-medium text-gray-500 uppercase tracking-wider">Humidity (%)</th>
                </tr>
            </thead>
            <tbody id="savedData" class="bg-white">
                <!-- Saved sensor data will be displayed here -->
            </tbody>
        </table>
        <div id="additionalHumidity" class="mt-4">
            <table class="humidity-table">
                <thead>
                    <tr>
                        <th class="px-4 py-2 text-sm font-medium text-gray-500 uppercase tracking-wider">Timestamp</th>
                        <th class="px-4 py-2 text-sm font-medium text-gray-500 uppercase tracking-wider">Humidity (%)</th>
                    </tr>
                </thead>
                <tbody>
                    <!-- Additional humidity values will be displayed here -->
                </tbody>
            </table>
        </div>
    </div>

    <script>
        // Function to fetch saved sensor data
        function fetchSavedData() {
            fetch('/savedData')
                .then(response => response.json())
                .then(data => displaySavedData(data))
                .catch(error => console.error('Error fetching data:', error));
        }

        // Function to display saved sensor data
        function displaySavedData(data) {
            const tableBody = document.getElementById('savedData');
            tableBody.innerHTML = ''; 

            const temperatureArray = data[0];
            const humidityArray = data[1];

            const maxLength = Math.max(temperatureArray.length, humidityArray.length);

            for (let index = 0; index < maxLength; index++) {
                const timestamp = (humidityArray[index] || '').split(',')[0];
                const temperature = (temperatureArray[index] || '').split(',')[1];
                const humidity = (humidityArray[index] || '').split(',')[1];

                const row = `
                    <tr>
                        <td class="border px-4 py-2">${index + 1}</td>
                        <td class="border px-4 py-2">${timestamp}</td>
                        <td class="border px-4 py-2">${temperature}</td>
                        <td class="border px-4 py-2">${humidity}</td>
                    </tr>
                `;

                tableBody.innerHTML += row;
            }

        }

        // Fetch saved sensor data initially
        fetchSavedData();

        // Fetch saved sensor data every 45 seconds
        setInterval(fetchSavedData, 60000);
    </script>
</body>
</html>

)=====";