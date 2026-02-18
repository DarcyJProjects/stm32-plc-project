// Populate COM port dropdown
const portSelect = document.getElementById("portSelect");
const baudSelect = document.getElementById("baudSelect");
const slaveSelect = document.getElementById("slaveAddress");
const connectionStatusDOM = document.getElementById("connectionStatus"); 
const connectBtn = document.getElementById("connectBtn");
const refreshPortsBtn = document.getElementById("refreshPortsBtn");

const ftStatusIcon = document.getElementById("ftStatusIcon");
const ftStatusText = document.getElementById("ftStatusText");

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
        
        connectionStatusDOM.textContent = connectionStatusDefaultText;
    } catch (exception) {
        portSelect.innerHTML = '<option>Error loading ports</option>';
        connectionStatusDOM.textContent = 'Failed to fetch ports.';
    }
}

refreshPortsBtn.addEventListener('click', () => {
    if (!isConnected) {
        fetchPorts();
    }
});

function updateUI(connected, port, slave) {
    connectionStatusDOM.classList.remove("text-muted", "text-danger", "text-success");
    
    if (connected) {
        connectionStatusDOM.textContent = `Connected to ${port}`;
        connectionStatusDOM.classList.add("text-success");
        connectBtn.classList.remove("btn-success");
        connectBtn.classList.add("btn-danger");
        connectBtn.innerText = "Disconnect";

        if (ftStatusIcon) {
            ftStatusIcon.classList.remove("text-danger");
            ftStatusIcon.classList.add("text-success");
            ftStatusText.innerHTML = "System Connected";
        }
    } else {
        connectionStatusDOM.textContent = `Disconnected from ${port}`;
        connectionStatusDOM.classList.add("text-muted");
        connectBtn.classList.remove("btn-danger");
        connectBtn.classList.add("btn-success");
        connectBtn.innerText = "Connect";

        if (ftStatusIcon) {
            ftStatusIcon.classList.remove("text-success");
            ftStatusIcon.classList.add("text-danger");
            ftStatusText.innerHTML = "System Disconnected";
        }
    }

    if (slave !== undefined) {
        slaveSelect.value = slave;
    } 

    if (port && portSelect.options.length > 0) {
        portSelect.value = port;
    }
}

// Helper function to monitor connection (Moved outside click listener for clarity)
async function monitorConnectionLoop() {
    const isConnectedBuffer = isConnected;
    const portBuffer = currentPort;
    await getStatus();
    
    // If state changed (e.g. cable unplugged)
    if (isConnectedBuffer != isConnected) {
        currentPort = portBuffer; // keep old port name for the 'Disconnected from X' message
        updateUI(isConnected, currentPort, currentSlave);
        statusPolling(isConnected);
        
        // Stop monitoring if disconnected
        if (!isConnected) {
            clearInterval(connectionStatusTimer);
        }
    }
}

connectBtn.addEventListener('click', async () => {
    await getStatus();
    const selectedPort = portSelect.value;
    const selectedBaud = baudSelect.value;
    const selectedSlave = slaveSelect.value;  

    if (selectedPort == noPortsText) {
        connectionStatusDOM.textContent = `No valid port was selected.`;
        connectionStatusDOM.classList.remove("text-muted", "text-success");
        connectionStatusDOM.classList.add("text-danger");
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
            
            // Start monitoring loop
            connectionStatusTimer = setInterval(monitorConnectionLoop, connectionStatusTimerInterval);
        } else {
            statusPolling(false);
            connectionStatusDOM.textContent = `Failed: ${data.error}`;
            connectionStatusDOM.classList.remove("text-muted", "text-danger");
            connectionStatusDOM.classList.add("text-danger");
            
            connectBtn.classList.remove("btn-danger");
            connectBtn.classList.add("btn-success");
            connectBtn.innerText = "Connect";

            if (ftStatusIcon) {
                ftStatusIcon.classList.remove("text-success");
                ftStatusIcon.classList.add("text-danger");
                ftStatusText.innerHTML = "System Disconnected";
            }
        }        
    } else {
        // Disconnect
        const res = await fetch(`/disconnect`);
        const data = await res.json();

        statusPolling(false);

        if (data.success) {
            updateUI(false, selectedPort);
            ruleListUpdate();
            clearInterval(connectionStatusTimer);
        } else {
            connectionStatusDOM.textContent = `Failed to disconnect: ${data.error}`;
            connectionStatusDOM.classList.remove("text-success");
            connectionStatusDOM.classList.add("text-danger");
        }
    }
});