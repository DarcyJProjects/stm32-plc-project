const inputAddressR = document.getElementById('inputAddressR');
const inputSendR = document.getElementById('inputSendR');
const inputStatusR = document.getElementById('inputStatusR');

// Read
async function readInput(address, quantity) {
  try {
    const addr = parseInt(address);
    const quan = parseInt(quantity);

    inputStatusR.textContent = 'Sending request...';
    const url = `/read?type=input&address=${encodeURIComponent(address)}&quantity=${quantity}`;
    console.log(url);
    const response = await fetch(url);
    const data = await response.json();

    if (response.ok) {
      inputStatusR.innerHTML = "";
      const inputValues = data.data;

      const headingRow = document.getElementById("inputResultTableR_heading");
      const valueRow = document.getElementById("inputResultTableR_value");

      for (let i = addr; i < addr + quan; i++) {
        // Heading Cell
        const cell = document.createElement('td'); // create a table cell
        cell.textContent = i; // set cell content
        headingRow.appendChild(cell); // add cell to row

        // Value cell
        const cell2 = document.createElement('td');
        cell2.textContent = inputValues[i - addr];
        valueRow.appendChild(cell2);
      }
    } else {
      inputStatusR.innerHTML = newStatus("alert-danger", `Error: ${data.error || 'Unknown error'}`);
    }
  } catch (err) {
      inputStatusR.innerHTML = newStatus("alert-danger", `Server error: ${err.message}`);
  }
}

inputSendR.addEventListener('click', () => {
  const addr = inputAddressR.value;
  const quan = inputQuantityR.value;

  const headingRow = document.getElementById("inputResultTableR_heading");
  const valueRow = document.getElementById("inputResultTableR_value");
  headingRow.innerHTML = "";
  valueRow.innerHTML = "";

  if (addr === '' || Number(addr) < 0) {
    inputStatusR.innerHTML = newStatus("alert-warning", `Please enter a valid input register address (0 or higher).`);
    return;
  }
  if (quan === '' || Number(quan) < 1) {
    inputStatusR.innerHTML = newStatus("alert-warning", `Please enter a valid quantity to read (1 or higher).`);
    return;
  }
  readInput(addr, quan);
});