// Logic Rule Elements
const inputType1 = document.getElementById("inputType1");
const inputType2 = document.getElementById("inputType2");
const inputRegister1 = document.getElementById("inputRegister1");
const inputRegister2 = document.getElementById("inputRegister2");
const op1 = document.getElementById("op1");
const op2 = document.getElementById("op2");
const compareValue1 = document.getElementById("compareValue1");
const compareValue2 = document.getElementById("compareValue2");
const outputType = document.getElementById("outputType");
const outputRegister = document.getElementById("outputRegister");
const outputValue = document.getElementById("outputValue");

const rulePreview = document.getElementById("rulePreview");
const rulePreviewArea = document.getElementById("rulePreviewArea");
const ruleNew = document.getElementById('ruleNew');
const ruleNewIf1 = document.getElementById('ruleNewIf1');
const ruleJoin = document.getElementById('ruleJoin');
const ruleJoinDiv = document.getElementById('ruleJoinDiv');
const ruleNewIf2 = document.getElementById('ruleNewIf2');
const ruleNewOutput = document.getElementById('ruleNewOutput');
const ruleAddJoin = document.getElementById('ruleAddJoin');
const ruleSubmit = document.getElementById('ruleSubmit');
const ruleCancel = document.getElementById('ruleCancel');
const ruleStatus = document.getElementById('ruleStatus');
const ruleCount = document.getElementById('ruleCount');
const ruleList = document.getElementById('ruleList');

// Copy Rule Elements
const copyInputType = document.getElementById("copyInputType");
const copyInputRegister = document.getElementById("copyInputRegister");
const copyOutputType = document.getElementById("copyOutputType");
const copyOutputRegister = document.getElementById("copyOutputRegister");
const copyRulePreview = document.getElementById("copyRulePreview");
const copyRulePreviewArea = document.getElementById("copyRulePreviewArea");
const copyRuleNew = document.getElementById("copyRuleNew");
const copyRuleNewAction = document.getElementById("copyRuleNewAction");
const copyRuleSubmit = document.getElementById("copyRuleSubmit");
const copyRuleCancel = document.getElementById("copyRuleCancel");
const copyRuleStatus = document.getElementById("copyRuleStatus");
const copyRuleCount = document.getElementById("copyRuleCount");
const copyRuleList = document.getElementById("copyRuleList");

// Shared List & Delete Elements
const ruleListRefresh = document.getElementById('ruleListRefresh');
const ruleListRefreshBusy = document.getElementById('ruleListRefreshBusy');
const ruleListViewArea = document.getElementById('ruleListViewArea');
const ruleListViewAreaLabel = document.getElementById('ruleListViewAreaLabel');
const ruleListViewAreaContent = document.getElementById('ruleListViewAreaContent');
const ruleListStatus = document.getElementById('ruleListStatus');
const ruleListDelete = document.getElementById('ruleListDelete');
const confirmDeleteRuleButton = document.getElementById('confirmDeleteRuleButton');
const modalRTD = new bootstrap.Modal(document.getElementById('confirmDeleteRuleModal'));

let ruleToDeleteIndex = null;
let ruleToDeleteType = null; // 1 = Logic, 2 = Copy

let joinActive = false;
let ruleCountNum = 0;
let copyRuleCountNum = 0;
let ruleListArray = [];
let copyRuleListArray = [];

// Helper Functions
function getTypeString(type) {
  switch (type) {
    case "Coil": return "coil";
    case "Discrete Input": return "discrete";
    case "Holding Register": return "holding";
    case "Input Register": return "input";
    case "Virtual Coil": return "vir_coil";
    case "Virtual Holding Register": return "vir_holding";
    default: return "null";
  }
}

function getOperationString(operation) {
  switch (operation) {
    case "==": return "CMP_EQ";
    case "!=": return "CMP_NEQ";
    case ">": return "CMP_GT";
    case "<": return "CMP_LT";
    case ">=": return "CMP_GTET";
    case "<=": return "CMP_LTET";
    default: return "null";
  }
}

function getJoinIndex(joinString) {
  switch (joinString.toLowerCase()) {
    case "and": return 2;
    case "or": return 3;
    default: return 0;
  }
}

function getJoinStringFromIndex(joinIndex) {
  switch (joinIndex) {
    case 2: return "LOGIC_AND";
    case 3: return "LOGIC_OR";
    default: return "LOGIC_NONE";
  }
}

