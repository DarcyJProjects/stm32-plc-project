var ain1Mode = document.getElementById("ain1Mode");
var ain2Mode = document.getElementById("ain2Mode");
var ain3Mode = document.getElementById("ain3Mode");
var ain4Mode = document.getElementById("ain4Mode");

var aout1Mode = document.getElementById("aout1Mode");
var aout2Mode = document.getElementById("aout2Mode");

var hwStatus = document.getElementById("hwStatus");

var hwmodesave = document.getElementById("hwmodesave");

let changed = [];

function modeChanged(element) {
  hwmodesave.classList.remove("btn-secondary");
  hwmodesave.classList.add("btn-primary");

  if (!changed.includes(element)) {
    changed.push(element);
  }
}

async function hwGetModes() {
  // Reset button
  hwmodesave.classList.remove("btn-primary");
  hwmodesave.classList.add("btn-secondary");

  // Reset changed array
  changed = [];

  if (!isConnected) {
    // Disable selection boxes
    ain1Mode.disabled = true;
    ain2Mode.disabled = true;
    ain3Mode.disabled = true;
    ain4Mode.disabled = true;
    aout1Mode.disabled = true;
    aout2Mode.disabled = true;
    return;
  }

  await statusPolling(false);

  // Get AIN modes
  for (var i = 0; i < 4; i++) {
    const url = `/getmode?type=input&address=${i}`;
    const response = await fetch(url);
    const data = await response.json();
    
    if (data.success == true) {
      hwSetInModeSelector(i+1, parseInt(data.mode));
    } else {
      vrListStatus.innerHTML = newStatus("alert-danger", `Error: ${data.error}`);
    }
  }

  // Get AOUT modes
  for (var i = 0; i < 2; i++) {
    const url = `/getmode?type=holding&address=${i}`;
    const response = await fetch(url);
    const data = await response.json();
    
    if (data.success == true) {
      hwSetOutModeSelector(i+1, parseInt(data.mode));
    } else {
      hwStatus.innerHTML = newStatus("alert-danger", `Error: ${data.error}`);
    }
  }

  // Enable selection boxes
  console.log("enabled");
  ain1Mode.disabled = false;
  ain2Mode.disabled = false;
  ain3Mode.disabled = false;
  ain4Mode.disabled = false;
  aout1Mode.disabled = false;
  aout2Mode.disabled = false;

  await statusPolling(true);
}

async function hwModeSet() {
  if (!isConnected) {
    // Reset changed array
    changed = [];
    // Reset button
    hwmodesave.classList.remove("btn-primary");
    hwmodesave.classList.add("btn-secondary");

    return;
  };

  await statusPolling(false);

  let successNum = 0;

  // Set modes that are changed
  for (const item of changed) {
    var type = "input";
    if (item == aout1Mode || item == aout2Mode) {
      type = "holding";
    }

    var mode = item.selectedIndex;

    var index = hwGetItemAddress(item);

    const url = `/setmode?type=${type}&address=${index}&mode=${mode}`;
    const response = await fetch(url);
    const data = await response.json();
    
    if (data.success == true) {
      hwStatus.innerHTML = newStatus("alert-success", `Sucessfully set modes of analogue hardware IO.`);
    } else {
      hwStatus.innerHTML = newStatus("alert-danger", `Error: ${data.error}`);
      return;
    }
  };

  // Enable selection boxes
  console.log("enabled");
  ain1Mode.disabled = false;
  ain2Mode.disabled = false;
  ain3Mode.disabled = false;
  ain4Mode.disabled = false;
  aout1Mode.disabled = false;
  aout2Mode.disabled = false;

  // Reset changed array
  changed = [];

  // Reset button
  hwmodesave.classList.remove("btn-primary");
  hwmodesave.classList.add("btn-secondary");

  await statusPolling(true);
}

function hwSetInModeSelector(index, mode) {
  switch (index) {
    case 1:
      ain1Mode.selectedIndex = mode;
      break;
    case 2:
      ain2Mode.selectedIndex = mode;
      break;
    case 3:
      ain3Mode.selectedIndex = mode;
      break;
    case 4:
      ain4Mode.selectedIndex = mode;
      break;
  }
}

function hwSetOutModeSelector(index, mode) {
  switch (index) {
    case 1:
      aout1Mode.selectedIndex = mode;
      break;
    case 2:
      aout2Mode.selectedIndex = mode;
      break;
  }
}

function hwGetItemAddress(item) {
  switch (item) {
    case ain1Mode:
      return 0;
    case ain2Mode:
      return 1;
    case ain3Mode:
      return 2;
    case ain4Mode:
      return 3;
    case aout1Mode:
      return 0;
    case aout2Mode:
      return 1;
  }
}

[ain1Mode, ain2Mode, ain3Mode, ain4Mode, aout1Mode, aout2Mode].forEach(el => {
  el.addEventListener("change", () => modeChanged(el));
});

hwmodesave.addEventListener("click", async () => hwModeSet());