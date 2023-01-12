// HTML Elements
const rebootBtn = document.getElementById("reboot");
const shutdownBtn = document.getElementById("shutdown");
const cancelBtn = document.getElementById("cancel");
const limitInput = document.querySelector(".limit-input");
const table = document.querySelector(".command-info-table");
const runForm = document.getElementById("run-form");
const killForm = document.getElementById("kill-form");
const refreshTime = document.getElementById("update-sec");
const utilSetting = document.getElementById("log-load");
const uptime = document.querySelector("#uptime");
const loadavg = document.querySelector("#load-avg");
const logsInput = document.getElementById("log-entries-input");
const clear = document.getElementById("clear");
const textarea = document.getElementById("logs-area");

// Functions
function sendRequest(target) {
  let xhr = new XMLHttpRequest();
  xhr.open("GET", target);
  xhr.send();
}

const delay = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

let logArray = [];
let inputsArray = [];
let tablearray = [];
let blockArray = [];
let historyArray = [];
let dsw;

const localDataBase = function () {
  if (sessionStorage.getItem("logs") === null) {
    logArray = [];
  } else {
    logArray = JSON.parse(sessionStorage.getItem("logs"));
  }

  if (sessionStorage.getItem("table") === null) {
    tablearray = [];
  } else {
    tablearray = JSON.parse(sessionStorage.getItem("table"));
  }

  if (sessionStorage.getItem("settings") === null) {
    inputsArray = [2, 70, 3];
  } else {
    inputsArray = JSON.parse(sessionStorage.getItem("settings"));
  }

  if (sessionStorage.getItem("blocks") === null) {
    blockArray = ["", ""];
  } else {
    blockArray = JSON.parse(sessionStorage.getItem("blocks"));
  }

  if (sessionStorage.getItem("history") === null) {
    historyArray = [];
  } else {
    historyArray = JSON.parse(sessionStorage.getItem("history"));
  }

  if (sessionStorage.getItem("dsw") === null) {
    dsw = true;
  } else {
    dsw = JSON.parse(sessionStorage.getItem("dsw"));
  }

  refreshTime.value = inputsArray[0];
  utilSetting.value = inputsArray[1];
  logsInput.value = inputsArray[2];

  document.getElementById("run").value = blockArray[0];
  document.getElementById("kill").value = blockArray[1];
};

localDataBase();

(function firstPageLoad() {
  refreshload();
  refreshUptime();
  refreshLoadavg();
  if (tablearray.length !== 0) {
    tablearray.forEach((row, index) => {
      createTableRow(
        row.pid,
        row.command,
        row.load,
        row.currentcpu,
        row.allowedcpu,
        index
      );
    });
  }
})();

function refreshload() {
  sendRequest("./refreshcpuload");
  const readFile = async () => {
    await fetch("../data_files/load.json").then((response) => {
      response.json().then((response) => {
        for (let i = 0; i < parseInt(response[0]); i++) {
          let cpuid = `#cpuload${i}`;
          const progress = document.querySelector(cpuid);
          if (parseInt(response[i + 1]) < 15) {
            progress.style.width = response[i + 1] + "%";
            progress.style.fontSize = "0.8rem";
            progress.innerText = response[i + 1] + "%";
            progress.parentElement.parentElement.removeAttribute("style");
            progress.style.opacity = 1;
          } else {
            progress.style.width = response[i + 1] + "%";
            progress.innerText = response[i + 1] + "%";
            progress.parentElement.parentElement.removeAttribute("style");
            progress.style.opacity = 1;
          }
        }
      });
    });
  };
  readFile();
}

function refreshUptime() {
  sendRequest("./refreshuptime");
  (async function () {
    await fetch("../data_files/uptime.json").then((response) => {
      response.json().then((response) => {
        uptimeVal = response[0].uptime;
        if (uptimeVal != undefined) {
          uptime.innerText = uptimeVal;
        }
      });
    });
  })();
}

function refreshLoadavg() {
  sendRequest("./refreshloadavg");
  (async function () {
    await fetch("../data_files/loadavg.json").then((response) => {
      response.json().then((response) => {
        loadavgVal = response[0].loadavg;
        if (loadavgVal != undefined) {
          loadavg.innerText = loadavgVal;
        }
      });
    });
  })();
}

