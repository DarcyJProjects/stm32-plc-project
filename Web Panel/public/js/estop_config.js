const esChannel = document.getElementById("esChannel");
const esInputMode = document.getElementById("esInputMode");
const esSet = document.getElementById("esSet");
const esStatus = document.getElementById("esStatus");

esSet.addEventListener("click", async () => {
  const channel = esChannel.value;
  const inputMode = esInputMode.selectedIndex;

  if (isNaN(channel) || channel > 0xFFFF || channel < 0 ) {
    frStatus.innerHTML = newStatus("alert-danger", `Error: The Discrete Input Address must be a valid.`);
    return;
  }

  const response = await fetch('/setemergencystop', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
          channel: parseInt(channel), // Ensure number
          inputMode: inputMode
      })
  });

  const data = await response.json();
  
  if (data.success == true) {
    esStatus.innerHTML = newStatus("alert-success", `Successfully configured the emergency stop button.`);
  } else {
    esStatus.innerHTML = newStatus("alert-danger", `Error: ${data.error}`);
  }
});