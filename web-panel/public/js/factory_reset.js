const confirmFactoryResetButton = document.getElementById('confirmFactoryResetButton');
const frModal = new bootstrap.Modal(document.getElementById('factoryResetModal'));

const factoryResetBtn = document.getElementById("factoryResetBtn");
const frStatus = document.getElementById("frStatus");

factoryResetBtn.addEventListener("click", () => frModal.show());

confirmFactoryResetButton.addEventListener("click", async () => {
  frModal.hide();

  const url = `/factoryreset?confirmation=true`;
  const response = await fetch(url);
  const data = await response.json();
  
  if (data.success == true) {
    frStatus.innerHTML = newStatus("alert-success", `Successfully performed a factory reset.`);

    await ruleListUpdate();
    await vrListUpdate();
    await hwGetModes();
  } else {
    frStatus.innerHTML = newStatus("alert-danger", `Error: ${data.error}`);
  }
});