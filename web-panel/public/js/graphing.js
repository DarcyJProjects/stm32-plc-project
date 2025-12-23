const addGraphingItem = document.getElementById('addGraphingItem');
const removeGraphingItem = document.getElementById('removeGraphingItem');
const addGraphingItemStatus = document.getElementById('addGraphingItemStatus');
const removeGraphingItemStatus = document.getElementById('removeGraphingItemStatus');

const chartStartStop = document.getElementById('chartStartStop');
const chartResetZoom = document.getElementById('chartResetZoom');
const chartExport = document.getElementById('chartExport');

const addType = document.getElementById('registerSelect');
const addAddress = document.getElementById('graphAddress');

const context = document.getElementById("registerChart").getContext("2d");

const chart = new Chart(context, {
  type: 'line',
  data: {
    datasets: []
  },
  options: {
    responsive: true,
    animation: true,
    parsing: false,
    scales: {
      //x: { title: { display: true, text: 'Time' }},
      //y: { title: { display: true, text: 'Value' }}
      x: {
        type: 'time',
        time: {
          unit: 'second',
          tooltipFormat: 'HH:mm:ss'
        },
        title: { display: true, text: 'Time' },
        ticks: {
          autoSkip: false,
          source: 'auto'
        },
        adapters: {
          date: luxon.DateTime
        },
        min: Date.now() - 60000, // show past 60 seconds
        max: Date.now()
      },
      /*y: {
        title: { display: true, text: 'Value' }
      }*/
      yDigital: {
        type: 'linear',
        position: 'left',
        min: 0,
        max: 1,
        title: {
          display: true,
          text: 'Digital'
        },
        grid: {
          drawOnChartArea: false, // avoid grid overlap
          drawTicks: false,
          drawBorder: false,
        },
      },
      yAnalogue: {
        type: 'linear',
        position: 'right',
        //min: 0,
        //max: 65535,
        title: {
          display: true,
          text: 'Analogue'
        },
        grid: {
          drawTicks: true,
          drawBorder: true,
          drawOnChartArea: true,
        },
      }
    },
    plugins: {
      zoom: {
        pan: {
          enabled: true,
          mode: 'x',
          modifierKey: 'ctrl'  // Optional
        },
        zoom: {
          wheel: {
            enabled: true
          },
          pinch: {
            enabled: true
          },
          mode: 'x'
        }
      }
    },
    tooltip: {
      enabled: true,
      mode: 'nearest',
      intersect: false,
      callbacks: {
        label: function(context) {
          const label = context.dataset.label || '';
          const yValue = context.parsed.y;
          return `${label}: ${yValue}`;
        }
      }
    }
  }
});

let timer;
let addressToRead = 0; // SET ADDRESS
let quantity = 1;

async function fetchRegisterAndUpdateChart() {
  // Stop status polling as to not interfere with graphing requests
  await statusPolling(false);

  addGraphingItemStatus.innerHTML = ""; 

  try {
    for (const dataset of chart.data.datasets) {
      const address = dataset.address;
      const type = dataset.regType;

      const res = await fetch(`/read?type=${type}&address=${encodeURIComponent(address)}`);
      const data = await res.json();

      if (res.ok && data.data?.[0] !== undefined) {
        dataset.data.push({
          x: Date.now(),
          y: data.data[0]
        });
      } else {
        addGraphingItemStatus.innerHTML = newStatus("alert-danger", `Error: ${data.error || 'Unknown error'}`);
        return;
      }
    }
    chart.options.scales.x.min = Date.now() - 60000;
    chart.options.scales.x.max = Date.now();
    chart.update();
  } catch (err) {
    addGraphingItemStatus.innerHTML = newStatus("alert-danger", `Error: ${err.message}`);
    console.error("Chart update error:", err);
  }

  // restart
  await statusPolling(true);
}

function startPolling() {
  clearInterval(timer);
  const intervalSec = parseInt(document.getElementById('intervalInput').value || 5);
  if (intervalSec < 1) {
    graphStatus.innerHTML = newStatus("alert-warning", `Please enter a interval greater than or equal to 1 second.`);
    return;
  }

  if (chart.data.datasets.length === 0) {
    graphStatus.innerHTML = newStatus("alert-warning", `Please add a dataset to the graph.`);
    return;
  }

  graphStatus.innerHTML = "";
  timer = setInterval(fetchRegisterAndUpdateChart, intervalSec * 1000);

  // Change start to stop button
  chartStartStop.innerText = "Stop";
  chartStartStop.classList.remove("btn-success");
  chartStartStop.classList.add("btn-danger");
}