function getTypeIndex(typeString) {
  switch (typeString.toLowerCase()) {
    case "coil": return 1;
    case "discrete input": return 2;
    case "holding register": return 3;
    case "input register": return 4;
    case "virtual coil": return 5;
    case "virtual holding register": return 6;
    default: return 0;
  }
}

function getTypeStringFromIndex(typeIndex) {
  switch (typeIndex) {
    case 1: return "COIL";
    case 2: return "DISCRETE";
    case 3: return "HOLDING";
    case 4: return "INPUT";
    case 5: return "VIR_COIL";
    case 6: return "VIR_HOLDING";
    default: return "NULL";
  }
}

function getOperationIndex(operationString) {
  switch (operationString.toLowerCase()) {
    case "==": return 1;
    case "!=": return 2;
    case ">": return 3;
    case "<": return 4;
    case ">=": return 5;
    case "<=": return 6;
    default: return 0;
  }
}

function getOperationStringFromIndex(operationIndex) {
  switch (operationIndex) {
    case 1: return "CMP_EQ";
    case 2: return "CMP_NEQ";
    case 3: return "CMP_GT";
    case 4: return "CMP_LT";
    case 5: return "CMP_GTET";
    case 6: return "CMP_LTET";
    default: return "NULL";
  }
}

// Logic Rule UI Updates
function updateRulePreview() {
  let preview;
  if (joinActive) {
    preview = `rule = (LogicRule){\n\tREG_${getTypeString(inputType1.value).toUpperCase()}, ${inputRegister1.value}, ${getOperationString(op1.value)}, ${compareValue1.value},\n\tREG_${getTypeString(inputType2.value).toUpperCase()}, ${inputRegister2.value}, ${getOperationString(op2.value)}, ${compareValue2.value},\n\tLOGIC_${ruleJoin.value.toUpperCase()},\n\tREG_${getTypeString(outputType.value).toUpperCase()}, ${outputRegister.value}, ${outputValue.value}\n};`;
  } else {
    preview = `rule = (LogicRule){\n\tREG_${getTypeString(inputType1.value).toUpperCase()}, ${inputRegister1.value}, ${getOperationString(op1.value)}, ${compareValue1.value},\n\tREG_${getTypeString(outputType.value).toUpperCase()}, ${outputRegister.value}, ${outputValue.value}\n};`;
  }
  rulePreview.innerHTML = preview;
}

function newRuleProcessInput() {
  updateRulePreview();
  if (joinActive) {
    if (inputType2.value == "Select" || Number(inputRegister2.value) < 0 || inputRegister2.value === "" || op2.value == "Select" || Number(compareValue2) < 0 || compareValue2.value === "") {
      ruleSubmit.classList.add("d-none");
      return;
    }
  }
  if (inputType1.value == "Select" || Number(inputRegister1.value) < 0 || inputRegister1.value === "" || op1.value == "Select" || Number(compareValue1) < 0 || compareValue1.value === "" || outputType.value == "Select" || Number(outputRegister.value) < 0 || outputRegister.value === "" || Number(outputValue.value) < 0 || outputValue.value === "") {
    ruleSubmit.classList.add("d-none");
    return;
  }
  ruleSubmit.classList.remove("d-none");
}

// Copy Rule UI Updates
function updateCopyRulePreview() {
  const preview = `copyRule = (CopyRule){\n\tREG_${getTypeString(copyInputType.value).toUpperCase()}, ${copyInputRegister.value},\n\tREG_${getTypeString(copyOutputType.value).toUpperCase()}, ${copyOutputRegister.value}\n};`;
  copyRulePreview.innerHTML = preview;
}

function newCopyRuleProcessInput() {
  updateCopyRulePreview();
  if (copyInputType.value == "Select" || Number(copyInputRegister.value) < 0 || copyInputRegister.value === "" || copyOutputType.value == "Select" || Number(copyOutputRegister.value) < 0 || copyOutputRegister.value === "") {
    copyRuleSubmit.classList.add("d-none");
    return;
  }
  copyRuleSubmit.classList.remove("d-none");
}

// Listeners - Logic Rule Designer
ruleNew.addEventListener('click', () => {
  ruleNew.classList.add("d-none");
  ruleNewIf1.classList.remove("d-none");
  ruleNewOutput.classList.remove("d-none");
  rulePreviewArea.classList.remove("d-none");
  ruleCancel.classList.remove("d-none");
  ruleStatus.innerHTML = "";
});

