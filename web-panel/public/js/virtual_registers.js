const vrNew = document.getElementById('vrNew');
const vrNewSubmit = document.getElementById('vrNewSubmit');
const vrNewType = document.getElementById('vrNewType');
const vrNewControls = document.getElementById('vrNewControls');
const vrNewStatus = document.getElementById('vrNewStatus');

const vrCount = document.getElementById('vrCount');
const vrList = document.getElementById('vrList');
const vrListRefresh = document.getElementById('vrListRefresh');
const vrListRefreshBusy = document.getElementById('vrListRefreshBusy');
const vrListStatus = document.getElementById('vrListStatus');

const vrListViewArea = document.getElementById('vrListViewArea');
const vrListViewAreaLabel = document.getElementById('vrListViewAreaLabel');
const vrListViewAreaValueRead = document.getElementById('vrListViewAreaValueRead');
const vrListViewAreaBtnRead = document.getElementById('vrListViewAreaBtnRead');
const vrListViewAreaValueWrite = document.getElementById('vrListViewAreaValueWrite');
const vrListViewAreaBtnWrite = document.getElementById('vrListViewAreaBtnWrite');

let vrCountCoils = 0;
let vrCountHolding = 0;

let selectedvr = null;
let selectedvrtype = null;

vrNew.addEventListener('click', () => {
  vrNew.classList.add("d-none");
  vrNewControls.classList.remove("d-none");
});

vrNewType.addEventListener('change', () => {
  vrNewStatus.innerHTML = "";
  if (vrNewType.selectedIndex != 0) {
    vrNewSubmit.classList.remove("btn-secondary");
    vrNewSubmit.classList.add("btn-primary");
  } else {
    vrNewSubmit.classList.add("btn-secondary");
    vrNewSubmit.classList.remove("btn-primary");
  }
});

vrNewSubmit.addEventListener('click', async () => {
  vrNewStatus.innerHTML = "";
  if (vrNewSubmit.classList.contains("btn-secondary")) {
    vrNewStatus.innerHTML = newStatus("alert-warning", `Please select a register type.`);
    return;
  }
  const registerType = vrNewType.selectedIndex; // ensure index 0 is a placeholder, e.g., "select"
  await vrNewRequest(registerType);

  vrListUpdate();
});

async function vrNewRequest(registerType) {
  try {
    const response = await fetch('/addvr', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ type: parseInt(registerType) })
    });

    const data = await response.json();

    vrNewStatus.innerHTML = "";
    if (response.ok) {
      const data = await response.json();

      const statusmsg = `A virtual register of type ${vrNewType.value} was successfully created.`;
      vrNewStatus.innerHTML = newStatus("alert-success", statusmsg);

      // cleanup
      vrNewSubmit.classList.remove("btn-primary");
      vrNewSubmit.classList.add("btn-secondary");
      vrNewControls.classList.add("d-none");
      vrNewType.selectedIndex = 0;
      vrNew.classList.remove("d-none");
    } else {
      vrNewStatus.innerHTML = newStatus("alert-danger", `Error: ${data.error || 'Unknown error'}`);
    }
  } catch (err) {
      vrNewStatus.innerHTML = newStatus("alert-danger", `Server error: ${err.message}`);
  }
}

async function vrCountUpdate() {
  if (isConnected) {
    let url = `/countvr?type=1`; // coils count
    let url2 = `/countvr?type=2`; // holding count

    let response = await fetch(url);
    let data = await response.json();
    vrCountCoils = parseInt(data.count);

    response = await fetch(url2);
    data = await response.json();
    vrCountHolding = parseInt(data.count);

    let coilsText = (vrCountCoils == 1 ?  "virtual coil." : "virtual coils."); // non plural / plural
    let holdingText = (vrCountHolding == 1 ?  "virtual holding register." : "virtual holding registers."); // non plural / plural
    vrCount.innerHTML = `${vrCountCoils} ${coilsText}<br>${vrCountHolding} ${holdingText}`;
  } else {
    ruleCount.innerText = "Please initiate a serial connection to view device status.";
  }
}

async function vrListUpdate() {
  // Pause status polling as this repeated modbus TX and RX can take some time
  await statusPolling(false);

  selectedvr = null;
  selectedvrtype = null;
  
  vrListRefresh.classList.add("d-none");
  vrListRefreshBusy.classList.remove("d-none");

  vrList.innerHTML = "";

  if (isConnected) {
    await vrCountUpdate();
    ruleListStatus.innerHTML = '';
    for (let i = 0; i < vrCountCoils; i++) {
      // Update the ruleList
      const item = document.createElement("button");
      item.className = "list-group-item list-group-item-action";
      item.textContent = `Virtual Coil ${i}`;
      item.addEventListener("click", () => viewvr(i, 1));
      vrList.appendChild(item);
    }
    for (let i = 0; i < vrCountHolding; i++) {
      // Update the ruleList
      const item = document.createElement("button");
      item.className = "list-group-item list-group-item-action";
      item.textContent = `Virtual Holding Register ${i}`;
      item.addEventListener("click", () => viewvr(i, 2));
      vrList.appendChild(item);
    }

    vrListViewArea.classList.add("d-none");
    
    await statusPolling(true);

    vrListRefresh.classList.remove("d-none");
    vrListRefreshBusy.classList.add("d-none");
  }
}

async function viewvr(i, type) {
  if (isNaN(i) || i < 0 ) {
    vrListStatus.innerHTML = newStatus("alert-danger", `Invalid stored virtual register address ${selectedvr}`);
  } else if (isNaN(type) || type < 1 || type > 2) {
    vrListStatus.innerHTML = newStatus("alert-danger", `Invalid stored virtual register type ${selectedvrtype}. Should only be 1 or 2.`);
    return;
  }

  selectedvr = i;
  selectedvrtype = type;

  vrListViewAreaValueRead.value = "";
  vrListViewAreaValueWrite.value = "";

  vrListViewArea.classList.remove("d-none");
  let typeText = (type == 1) ? "Coil" : "Holding Register";
  vrListViewAreaLabel.innerText = `Virtual ${typeText} ${i}:`;
}

vrListViewAreaBtnWrite.addEventListener('click', async () => {
  if (selectedvr == null || selectedvrtype == null) {
    return;
  }

  const value = parseInt(vrListViewAreaValueWrite.value);

  if (isNaN(value) || value < 0) {
    vrListStatus.innerHTML = newStatus("alert-danger", "Please provide a valid write value (for coils, 0 or 1; for holding registers, 0-65535).");
    return;
  }

  const response = await fetch('/writevr', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
          type: parseInt(selectedvrtype),
          address: parseInt(selectedvr),
          value: value
      })
  });
  
  const data = await response.json();

  if (data.success == true) {
    vrListStatus.innerHTML = newStatus("alert-success", "Write successful.");
  } else {
    vrListStatus.innerHTML = newStatus("alert-danger", `Error: ${data.error}`);
  }
});

vrListViewAreaBtnRead.addEventListener('click', async () => {
  if (selectedvr == null || selectedvrtype == null) {
    return;
  }

  const url = `/readvr?type=${selectedvrtype}&address=${selectedvr}`;
  const response = await fetch(url);
  const data = await response.json();

  if (data.success == false) {
    vrListStatus.innerHTML = newStatus("alert-danger", `Error: ${data.error}`);
    return;
  }

  vrListViewAreaValueRead.value = data.value;
});

vrListRefresh.addEventListener('click', async () => {
  vrListUpdate();
  vrListViewArea.classList.add("d-none");
});