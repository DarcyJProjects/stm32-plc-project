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

const ruleStatus = document.getElementById('ruleStatus');

const ruleCount = document.getElementById('ruleCount');

const ruleList = document.getElementById('ruleList');
const ruleListRefresh = document.getElementById('ruleListRefresh');
const ruleListRefreshBusy = document.getElementById('ruleListRefreshBusy');
const ruleListViewArea = document.getElementById('ruleListViewArea');
const ruleListViewAreaLabel = document.getElementById('ruleListViewAreaLabel');
const ruleListViewAreaContent = document.getElementById('ruleListViewAreaContent');
const ruleListStatus = document.getElementById('ruleListStatus');

const ruleCancel = document.getElementById('ruleCancel');

const ruleListDelete = document.getElementById('ruleListDelete');
const confirmDeleteRuleButton = document.getElementById('confirmDeleteRuleButton');
const modalRTD = new bootstrap.Modal(document.getElementById('confirmDeleteRuleModal'));

let ruleToDelete = null;

let joinActive = false;
let ruleCountNum = 0;
let ruleListArray = [];

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

ruleSubmit.addEventListener('click', async () => {
  try {
    ruleStatus.textContent = 'Sending request...';

    const payload = {
      input_type1: getTypeIndex(inputType1.value),
      input_reg1: parseInt(inputRegister1.value),
      op1: getOperationIndex(op1.value),
      compare_value1: parseInt(compareValue1.value),
      join: joinActive ? getJoinIndex(ruleJoin.value) : 1, // 1 = NONE
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
      ruleStatus.innerHTML = newStatus("alert-success", `Successfully added the rule.`);

      ruleCleanup();
      ruleListUpdate();
    } else {
      ruleStatus.innerHTML = newStatus("alert-danger", `Error: ${data.error || 'Unknown error'}`);
    }
  } catch (err) {
      ruleStatus.innerHTML = newStatus("alert-danger", `Server error: ${err.message}`);
  }
});

function ruleCleanup() {
  // Clean up
  
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

ruleCancel.addEventListener('click', () => {
  ruleCleanup();
});

async function ruleCountUpdate() {
  if (isConnected) {
    let url = "/getrulecount";
    const response = await fetch(url);
    const data = await response.json();
    ruleCountNum = parseInt(data.data);
    let ruleText = "rule is";
    if (ruleCountNum > 1 || ruleCountNum == 0) ruleText = "rules are";
    ruleCount.innerText = `${ruleCountNum} ${ruleText} active.`;
  } else {
    ruleCount.innerText = "Please initiate a serial connection to view device status.";
  }
}

async function ruleListUpdate() {
  // Pause status polling as this repeated modbus TX and RX can take some time
  await statusPolling(false);

  ruleToDelete = null;
  ruleListDelete.classList.add("d-none");
  
  ruleListRefresh.classList.add("d-none");
  ruleListRefreshBusy.classList.remove("d-none");

  let ruleListStatusBuffer = "";
  ruleList.innerHTML = "";
  ruleListArray.length = 0;

  if (isConnected) {
    await ruleCountUpdate();
    ruleListStatus.innerHTML = '';
    for (let i = 0; i < ruleCountNum; i++) {
      // Actually store the rule data
      let url = `/getrule?index=${i}`;
      const response = await fetch(url);
      const data = await response.json();

      if (!response.ok) {
        ruleListStatusBuffer = ruleListStatusBuffer + `Error: ${data.error || 'Unknown error'}<br>`;
        continue;
      }

      ruleListArray.push(data);
      
      // Update the ruleList
      const item = document.createElement("button");
      item.className = "list-group-item list-group-item-action";
      item.textContent = `Rule ${i}`;
      item.addEventListener("click", () => viewRule(i));
      ruleList.appendChild(item);
    }
  

    if (ruleListArray.length != ruleCountNum) {
      ruleListStatusBuffer = ruleListStatusBuffer + `Error: The number of successfully read rules does not equal the reported number of rules.<br><br><b>If graph polling is active, try stopping it temporarily. If another instance (browser tab) of the web panel is open, it's status update requests (and if active, graph polling requests) will likely interfere.</b><br>`;
    }

    if (ruleListStatusBuffer != "") {
      ruleListStatus.innerHTML = newStatus("alert-danger", `${ruleListStatusBuffer}`);
      ruleList.innerHTML = "";
    }

    await statusPolling(true);

    ruleListRefresh.classList.remove("d-none");
    ruleListRefreshBusy.classList.add("d-none");
  }
}

function viewRule(index) {
  if (index >= ruleListArray.length) {
    ruleToDelete = null;
    return;
  }

  ruleToDelete = index;
  ruleListDelete.classList.remove("d-none");
  
  ruleListViewAreaLabel.innerHTML = `<b>Viewing Rule ${index}:</b>`;
  ruleListViewArea.classList.remove("d-none");

  const ruleData = ruleListArray[index].data;

  let preview;
  if (parseInt(ruleData.joinRaw) != 1) {
    preview = `rule = (LogicRule){\n\tREG_${getTypeStringFromIndex(ruleData.input_type1Raw)}, ${ruleData.input_reg1}, ${getOperationStringFromIndex(ruleData.op1Raw)}, ${ruleData.compare_value1},\n\tREG_${getTypeStringFromIndex(ruleData.input_type2Raw)}, ${ruleData.input_reg2}, ${getOperationStringFromIndex(ruleData.op2Raw)}, ${ruleData.compare_value2},\n\tLOGIC_${getJoinStringFromIndex(ruleData.joinRaw)},\n\tREG_${getTypeStringFromIndex(ruleData.output_typeRaw)}, ${ruleData.output_reg}, ${ruleData.output_value}\n};`;
  } else {
    preview = `rule = (LogicRule){\n\tREG_${getTypeStringFromIndex(ruleData.input_type1Raw)}, ${ruleData.input_reg1}, ${getOperationStringFromIndex(ruleData.op1Raw)}, ${ruleData.compare_value1},\n\tREG_${getTypeStringFromIndex(ruleData.output_typeRaw)}, ${ruleData.output_reg}, ${ruleData.output_value}\n};`;
  }
  ruleListViewAreaContent.innerHTML = preview;
}

confirmDeleteRuleButton.addEventListener('click', () => {
  if (ruleToDelete !== null) {
    deleteRule(ruleToDelete);

    ruleToDelete = null;
    bootstrap.Modal.getInstance(document.getElementById('confirmDeleteRuleModal')).hide();
  }
});

async function deleteRule(index) {
  try{
    if (isConnected) {
      const response = await fetch('/deleterule', {
          method: 'DELETE',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ index: index })
      });
      
      const data = await response.json();

      if (response.ok) {
        await ruleListUpdate();
        ruleListStatus.innerHTML = newStatus("alert-success", `Successfully deleted the rule with ID=${index}`);
        ruleListViewArea.classList.add("d-none");
      } else {
        ruleListStatus.innerHTML = newStatus("alert-danger", `Error: ${data.error || 'Unknown error'}`);
      }
    } else {
      ruleListStatus.innerText = "No serial connection is currently active.";
    }
  } catch (err) {
      ruleStatus.innerHTML = newStatus("alert-danger", `Server error: ${err.message}`);
  }
}

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

ruleListDelete.addEventListener('click', () => modalRTD.show());

ruleListRefresh.addEventListener('click', () => {
  ruleListUpdate();
  ruleListViewAreaContent.innerHTML = "";
  ruleListViewArea.classList.add("d-none");
});