function terminate() {
  const killinput = document.getElementById("kill");

  if (tablearray.length !== 0) {
    if (killinput.value % 1 == 0 && killinput.value != "") {
      sendRequest(`./terminate?${killinput.value}`);
      const index = tablearray.findIndex((element) => {
        createNewLog(
          `The process with ID '${element.pid}' has been terminated`
        );
        return element.pid == killinput.value;
      });
      tablearray.splice(index, 1);
      sessionStorage.setItem("table", JSON.stringify(tablearray));
    } else {
      sendRequest(`./terminatebyname?${killinput.value}`);

      const removedta = tablearray.filter((element, index) => {
        return element.command == killinput.value;
      });
      const newta = removedta.forEach((object) => {
        tablearray.splice(
          tablearray.findIndex((element) => {
            return element.pid == object.pid;
          }),
          1
        );
        sessionStorage.setItem("table", JSON.stringify(tablearray));
      });
      if (removedta) {
        createNewLog(
          `All processes named '${killinput.value}' have been terminated`
        );
      }
    }
  } else {
    if (killinput.value % 1 == 0 && killinput.value != "") {
      sendRequest(`./terminate?${killinput.value}`);
      createNewLog(
        `The process with ID '${killinput.value}' has been terminated`
      );
    } else {
      sendRequest(`./terminatebyname?${killinput.value}`);

      createNewLog(
        `All processes named '${killinput.value}' have been terminated`
      );
    }
  }
  killinput.value = "";
  blockArray[1] = "";
  sessionStorage.setItem("blocks", JSON.stringify(blockArray));
  document.location.reload(1);
}

function createNewLog(msg) {
  let current = new Date();
  logArray.push(`${current.toLocaleString()} - ${msg}.`);
  sessionStorage.setItem("logs", JSON.stringify(logArray));
}

function showAlert() {
  if (dsw) {
    if (
      confirm(
        "You must open the server console screen and confirm for root permission if necessary!"
      )
    ) {
      sessionStorage.setItem("dsw", "false");
      dsw = false;
    } else {
      // Do nothing!
    }
  }
}

// create table function
function createTableRow(pid, command, cpuload, currentcpu, allowedcpu, rid) {
  if (document.getElementById(rid) == null) {
    let newTR = document.createElement("tr");
    newTR.id = rid;
    newTR.innerHTML = `<td class="pid-cell">${pid}</td><td>${command}</td><td>${cpuload}%</td><td>${currentcpu}</td><td>${allowedcpu}</td><td><form class="affinity-form" action=""><input class="input affinity-input" type="text" placeholder="e.g. 1 | e.g. 0,1 (without space)" autocomplete="off"/></form></td><td><form class="limit-form" action=""><input class="input limit-input" type="text" placeholder="0-100 | enter to set" autocomplete="off"/></form></td>`;
    table.appendChild(newTR);
  } else {
    document.getElementById(
      rid
    ).innerHTML = `<td class="pid-cell">${pid}</td><td>${command}</td><td>${cpuload}%</td><td>${currentcpu}</td><td>${allowedcpu}</td><td><form class="affinity-form" action=""><input class="input affinity-input" type="text" placeholder="e.g. 1 | e.g. 0,1 (without space)" autocomplete="off"/></form></td><td><form class="limit-form" action=""><input class="input limit-input" type="text" placeholder="0-100 | enter to set" autocomplete="off"/></form></td>`;
  }
}

// Events
rebootBtn.addEventListener("click", (event) => {
  event.preventDefault();
  let minute = window.prompt(
    "After how many minutes will the system reboot?",
    "(empty for reboot now)"
  );
  if (minute == null) {
  } else if (minute === "") {
    sendRequest("./reboot");
  } else if (minute % 1 == 0) {
    sendRequest(`./reboot?${minute}`);
  } else {
    alert("invalid value!");
  }
});

shutdownBtn.addEventListener("click", (event) => {
  event.preventDefault();
  let minute = window.prompt(
    "After how many minutes will the system shut down?",
    "(empty for shut down now)"
  );
  if (minute == null) {
  } else if (minute === "") {
    sendRequest("./shutdown");
  } else if (minute % 1 == 0) {
    sendRequest(`./shutdown?${minute}`);
  } else {
    alert("invalid value!");
  }
});

cancelBtn.addEventListener("click", (event) => {
  event.preventDefault();
  sendRequest("./cancel");
});