function endPolling() {
  clearInterval(timer);
  timer = null;

  // Change stop to start button
  chartStartStop.innerText = "Start";
  chartStartStop.classList.remove("btn-danger");
  chartStartStop.classList.add("btn-success");
}

function getRandomColour() {
  const r = Math.floor(Math.random() * 200);
  const g = Math.floor(Math.random() * 200);
  const b = Math.floor(Math.random() * 200);
  return `rgba(${r}, ${g}, ${b}, 1)`;
}

addGraphingItem.addEventListener('click', async () => {
  addGraphingItemStatus.innerHTML = "";
  removeGraphingItemStatus.innerHTML = "";

  let type;
  switch (addType.value) {
    case "Coil":
      type = "coil";
      break;
    case "Discrete Input":
      type = "discrete";
      break;
    case "Holding Register":
      type = "holding";
      break;
    case "Input Register":
      type = "input";
      break;
    default:
      addGraphingItemStatus.innerHTML = newStatus("alert-warning", `Please select a valid register type.`);
      return;
  }

  const address = addAddress.value;

  // Check and test validity
  if (address === '' || Number(address) < 0) {
    addGraphingItemStatus.innerHTML = newStatus("alert-warning", `Please enter a valid register address (0 or higher).`);
    return;
  }

  try {
    const url = `/read?type=${type}&address=${encodeURIComponent(address)}`;
    const res = await fetch(url);
    const data = await res.json();
    if (!res.ok) {
      addGraphingItemStatus.innerHTML = newStatus("alert-danger", `Error: ${data.error || 'Unknown error'}`);
      return;
    }
  } catch (err) {
    addGraphingItemStatus.innerHTML = newStatus("alert-danger", `Server error: ${err.message}`);
    return;
  }

  const yAxisID = (type === 'coil' || type === 'discrete') ? 'yDigital' : 'yAnalogue';

  // Create new dataset
  const newDataset = {
    label: `${addType.value} ${address}`,
    regType: type,
    address: parseInt(address), 
    data: [],
    borderColor: getRandomColour(),
    tension: 0.2,
    fill: false,
    yAxisID: yAxisID
  };

  // Push to chart
  chart.data.datasets.push(newDataset);
  chart.update();

  // Reset state
  addAddress.value = "";

  // Start polling
  startPolling();
});

removeGraphingItem.addEventListener('click', () => {
    removeGraphingItemStatus.innerHTML = "";

    let type;
    switch (addType.value) {
      case "Coil":
        type = "coil";
        break;
      case "Discrete Input":
        type = "discrete";
        break;
      case "Holding Register":
        type = "holding";
        break;
      case "Input Register":
        type = "input";
        break;
      default:
        removeGraphingItemStatus.innerHTML = newStatus("alert-warning", `Please select a valid register type.`);
        return;
    }

    const address = parseInt(addAddress.value);

    if (isNaN(address) || address < 0) {
      removeGraphingItemStatus.innerHTML = newStatus("alert-warning", `Please enter a valid register address (0 or higher).`);
      return;
    }

    // Label for comparsion (to find one to delete)
    const labelToRemove = `${addType.value} ${address}`;

    // Find dataset index
    const index = chart.data.datasets.findIndex(ds => ds.label === labelToRemove);

    if (index !== -1) {
      chart.data.datasets.splice(index, 1); // Remove the dataset

      if (chart.data.datasets.length === 0) {
        endPolling();
      }

      chart.update();
    } else {
      removeGraphingItemStatus.innerHTML = newStatus("alert-info", `Dataset not found.`);
    }

    addAddress.value = "";
});

document.getElementById('intervalInput').addEventListener('change', () => {
  if (chart.data.datasets.length > 0 && (timer)) { 
    startPolling();          
  } else {
    endPolling();
  }
});

chartStartStop.addEventListener('click', () => {
  if (chartStartStop.innerText == 'Start') {
    startPolling();
  } else {
    endPolling();
  }
});

chartResetZoom.addEventListener('click', () => {
  chart.resetZoom();
});

chartExport.addEventListener('click', () => {
  let csvContent = "data:text/csv;charset=utf-8,";

  chart.data.datasets.forEach(dataset => {
    csvContent += `\nDataset: ${dataset.label}\nTime,Value\n`;
    dataset.data.forEach(point => {
      const timestamp = new Date(point.x).toISOString();
      csvContent += `${timestamp},${point.y}\n`;
    });
  });

  const encodedUri = encodeURI(csvContent);
  const link = document.createElement("a");
  link.setAttribute('href', encodedUri);
  link.setAttribute('download', 'export.csv');
  document.body.appendChild(link);
  link.click();
  document.body.removeChild(link);
});