ruleAddJoin.addEventListener('click', () => {
  if (!joinActive) {
    ruleJoinDiv.classList.remove("d-none");
    ruleNewIf2.classList.remove("d-none");
    ruleSubmit.classList.add("d-none");
    joinActive = true;
    newRuleProcessInput();
    ruleAddJoin.innerText = "Remove Join";
    ruleAddJoin.classList.remove("btn-outline-primary");
    ruleAddJoin.classList.add("btn-danger");
  } else {
    ruleJoinDiv.classList.add("d-none");
    ruleNewIf2.classList.add("d-none");
    joinActive = false;
    newRuleProcessInput();
    ruleAddJoin.innerText = "Add Join";
    ruleAddJoin.classList.add("btn-outline-primary");
    ruleAddJoin.classList.remove("btn-danger");
  }
});

// Listeners - Copy Rule Designer
copyRuleNew.addEventListener('click', () => {
  copyRuleNew.classList.add("d-none");
  copyRuleNewAction.classList.remove("d-none");
  copyRulePreviewArea.classList.remove("d-none");
  copyRuleCancel.classList.remove("d-none");
  copyRuleStatus.innerHTML = "";
});

// Submit Logic Rule
ruleSubmit.addEventListener('click', async () => {
  try {
    ruleStatus.textContent = 'Sending request...';
    const payload = {
      rule_type: 1, // 1 = Logic Rule
      input_type1: getTypeIndex(inputType1.value),
      input_reg1: parseInt(inputRegister1.value),
      op1: getOperationIndex(op1.value),
      compare_value1: parseInt(compareValue1.value),
      join: joinActive ? getJoinIndex(ruleJoin.value) : 1, 
      output_type: getTypeIndex(outputType.value),
      output_reg: parseInt(outputRegister.value),
      output_value: parseInt(outputValue.value)
    };
    if (joinActive) {
      payload.input_type2 = getTypeIndex(inputType2.value);
      payload.input_reg2 = parseInt(inputRegister2.value);
      payload.op2 = getOperationIndex(op2.value);
      payload.compare_value2 = parseInt(compareValue2.value);
    }
    
    const response = await fetch('/addrule', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload)
    });
    const data = await response.json();
    if (response.ok) {
      ruleStatus.innerHTML = newStatus("alert-success", `Successfully added the Logic rule.`);
      ruleCleanup();
      ruleListUpdate();
    } else {
      ruleStatus.innerHTML = newStatus("alert-danger", `Error: ${data.error || 'Unknown error'}`);
    }
  } catch (err) {
      ruleStatus.innerHTML = newStatus("alert-danger", `Server error: ${err.message}`);
  }
});

// Submit Copy Rule
copyRuleSubmit.addEventListener('click', async () => {
  try {
    copyRuleStatus.textContent = 'Sending request...';
    const payload = {
      rule_type: 2, // 2 = Copy Rule
      input_type: getTypeIndex(copyInputType.value),
      input_reg: parseInt(copyInputRegister.value),
      output_type: getTypeIndex(copyOutputType.value),
      output_reg: parseInt(copyOutputRegister.value)
    };
    
    const response = await fetch('/addrule', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload)
    });
    const data = await response.json();
    if (response.ok) {
      copyRuleStatus.innerHTML = newStatus("alert-success", `Successfully added the Copy rule.`);
      copyRuleCleanup();
      ruleListUpdate();
    } else {
      copyRuleStatus.innerHTML = newStatus("alert-danger", `Error: ${data.error || 'Unknown error'}`);
    }
  } catch (err) {
      copyRuleStatus.innerHTML = newStatus("alert-danger", `Server error: ${err.message}`);
  }
});

// Cleanups
function ruleCleanup() {
  ruleNew.classList.remove("d-none");
  ruleNewIf1.classList.add("d-none");
  ruleNewOutput.classList.add("d-none");
  ruleNewIf2.classList.add("d-none");
  rulePreviewArea.classList.add("d-none");
  rulePreview.innerHTML = "";
  ruleCancel.classList.add("d-none");
  inputType1.selectedIndex = 0;
  inputType2.selectedIndex = 0;
  inputRegister1.value = "";
  inputRegister2.value = "";
  op1.selectedIndex = 0;
  op2.selectedIndex = 0;
  compareValue1.value = "";
  compareValue2.value = "";
  outputType.selectedIndex = 0;
  outputRegister.value = "";
  outputValue.value = "";
  ruleJoin.selectedIndex = 0;
  ruleStatus.innerHTML = "";
  joinActive = false;
  ruleAddJoin.innerText = "Add Join";
  ruleAddJoin.classList.add("btn-outline-primary");
  ruleAddJoin.classList.remove("btn-danger");
  ruleJoinDiv.classList.add("d-none");
  ruleSubmit.classList.add("d-none");
}

