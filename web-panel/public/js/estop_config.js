const esChannel = document.getElementById("esChannel");
const esInputMode = document.getElementById("esInputMode");
const esSet = document.getElementById("esSet");
const esStatus = document.getElementById("esStatus");

esSet.addEventListener("click", async () => {
  const channel = esChannel.value;
  const inputMode = esInputMode.selectedIndex;

  if (isNaN(channel) || channel > 0xFFFF || channel < 0 ) {
    frStatus.innerHTML = newStatus("alert-danger", `Error: The Discrete Input Address must be a valid.`);
  }

  const url = `/setemergencystop?channel=${channel}&inputMode=${inputMode}`;
  const response = await fetch(url);
  const data = await response.json();
  
  if (data.success == true) {
    esStatus.innerHTML = newStatus("alert-success", `Successfully configured the emergency stop button.`);
  } else {
    esStatus.innerHTML = newStatus("alert-danger", `Error: ${data.error}`);
  }
});