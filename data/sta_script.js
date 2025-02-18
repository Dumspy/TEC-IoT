let socket;

async function loadInitialData() {
    const response = await fetch('/initial-data');
    const data = await response.json();
    data.forEach(entry => {
        updateChart(entry.timestamp, entry.temp);
        addTimestampToDropdown(entry.timestamp);
    });
}

function connectWebSocket() {
    socket = new WebSocket('ws://' + window.location.hostname + '/ws');

    socket.onmessage = function(event) {
        const data = JSON.parse(event.data);
        updateChart(data.timestamp, data.temp);
        addTimestampToDropdown(data.timestamp);
    };
}

function updateChart(timestamp, newTemp) {
    const timeString = new Date(timestamp * 1000).toLocaleTimeString();
    tempChart.data.labels.push(timeString);
    tempChart.data.datasets[0].data.push(newTemp);
    tempChart.update();

    // Update the latest temperature value
    document.getElementById('latestTemp').innerText = `Latest Temperature: ${newTemp} °C`;
}

function addTimestampToDropdown(timestamp) {
    const select = document.getElementById('timestampSelect');
    const option = document.createElement('option');
    option.value = timestamp;
    option.text = new Date(timestamp * 1000).toLocaleString();
    select.add(option);
}

async function clearCSV() {
    await fetch('/clear-csv', { method: 'POST' });
    alert('CSV data cleared');
    document.getElementById('timestampSelect').innerHTML = '';
}

async function addRow() {
    const row = prompt('Enter row data (timestamp;temp):');
    if (row) {
        await fetch('/add-row', {
            method: 'POST',
            headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
            body: new URLSearchParams({ row })
        });
        alert('Row added');
        const [timestamp, temp] = row.split(';');
        addTimestampToDropdown(timestamp);
    }
}

async function deleteRow() {
    const select = document.getElementById('timestampSelect');
    const timestamp = select.value;
    if (timestamp) {
        await fetch('/delete-row', {
            method: 'POST',
            headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
            body: new URLSearchParams({ timestamp })
        });
        alert('Row deleted');
        select.remove(select.selectedIndex);
        
        // Update the chart
        const timeString = new Date(timestamp * 1000).toLocaleTimeString();
        const index = tempChart.data.labels.indexOf(timeString);
        if (index !== -1) {
            tempChart.data.labels.splice(index, 1);
            tempChart.data.datasets[0].data.splice(index, 1);
            tempChart.update();
        }
    }
}

function downloadCSV() {
    window.location.href = '/temperature_data.csv';
}

const ctx = document.getElementById('tempChart').getContext('2d');
const tempChart = new Chart(ctx, {
    type: 'line',
    data: {
        labels: [],
        datasets: [{ label: 'Temperature (°C)', data: [], borderColor: 'red', fill: false }]
    },
    options: {
        responsive: true,
        scales: {
            y: {
                beginAtZero: true,
                suggestedMin: 0,
                suggestedMax: 100
            }
        },
    }
});

loadInitialData();
connectWebSocket();