function copyRuleCleanup() {
  copyRuleNew.classList.remove("d-none");
  copyRuleNewAction.classList.add("d-none");
  copyRulePreviewArea.classList.add("d-none");
  copyRulePreview.innerHTML = "";
  copyRuleCancel.classList.add("d-none");
  copyInputType.selectedIndex = 0;
  copyInputRegister.value = "";
  copyOutputType.selectedIndex = 0;
  copyOutputRegister.value = "";
  copyRuleStatus.innerHTML = "";
  copyRuleSubmit.classList.add("d-none");
}

ruleCancel.addEventListener('click', () => ruleCleanup());
copyRuleCancel.addEventListener('click', () => copyRuleCleanup());

// Updating Lists and Counts
async function ruleCountUpdate() {
  if (isConnected) {
    // Get Logic Rule Count
    let res1 = await fetch("/getrulecount?rule_type=1");
    let data1 = await res1.json();
    ruleCountNum = parseInt(data1.data);
    ruleCount.innerText = `${ruleCountNum}`;

    // Get Copy Rule Count
    let res2 = await fetch("/getrulecount?rule_type=2");
    let data2 = await res2.json();
    copyRuleCountNum = parseInt(data2.data);
    copyRuleCount.innerText = `${copyRuleCountNum}`;
  } else {
    ruleCount.innerText = "--";
    copyRuleCount.innerText = "--";
  }
}

async function ruleListUpdate() {
  await statusPolling(false);

  ruleToDeleteIndex = null;
  ruleToDeleteType = null;
  ruleListViewArea.classList.add("d-none");
  
  ruleListRefresh.classList.add("d-none");
  ruleListRefreshBusy.classList.remove("d-none");

  let ruleListStatusBuffer = "";
  ruleList.innerHTML = "";
  copyRuleList.innerHTML = "";
  ruleListArray.length = 0;
  copyRuleListArray.length = 0;

  if (isConnected) {
    await ruleCountUpdate();
    ruleListStatus.innerHTML = '';
    
    // Fetch Logic Rules
    for (let i = 0; i < ruleCountNum; i++) {
      let url = `/getrule?rule_type=1&index=${i}`;
      const response = await fetch(url);
      const data = await response.json();
      if (!response.ok) {
        ruleListStatusBuffer += `Error fetching logic rule ${i}: ${data.error || 'Unknown error'}<br>`;
        continue;
      }
      ruleListArray.push(data);
      const item = document.createElement("button");
      item.className = "list-group-item list-group-item-action list-group-item-sm";
      item.textContent = `Logic Rule ${i}`;
      item.addEventListener("click", () => viewRule(1, i));
      ruleList.appendChild(item);
    }
    
    // Fetch Copy Rules
    for (let i = 0; i < copyRuleCountNum; i++) {
      let url = `/getrule?rule_type=2&index=${i}`;
      const response = await fetch(url);
      const data = await response.json();
      if (!response.ok) {
        ruleListStatusBuffer += `Error fetching copy rule ${i}: ${data.error || 'Unknown error'}<br>`;
        continue;
      }
      copyRuleListArray.push(data);
      const item = document.createElement("button");
      item.className = "list-group-item list-group-item-action list-group-item-sm";
      item.textContent = `Copy Rule ${i}`;
      item.addEventListener("click", () => viewRule(2, i));
      copyRuleList.appendChild(item);
    }

    if (ruleListStatusBuffer != "") {
      ruleListStatus.innerHTML = newStatus("alert-danger", `${ruleListStatusBuffer}`);
    }

    await statusPolling(true);
    ruleListRefresh.classList.remove("d-none");
    ruleListRefreshBusy.classList.add("d-none");
  }
}

