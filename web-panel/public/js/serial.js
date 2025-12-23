// Populate COM port dropdown
const portSelect = document.getElementById("portSelect");
const baudSelect = document.getElementById("baudSelect");
const slaveSelect = document.getElementById("slaveAddress");
const connectionStatus = document.getElementById("connectionStatus");
const connectBtn = document.getElementById("connectBtn");
const refreshPortsBtn = document.getElementById("refreshPortsBtn");

const connectionStatusDefaultText = 'Select a port and click connect.';
const noPortsText = "No ports found";

let isConnected;
let currentPort; // only required for window.onload
let currentSlave; // only required for window.onload

let connectionStatusTimer;
const connectionStatusTimerInterval = 1000;

async function getStatus() {
    try {
    const res = await fetch('/status');
    const connected = await res.json();
    isConnected = connected.status;
    currentPort = connected.port;
    currentSlave = connected.slave;
    } catch (exception) {
    isConnected = false; // assume
    }
}

async function fetchPorts() {
    try {
    const res = await fetch('/ports');
    const ports = await res.json();

    portSelect.innerHTML = '';
    
    if (ports.length == 0) {
        const option = document.createElement('option');
        option.value = noPortsText;
        option.textContent = noPortsText;
        portSelect.appendChild(option);
    } else {
        ports.forEach(port => {
        const option = document.createElement('option');
        option.value = port;
        option.textContent = port;
        portSelect.appendChild(option);
        });
    }
    
    
    connectionStatus.textContent = connectionStatusDefaultText;
    } catch (exception) {
    portSelect.innerHTML = '<option>Error loading ports</option>';
    connectionStatus.textContent = 'Failed to fetch ports.';
    }
}

refreshPortsBtn.addEventListener('click', () => {
    if (!isConnected) {
    fetchPorts();
    }
});

function updateUI(connected, port, slave) {
    connectionStatus.classList.remove("text-muted", "text-danger", "text-success");
    if (connected) {
    connectionStatus.textContent = `Connected to ${port}`;
    connectionStatus.classList.add("text-success");
    connectBtn.classList.remove("btn-success");
    connectBtn.classList.add("btn-danger");
    connectBtn.innerText = "Disconnect";
    } else {
    connectionStatus.textContent = `Disconnected from ${port}`;
    connectionStatus.classList.add("text-muted");
    connectBtn.classList.remove("btn-danger");
    connectBtn.classList.add("btn-success");
    connectBtn.innerText = "Connect";
    }

    if (slave !== undefined) {
    slaveSelect.value = slave;
    } 
}

connectBtn.addEventListener('click', async () => {
    await getStatus();
    const selectedPort = portSelect.value;
    const selectedBaud = baudSelect.value;
    const selectedSlave = slaveSelect.value;  

    if (selectedPort == noPortsText) {
    connectionStatus.textContent = `No valid port was selected.`;
    connectionStatus.classList.remove("text-muted", "text-success");
    connectionStatus.classList.add("text-danger");
    return;
    }

    if (!isConnected) {
    // Connect
    const res = await fetch(`/connect?port=${encodeURIComponent(selectedPort)}&baud=${encodeURIComponent(selectedBaud)}&slave=${encodeURIComponent(selectedSlave)}`);
    const data = await res.json();

    
    if (data.success) {
        isConnected = true;
        updateUI(true, selectedPort);
        statusPolling(true);
        await ruleListUpdate();
        await vrListUpdate();
        await hwGetModes();
        connectionStatusTimer = setInterval(connectionStatus, connectionStatusTimerInterval);
    } else {
        statusPolling(false);
        connectionStatus.textContent = `Failed: ${data.error}`;
        connectionStatus.classList.remove("text-muted", "text-danger");
        connectionStatus.classList.add("text-danger");
        
        connectBtn.classList.remove("btn-danger");
        connectBtn.classList.add("btn-success");
        connectBtn.innerText = "Connect";
    }        
    } else {
    // Disconnect
    const res = await fetch(`/disconnect`);
    const data = await res.json();

    statusPolling(false);

    if (data.success) {
        updateUI(false, selectedPort);
        ruleListUpdate();
        connectionStatusTimer = clearInterval();
    } else {
        connectionStatus.textContent = `Failed to disconnect: ${data.error}`;
        connectionStatus.classList.remove("text-success");
        connectionStatus.classList.add("text-danger");
    }
    }

    // Status polling regularly to check for serial port disconnect
    async function connectionStatus() {
    const isConnectedBuffer = isConnected;
    const portBuffer = currentPort;
    await getStatus();
    if (isConnectedBuffer != isConnected) {
        currentPort = portBuffer; // for disconnect text to be correct
        updateUI(isConnected, currentPort, currentSlave);
        statusPolling(isConnected);
    }
    }

});