runForm.addEventListener("submit", (event) => {
  event.preventDefault();

  if (document.getElementById("run").value !== "") {
    sendRequest("./runcommand?" + document.getElementById("run").value);
    document.getElementById("run").value = "";

    blockArray[0] = "";
    sessionStorage.setItem("blocks", JSON.stringify(blockArray));

    (async function getProcessData() {
      let rowobject;
      await delay(1800);
      await fetch("./data_files/processinfo.json").then((response) => {
        console.log("first fetch done");
        response.json().then((response) => {
          rowobject = { ...response[0] };

          let n = response[0].allowedcpu.split(" ");
          rowobject.allowedcpu = n[n.length - 1];

          if (rowobject.command == "sh") {
            (async function updateProcess() {
              sendRequest("./updateprocess");
              await delay(1600);
              await fetch("./data_files/updatedinfo.json").then((response) => {
                response.json().then((response) => {
                  console.log("second fetch done");
                  rowobject = { ...response[0] };
                  let n = response[0].allowedcpu.split(" ");
                  rowobject.allowedcpu = n[n.length - 1];
                  tablearray.push(rowobject);
                  rowobject.rid = tablearray.indexOf(rowobject);
                  sessionStorage.setItem("table", JSON.stringify(tablearray));
                  createNewLog(`New process '${rowobject.command}' started`);
                  document.location.reload(1);
                });
              });
            })();
          } else {
            console.log("else done");
            tablearray.push(rowobject);
            rowobject.rid = tablearray.indexOf(rowobject);
            sessionStorage.setItem("table", JSON.stringify(tablearray));
            createNewLog(`New process '${rowobject.command}' started`);
            document.location.reload(1);
          }
        });
      });
    })();
  }
});

killForm.addEventListener("submit", (event) => {
  event.preventDefault();
  terminate();
});

document.getElementById("run").addEventListener("input", (event) => {
  event.preventDefault();
  blockArray[0] = document.getElementById("run").value;
  sessionStorage.setItem("blocks", JSON.stringify(blockArray));
});

document.getElementById("kill").addEventListener("input", (event) => {
  event.preventDefault();
  blockArray[1] = document.getElementById("kill").value;
  sessionStorage.setItem("blocks", JSON.stringify(blockArray));
});

// Table events
let checkFocus = false;

function tableEvent() {
  table.addEventListener("click", (event) => {
    (async function te() {
      event.preventDefault();
      checkFocus = true;
      pidCellEvent();
      await delay(5000);
      checkFocus = false;
      updatetable();
    })();
  });
}

tableEvent();

function limitEvent() {
  document.querySelectorAll(".limit-form").forEach((item) => {
    item.addEventListener("submit", (event) => {
      event.preventDefault();
      itemValue = item.firstElementChild.value;
      selectedrow = item.parentElement.parentElement;
      if (itemValue >= 0 && itemValue <= 100) {
        sendRequest(
          "./limitcpu?" +
            itemValue +
            "&" +
            selectedrow.cells[0].firstChild.nodeValue
        );
        item.firstElementChild.value = "";
      } else {
        alert("Input must be in interval 0-100");
        item.firstElementChild.value = "";
      }
      showAlert();
      updatetable();
    });
  });
}

function slcEvent() {
  document.querySelectorAll(".affinity-form").forEach((item) => {
    item.addEventListener("submit", (event) => {
      event.preventDefault();
      itemValue = item.firstElementChild.value;
      selectedrow = item.parentElement.parentElement;

      let pid = item.parentElement.parentElement.cells[0].innerText;
      sendRequest(`./affinity?${pid}&${itemValue}`);
      item.firstElementChild.value = "";

      showAlert();
      updatetable();
    });
  });
}

function pidCellEvent() {
  document.querySelectorAll(".pid-cell").forEach((item) => {
    item.addEventListener("click", (event) => {
      event.preventDefault();
      document.getElementById("kill").value = item.innerHTML;
    });
  });
}

limitEvent();
slcEvent();
pidCellEvent();

if (refreshTime.value == "") {
  document.getElementById("update-cb").removeAttribute("checked");
}

if (utilSetting.value == "") {
  document.getElementById("log-cb").removeAttribute("checked");
}

// Settings
document.getElementById("update-cb").addEventListener("change", function () {
  if (this.checked && refreshTime.value !== "") {
    inputsArray[0] = refreshTime.value;
    sessionStorage.setItem("settings", JSON.stringify(inputsArray));
    refresfPage();
  } else {
    refreshTime.value = "";
    inputsArray[0] = refreshTime.value;
    sessionStorage.setItem("settings", JSON.stringify(inputsArray));
  }
});

document.getElementById("log-cb").addEventListener("change", function () {
  if (this.checked && utilSetting.value !== "") {
    inputsArray[1] = utilSetting.value;
    sessionStorage.setItem("settings", JSON.stringify(inputsArray));
  } else {
    utilSetting.value = "";
    inputsArray[1] = utilSetting.value;
    sessionStorage.setItem("settings", JSON.stringify(inputsArray));
  }
});

refreshTime.addEventListener("input", (event) => {
  event.preventDefault();
  if (document.getElementById("update-cb").checked) {
    if (refreshTime.value % 1 == 0) {
      inputsArray[0] = refreshTime.value;
      sessionStorage.setItem("settings", JSON.stringify(inputsArray));
    }
  }
});
refreshTime.addEventListener("change", (event) => {
  event.preventDefault();
  if (document.getElementById("update-cb").checked) {
    refresfPage();
    updatetable();
  }
});

