window.onload = async function () {
  await fetchPorts();
  await getStatus();
  if (isConnected) {
    updateUI(true, currentPort, currentSlave);
    statusPolling(true);
  }
  await ruleListUpdate();
  await vrListUpdate();
  await hwGetModes();
};