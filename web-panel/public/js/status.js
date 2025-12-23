const statusVoltage = document.getElementById('statusVoltage');
const statusCurrent = document.getElementById('statusCurrent');
const statusStatus = document.getElementById('statusStatus');

const statusVoltageAddress = 4; // current status must be the one after voltage
const statusVoltageScale = 0.00125; // V
const statusCurrentScale = 0.00015; // A

statusVoltageDefaultText = statusVoltage.innerText;

const statusPollingInterval = 2000; // ms

let statusTimer;

async function updateStatus() {
  try {
    const url = `/read?type=input&address=${encodeURIComponent(statusVoltageAddress)}&quantity=2`;
    
    const response = await fetch(url);
    statusStatus.innerHTML = "";
    if (response.ok) {
      const data = await response.json();

      const voltage = data.data[0] * statusVoltageScale;
      const current = data.data[1] * statusCurrentScale * 1000; // * 1000 to mA

      statusVoltage.innerText = "Supply Voltage: " + voltage.toFixed(2) + " V";
      statusCurrent.innerText = "Current Draw: " + current.toFixed(2) + " mA";
    } else {
      statusVoltage.innerText = statusVoltageDefaultText;
      statusCurrent.innerText = "";
      statusStatus.innerHTML = newStatus("alert-danger", `Error: ${data.error || 'Unknown error'}`);
    }
  } catch (err) {
      statusStatus.innerHTML = newStatus("alert-danger", `Server error: ${err.message}`);
  }
}

function statusPolling(enable) {
  if (enable) {
    clearInterval(statusTimer);
    statusTimer = setInterval(updateStatus, statusPollingInterval);
  } else {
    statusVoltage.innerText = statusVoltageDefaultText;
    statusCurrent.innerText = "";
    clearInterval(statusTimer);
    statusTimer = null;
  }
}