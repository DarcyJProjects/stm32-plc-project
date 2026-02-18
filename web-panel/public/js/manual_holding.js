const holdingAddressR = document.getElementById('holdingAddressR');
const holdingQuantityR = document.getElementById('holdingQuantityR');
const holdingSendR = document.getElementById('holdingSendR');
const holdingStatusR = document.getElementById('holdingStatusR');

const holdingValueW = document.getElementById('holdingValueW');
const holdingAddressW = document.getElementById('holdingAddressW');
const holdingSendW = document.getElementById('holdingSendW');
const holdingStatusW = document.getElementById('holdingStatusW');

const holdingAddressWM = document.getElementById('holdingAddressWM');
const holdignSendWM = document.getElementById('holdingSendWM');
const holdingStatusWM = document.getElementById('holdingStatusWM');

// Write Single
async function writeHolding(address, value) {
  try {
    holdingStatusW.textContent = 'Sending request...';
    
    const response = await fetch('/write', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
            type: 'holding',
            address: parseInt(address),
            value: parseInt(value)
        })
    });

    const data = await response.json();

    if (response.ok) {
      holdingStatusW.innerHTML = newStatus("alert-success", `Holding register at address ${address} set to ${value}.`);
    } else {
      holdingStatusW.innerHTML = newStatus("alert-danger", `Error: ${data.error || 'Unknown error'}`);
    }
  } catch (err) {
      holdingStatusW.innerHTML = newStatus("alert-danger", `Server error: ${err.message}`);
  }
}

// Write multiple 
async function writeMultipleHolding(address, valueArray) {
  try {
    holdingStatusWM.textContent = 'Sending request...';
    
    const response = await fetch('/write', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
            type: 'holding',
            address: parseInt(address),
            value: valueArray.join(',') // Join to CSV string
        })
    });

    const data = await response.json();

    if (response.ok) {
      holdingStatusWM.innerHTML = newStatus("alert-success", `Holding registers starting at address ${address} set to ${valueArray}.`);
    } else {
      holdingStatusWM.innerHTML = newStatus("alert-danger", `Error: ${data.error || 'Unknown error'}`);
    }
  } catch (err) {
    holdingStatusWM.innerHTML = newStatus("alert-danger", `Server error: ${err.message}`);
  }
}

// Read
async function readHolding(address, quantity) {
  try {
    const addr = parseInt(address);
    const quan = parseInt(quantity);

    holdingStatusR.textContent = 'Sending request...';
    const url = `/read?type=holding&address=${encodeURIComponent(address)}&quantity=${quantity}`;
    console.log(url);
    const response = await fetch(url);
    const data = await response.json();

    if (response.ok) {
      holdingStatusR.innerHTML = "";
      const holdingValues = data.data;

      const headingRow = document.getElementById("holdingResultTableR_heading");
      const valueRow = document.getElementById("holdingResultTableR_value");

      for (let i = addr; i < addr + quan; i++) {
        // Heading Cell
        const cell = document.createElement('td'); // create a table cell
        cell.textContent = i; // set cell content
        headingRow.appendChild(cell); // add cell to row

        // Value cell
        const cell2 = document.createElement('td');
        cell2.textContent = holdingValues[i];
        valueRow.appendChild(cell2);
      }
    } else {
      holdingStatusR.innerHTML = newStatus("alert-danger", `Error: ${data.error || 'Unknown error'}`);
    }
  } catch (err) {
      holdingStatusR.innerHTML = newStatus("alert-danger", `Server error: ${err.message}`);
  }
}

// Update WM inputs
function updateWMInputs() {
  const addr = parseInt(holdingAddressWM.value);
  const quan = parseInt(holdingQuantityWM.value);
  const workingDiv = document.getElementById("holdingWriteMultipleToggleDiv");

  workingDiv.innerHTML = "";
  holdingStatusWM.innerHTML = "";

  if (Number(quan) > 256) {
    holdingStatusWM.innerHTML = newStatus("alert-warning", `Please enter a quantity less than or equal to 256.`);
    return;
  }

  if (isNaN(quan) || quan < 1 || quan > 256 || isNaN(addr) || addr < 0) {
    return;
  }

  /*<div class="mb-3">
    <label for="holdingValueW" class="form-label">Value</label>
    <input type="number" class="form-control" id="holdingValueW" placeholder="Enter value to write (0-65535)" min="0" />
  </div>*/

  workingDiv.innerHTML = "<h6>Holding Register (Value 0-65535)</h6>";

  for (var i = addr; i < addr + quan; i++) {
    const id = `holdingInput-${i}`;
    
    const formDiv = document.createElement("div");
    formDiv.className = "mb-3 d-flex align-items-center";

    const label = document.createElement("label");
    label.className = "me-2"; // right margin
    label.setAttribute("for", id);
    label.textContent = `Register ${i}`;
    
    const input = document.createElement("input");
    input.className = "form-control";
    input.style.width = "120px";
    input.type = "number";
    input.id = id;
    input.min = "0";

    formDiv.appendChild(label);
    formDiv.appendChild(input);

    workingDiv.appendChild(formDiv);
  }
}

holdingSendW.addEventListener('click', () => {
  const addr = holdingAddressW.value;
  if (addr === '' || Number(addr) < 0) {
    holdingStatusW.innerHTML = newStatus("alert-warning", `Please enter a valid holding register address (0 or higher).`);
    return;
  }
  const val = holdingValueW.value;
  writeHolding(addr, val);
});

holdingSendWM.addEventListener('click', () => {
  const address = parseInt(holdingAddressWM.value);
  const quantity = parseInt(holdingQuantityWM.value);

  if (isNaN(address) || address < 0) {
    holdingStatusWM.innerHTML = newStatus("alert-warning", `Please enter a valid holding register address (0 or higher).`);
    return;
  }
  if (isNaN(quantity) || quantity < 1) {
    holdingStatusWM.innerHTML = newStatus("alert-warning", `Please enter a valid quantity to write (1 or higher).`);
    return;
  }

  const valueArray = [];
  for (var i = address; i < address + quantity; i++) {
    const inputElement = document.getElementById(`holdingInput-${i}`);
    const inputValue = inputElement.value;
    valueArray.push(inputValue);
  }

  writeMultipleHolding(address, valueArray); // handles value gathering in here
});

holdingSendR.addEventListener('click', () => {
  const addr = holdingAddressR.value;
  const quan = holdingQuantityR.value;

  const headingRow = document.getElementById("holdingResultTableR_heading");
  const valueRow = document.getElementById("holdingResultTableR_value");
  headingRow.innerHTML = "";
  valueRow.innerHTML = "";

  if (addr === '' || Number(addr) < 0) {
    holdingStatusR.innerHTML = newStatus("alert-warning", `Please enter a valid holding register address (0 or higher).`);
    return;
  }
  if (quan === '' || Number(quan) < 1) {
    holdingStatusR.innerHTML = newStatus("alert-warning", `Please enter a valid quantity to read (1 or higher).`);
    return;
  }
  readHolding(addr, quan);
});

holdingQuantityWM.addEventListener('input', () => {
  updateWMInputs();
});

holdingAddressWM.addEventListener('input', () => {
  updateWMInputs();
});