// Viewing rules
function viewRule(type, index) {
  ruleToDeleteIndex = index;
  ruleToDeleteType = type;
  ruleListViewArea.classList.remove("d-none");

  if (type === 1) { // Logic Rule
    ruleListViewAreaLabel.innerHTML = `<b>Viewing Logic Rule ${index}:</b>`;
    const ruleData = ruleListArray[index].data;
    let preview;
    if (parseInt(ruleData.joinRaw) != 1) {
      preview = `rule = (LogicRule){\n\tREG_${getTypeStringFromIndex(ruleData.input_type1Raw)}, ${ruleData.input_reg1}, ${getOperationStringFromIndex(ruleData.op1Raw)}, ${ruleData.compare_value1},\n\tREG_${getTypeStringFromIndex(ruleData.input_type2Raw)}, ${ruleData.input_reg2}, ${getOperationStringFromIndex(ruleData.op2Raw)}, ${ruleData.compare_value2},\n\tLOGIC_${getJoinStringFromIndex(ruleData.joinRaw)},\n\tREG_${getTypeStringFromIndex(ruleData.output_typeRaw)}, ${ruleData.output_reg}, ${ruleData.output_value}\n};`;
    } else {
      preview = `rule = (LogicRule){\n\tREG_${getTypeStringFromIndex(ruleData.input_type1Raw)}, ${ruleData.input_reg1}, ${getOperationStringFromIndex(ruleData.op1Raw)}, ${ruleData.compare_value1},\n\tREG_${getTypeStringFromIndex(ruleData.output_typeRaw)}, ${ruleData.output_reg}, ${ruleData.output_value}\n};`;
    }
    ruleListViewAreaContent.innerHTML = preview;

  } else if (type === 2) { // Copy Rule
    ruleListViewAreaLabel.innerHTML = `<b>Viewing Copy Rule ${index}:</b>`;
    const ruleData = copyRuleListArray[index].data;
    const preview = `copyRule = (CopyRule){\n\tREG_${getTypeStringFromIndex(ruleData.input_typeRaw)}, ${ruleData.input_reg},\n\tREG_${getTypeStringFromIndex(ruleData.output_typeRaw)}, ${ruleData.output_reg}\n};`;
    ruleListViewAreaContent.innerHTML = preview;
  }
}

// Deleting Rules
ruleListDelete.addEventListener('click', () => {
    if(ruleToDeleteIndex !== null) modalRTD.show()
});

confirmDeleteRuleButton.addEventListener('click', () => {
  if (ruleToDeleteIndex !== null && ruleToDeleteType !== null) {
    deleteRule(ruleToDeleteType, ruleToDeleteIndex);
    ruleToDeleteIndex = null;
    ruleToDeleteType = null;
    bootstrap.Modal.getInstance(document.getElementById('confirmDeleteRuleModal')).hide();
  }
});

async function deleteRule(type, index) {
  try{
    if (isConnected) {
      const response = await fetch('/deleterule', {
          method: 'DELETE',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ rule_type: type, index: index })
      });
      const data = await response.json();

      if (response.ok) {
        await ruleListUpdate();
        const ruleTypeName = type === 1 ? "Logic" : "Copy";
        ruleListStatus.innerHTML = newStatus("alert-success", `Successfully deleted ${ruleTypeName} Rule with ID=${index}`);
        ruleListViewArea.classList.add("d-none");
      } else {
        ruleListStatus.innerHTML = newStatus("alert-danger", `Error: ${data.error || 'Unknown error'}`);
      }
    } else {
      ruleListStatus.innerHTML = newStatus("alert-warning", "No serial connection is currently active.");
    }
  } catch (err) {
      ruleStatus.innerHTML = newStatus("alert-danger", `Server error: ${err.message}`);
  }
}

// Event Listeners for UI interaction
inputType1.addEventListener('change', () => newRuleProcessInput());
inputType2.addEventListener('change', () => newRuleProcessInput());
inputRegister1.addEventListener('input', () => newRuleProcessInput());
inputRegister2.addEventListener('input', () => newRuleProcessInput());
op1.addEventListener('change', () => newRuleProcessInput());
op2.addEventListener('change', () => newRuleProcessInput());
compareValue1.addEventListener('input', () => newRuleProcessInput());
compareValue2.addEventListener('input', () => newRuleProcessInput());
outputType.addEventListener('change', () => newRuleProcessInput());
outputRegister.addEventListener('input', () => newRuleProcessInput());
outputValue.addEventListener('input', () => newRuleProcessInput());
ruleJoin.addEventListener('change', () => newRuleProcessInput());

copyInputType.addEventListener('change', () => newCopyRuleProcessInput());
copyInputRegister.addEventListener('input', () => newCopyRuleProcessInput());
copyOutputType.addEventListener('change', () => newCopyRuleProcessInput());
copyOutputRegister.addEventListener('input', () => newCopyRuleProcessInput());

ruleListRefresh.addEventListener('click', () => {
  ruleListUpdate();
  ruleListViewAreaContent.innerHTML = "";
  ruleListViewArea.classList.add("d-none");
});