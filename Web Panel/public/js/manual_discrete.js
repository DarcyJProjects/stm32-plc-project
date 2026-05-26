const discreteAddressR = document.getElementById('discreteAddressR');
const discreteSendR = document.getElementById('discreteSendR');
const discreteStatusR = document.getElementById('discreteStatusR');

// Read
async function readDiscrete(address, quantity) {
  try {
    const addr = parseInt(address);
    const quan = parseInt(quantity);

    discreteStatusR.textContent = 'Sending request...';
    const url = `/read?type=discrete&address=${encodeURIComponent(address)}&quantity=${quantity}`;
    console.log(url);
    const response = await fetch(url);
    const data = await response.json();

    if (response.ok) {
      discreteStatusR.innerHTML = "";
      const discreteValues = data.data;

      const headingRow = document.getElementById("discreteResultTableR_heading");
      const valueRow = document.getElementById("discreteResultTableR_value");

      for (let i = addr; i < addr + quan; i++) {
        // Heading Cell
        const cell = document.createElement('td'); // create a table cell
        cell.textContent = i; // set cell content
        headingRow.appendChild(cell); // add cell to row

        // Value cell
        const cell2 = document.createElement('td');
        cell2.textContent = discreteValues[i - addr] ? "ON" : "OFF";
        valueRow.appendChild(cell2);
      }
    } else {
      discreteStatusR.innerHTML = newStatus("alert-danger", `Error: ${data.error || 'Unknown error'}`);
    }
  } catch (err) {
      discreteStatusR.innerHTML = newStatus("alert-danger", `Server error: ${err.message}`);
  }
}

discreteSendR.addEventListener('click', () => {
  const addr = discreteAddressR.value;
  const quan = discreteQuantityR.value;

  const headingRow = document.getElementById("discreteResultTableR_heading");
  const valueRow = document.getElementById("discreteResultTableR_value");
  headingRow.innerHTML = "";
  valueRow.innerHTML = "";

  if (addr === '' || Number(addr) < 0) {
    discreteStatusR.innerHTML = newStatus("alert-warning", `Please enter a valid discrete input address (0 or higher).`);
    return;
  }
  if (quan === '' || Number(quan) < 1) {
    discreteStatusR.innerHTML = newStatus("alert-warning", `Please enter a valid quantity to read (1 or higher).`);
    return;
  }
  readDiscrete(addr, quan);
});