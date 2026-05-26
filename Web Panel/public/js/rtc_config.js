// RTC
const rtcStatus = document.getElementById("rtcStatus");
const rtctimezone = document.getElementById("rtctimezone");
const rtcset = document.getElementById("rtcset");

async function populateTimezones() {
  const timezones = Intl.supportedValuesOf?.("timeZone") || [];

  if (timezones.length === 0) {
    // Fallback to a list
    timezones.push("UTC", "Australia/Perth", "Europe/London", "Europe/Berlin", "America/New_York",
            "America/Los_Angeles", "Asia/Tokyo"
    );
  }

  timezones.forEach(tz => {
    const option = document.createElement("option");
    option.value = tz;
    option.textContent = tz;
    rtctimezone.appendChild(option);
  });

  // default to local timezone
  rtctimezone.value = Intl.DateTimeFormat().resolvedOptions().timeZone;
}



rtcset.addEventListener("click", async () => {
  const tz = rtctimezone.value;

  const response = await fetch('/setrtc', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ tz: tz })
  });
  
  const data = await response.json();
  
  if (data.success == true) {
    rtcStatus.innerHTML = newStatus("alert-success", `Successfully set the on-board RTC to the ${tz} timezone.`);
  } else {
    rtcStatus.innerHTML = newStatus("alert-danger", `Error: ${data.error}`);
  }
});

populateTimezones();