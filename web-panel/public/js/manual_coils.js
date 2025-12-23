const coilAddressR = document.getElementById('coilAddressR');
const coilQuantityR = document.getElementById('coilQuantityR');
const coilSendR = document.getElementById('coilSendR');
const coilStatusR = document.getElementById('coilStatusR');

const coilToggleW = document.getElementById('coilToggleW');
const coilAddressW = document.getElementById('coilAddressW');
const coilSendW = document.getElementById('coilSendW');
const coilStatusW = document.getElementById('coilStatusW');

const coilQuantityWM = document.getElementById('coilQuantityWM');
const coilAddressWM = document.getElementById('coilAddressWM');
const coilSendWM = document.getElementById('coilSendWM');

// Enable toggle only when coil address is valid
coilAddressW.addEventListener('input', () => {
  const val = coilAddressW.value;
  coilToggleW.disabled = !(val !== '' && Number(val) >= 0);
  coilStatusW.textContent = '';
});

// Write Single
async function writeCoil(address, value) {
  try {
    coilStatusW.textContent = 'Sending request...';
    const valInt = value ? 1 : 0;
    const url = `/write?type=coil&address=${encodeURIComponent(address)}&value=${valInt}`;
    
    const response = await fetch(url);
    const data = await response.json();

    if (response.ok) {
      coilStatusW.innerHTML = newStatus("alert-success", `Coil at address ${address} set to ${valInt}`);
    } else {
      coilStatusW.innerHTML = newStatus("alert-danger", `Error: ${data.error || 'Unknown error'}`);
    }
  } catch (err) {
      coilStatusW.innerHTML = newStatus("alert-danger", `Server error: ${err.message}`);
  }
}

// Write multiple 
async function writeMultipleCoils(address, valueArray) {
  try {
    coilStatusWM.textContent = 'Sending request...';
    
    const url = `/write?type=coil&address=${encodeURIComponent(address)}&value=${valueArray}`;

    const response = await fetch(url);
    const data = await response.json();

    if (response.ok) {
      coilStatusWM.innerHTML = newStatus("alert-success", `Coils starting at address ${address} set to ${valueArray}.`);
    } else {
      coilStatusWM.innerHTML = newStatus("alert-danger", `Error: ${data.error || 'Unknown error'}`);
    }
  } catch (err) {
    coilStatusWM.innerHTML = newStatus("alert-danger", `Server error: ${err.message}`);
  }
}

// Read
async function readCoil(address, quantity) {
  try {
    const addr = parseInt(address);
    const quan = parseInt(quantity);

    coilStatusR.textContent = 'Sending request...';
    const url = `/read?type=coil&address=${encodeURIComponent(address)}&quantity=${quantity}`;
    console.log(url);
    const response = await fetch(url);
    const data = await response.json();

    if (response.ok) {
      coilStatusR.innerHTML = "";
      const coilValues = data.data;

      const headingRow = document.getElementById("coilResultTableR_heading");
      const valueRow = document.getElementById("coilResultTableR_value");

      for (let i = addr; i < addr + quan; i++) {
        // Heading Cell
        const cell = document.createElement('td'); // create a table cell
        cell.textContent = i; // set cell content
        headingRow.appendChild(cell); // add cell to row

        // Value cell
        const cell2 = document.createElement('td');
        cell2.textContent = coilValues[i] ? "ON" : "OFF";
        valueRow.appendChild(cell2);
      }
    } else {
      coilStatusR.innerHTML = newStatus("alert-danger", `Error: ${data.error || 'Unknown error'}`);
    }
  } catch (err) {
      coilStatusR.innerHTML = newStatus("alert-danger", `Server error: ${err.message}`);
  }
}

// Update WM toggles
function updateWMToggles() {
  const addr = parseInt(coilAddressWM.value);
  const quan = parseInt(coilQuantityWM.value);
  const workingDiv = document.getElementById("coilWriteMultipleToggleDiv");

  workingDiv.innerHTML = "";
  coilStatusWM.innerHTML = "";

  if (Number(quan) > 256) {
    coilStatusWM.innerHTML = newStatus("alert-warning", `Please enter a quantity less than or equal to 256.`);
    return;
  }

  if (isNaN(quan) || quan < 1 || quan > 256 || isNaN(addr) || addr < 0) {
    return;
  }

  /*<div class="form-check form-switch mb-3">
    <input class="form-check-input" type="checkbox" id="coilToggleWM" disabled/>
    <label class="form-check-label" for="coilToggleWM">Coil OFF/ON (0/1)</label>
  </div>*/

  workingDiv.innerHTML = "<h6>Coil OFF/ON (0/1)</h6>";

  for (var i = addr; i < addr + quan; i++) {
    const id = `coilToggleWM-${i}`;
    
    const formDiv = document.createElement("div");
    formDiv.className = "form-check form-switch mb-3";
    
    const input = document.createElement("input");
    input.className = "form-check-input";
    input.type = "checkbox";
    input.id = id;
    
    const label = document.createElement("label");
    label.className = "form-check-label";
    label.setAttribute("for", id);
    label.textContent = `Coil ${i}`;

    formDiv.appendChild(input);
    formDiv.appendChild(label);

    workingDiv.appendChild(formDiv);
  }
}

coilSendW.addEventListener('click', () => {
  const addr = coilAddressW.value;
  if (addr === '' || Number(addr) < 0) {
    coilStatusW.innerHTML = newStatus("alert-warning", `Please enter a valid coil address (0 or higher).`);
    return;
  }
  const val = coilToggleW.checked;
  writeCoil(addr, val);
});

coilSendWM.addEventListener('click', () => {
  const address = parseInt(coilAddressWM.value);
  const quantity = parseInt(coilQuantityWM.value);

  if (isNaN(address) || address < 0) {
    coilStatusWM.innerHTML = newStatus("alert-warning", `Please enter a valid coil address (0 or higher).`);
    return;
  }
  if (isNaN(quantity) || quantity < 1) {
    coilStatusWM.innerHTML = newStatus("alert-warning", `Please enter a valid quantity to write (1 or higher).`);
    return;
  }

  const valueArray = [];
  for (var i = address; i < address + quantity; i++) {
    const toggleElement = document.getElementById(`coilToggleWM-${i}`);
    const toggleValue = toggleElement.checked ? 1 : 0;
    valueArray.push(toggleValue);
  }

  writeMultipleCoils(address, valueArray); // handles value gathering in here
});

coilSendR.addEventListener('click', () => {
  const addr = coilAddressR.value;
  const quan = coilQuantityR.value;

  const headingRow = document.getElementById("coilResultTableR_heading");
  const valueRow = document.getElementById("coilResultTableR_value");
  headingRow.innerHTML = "";
  valueRow.innerHTML = "";

  if (addr === '' || Number(addr) < 0) {
    coilStatusR.innerHTML = newStatus("alert-warning", `Please enter a valid coil address (0 or higher).`);
    return;
  }
  if (quan === '' || Number(quan) < 1) {
    coilStatusR.innerHTML = newStatus("alert-warning", `Please enter a valid quantity to read (1 or higher).`);
    return;
  }
  readCoil(addr, quan);
});

coilQuantityWM.addEventListener('input', () => {
  updateWMToggles();
});

coilAddressWM.addEventListener('input', () => {
  updateWMToggles();
});