utilSetting.addEventListener("input", (event) => {
  event.preventDefault();
  if (utilSetting.value % 1 == 0) {
    inputsArray[1] = utilSetting.value;
    sessionStorage.setItem("settings", JSON.stringify(inputsArray));
  }
});

// Log events
logsInput.addEventListener("input", (event) => {
  event.preventDefault();
  if (utilSetting.value % 1 == 0) {
    inputsArray[2] = logsInput.value;
    sessionStorage.setItem("settings", JSON.stringify(inputsArray));
    updateLogs();
  }
});

function updateLogs() {
  let logs = "";

  if (logArray.length !== 0) {
    if (logArray.length < logsInput.value) {
      for (let i = 1; i <= logArray.length; i++) {
        logs = logArray[logArray.length - i] + "\n" + logs;
      }
    } else {
      for (let i = 1; i <= logsInput.value; i++) {
        logs = logArray[logArray.length - i] + "\n" + logs;
      }
    }
  }

  textarea.value = logs;
}

clear.addEventListener("click", (event) => {
  event.preventDefault();
  textarea.value = "";
  logArray = [];
  historyArray = [];
  sessionStorage.removeItem("logs");
  sessionStorage.removeItem("history");
});

updateLogs();

// check on load
if (document.getElementById("run").value !== "") {
  document.getElementById("run").focus();
} else if (document.getElementById("kill").value !== "") {
  document.getElementById("kill").focus();
}

// refresh
async function updatetable() {
  while (
    sessionStorage.getItem("table") !== null &&
    tablearray.length !== 0 &&
    document.getElementById("update-cb").checked &&
    !checkFocus
  ) {
    tablearray.forEach((row, index) => {
      if (!checkFocus) {
        (async function updateProcess() {
          sendRequest(`./updateprocess?${row.pid}`);

          await fetch("./data_files/updatedinfo.json").then((response) => {
            response.json().then((response) => {
              if (response[1] == "error") {
                const index = tablearray.findIndex((element) => {
                  console.log(response[0]);
                  console.log(element.pid);
                  return element.pid == response[0];
                });
                console.log(index);
                if (index !== -1) {
                  createNewLog(
                    `The process with PID ${response[0]} was terminated by an external event`
                  );
                  tablearray.splice(index, 1);
                  sessionStorage.setItem("table", JSON.stringify(tablearray));
                  //document.location.reload(1);
                }
              } else {
                if (response[0].pid == row.pid) {
                  row = { ...response[0] };
                  let n = response[0].allowedcpu.split(" ");
                  row.allowedcpu = n[n.length - 1];
                  row.rid = index;
                  tablearray[index] = row;
                  sessionStorage.setItem("table", JSON.stringify(tablearray));
                  if (document.getElementById("log-cb").checked) {
                    if (historyArray.length !== 0) {
                      if (
                        row.load > utilSetting.value &&
                        !historyArray.includes(row.pid)
                      ) {
                        createNewLog(
                          `CPU util. of process with PID '${row.pid}' is higher then ${utilSetting.value}`
                        );
                        historyArray.push(row.pid);
                        sessionStorage.setItem(
                          "history",
                          JSON.stringify(historyArray)
                        );
                      }
                    } else if (row.load > utilSetting.value) {
                      createNewLog(
                        `CPU util. of process with PID '${row.pid}' is higher then ${utilSetting.value}`
                      );
                      historyArray.push(row.pid);
                      sessionStorage.setItem(
                        "history",
                        JSON.stringify(historyArray)
                      );
                    }
                  }
                }
              }
            });
          });
          createTableRow(
            row.pid,
            row.command,
            row.load,
            row.currentcpu,
            row.allowedcpu,
            index
          );
        })();
      }
    });
    pidCellEvent();
    await delay(1150 * tablearray.length);
  }
  limitEvent();
  slcEvent();
}
updatetable();

let health = 0;
async function refresfPage() {
  do {
    (function firstPageLoad() {
      refreshload();
      refreshUptime();
      refreshLoadavg();
      updateLogs();
      health += parseInt(refreshTime.value);
      if (health >= 42 && !checkFocus) {
        document.location.reload(1);
      }
    })();
    await delay(refreshTime.value * 1000);
  } while (
    document.getElementById("update-cb").checked &&
    refreshTime.value !== ""
  );
}

if (document.getElementById("update-cb").checked && refreshTime.value !== "") {
  refresfPage();
}
