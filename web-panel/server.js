const express = require("express");
const cors = require("cors");
const ModbusRTU = require("modbus-serial");
const { SerialPort } = require("serialport");

const app = express();
app.use(cors());
app.use(express.json());
app.use(express.static('public'));

const client = new ModbusRTU;
const timeoutMs = 3000; // timeout for all functions sent over modbus in milliseconds


let isConnected = false;
let currentPort;
let currentSlave;

// Timeout function
function withTimeout(awaiting, ms) {
    const timeout = new Promise((_, reject) =>
        setTimeout(() => reject(new Error('Timeout')), ms)
    );
    return Promise.race([awaiting, timeout]);
}

// Read (single or multiple)
app.get("/read", async (req, res) => {
    if (!isConnected) return res.status(400).json({ error: "Not connected to any port" });

    const { type, address, quantity } = req.query;

    try {
        const addr = parseInt(address);
        const count = parseInt(quantity || 1); // default to 1

        let resultAwait;
        switch (type) {
            case "coil":
                resultAwait = client.readCoils(addr, count);
                break;
            case "discrete":
                resultAwait = client.readDiscreteInputs(addr, count);
                break;
            case "holding":
                resultAwait = client.readHoldingRegisters(addr, count);
                break;
            case "input":
                resultAwait = client.readInputRegisters(addr, count);
                break;
            default:
                return res.status(400).json({ error: "Invalid register type" });
        }

        // timeout
        const result = await withTimeout(resultAwait, timeoutMs);

        res.json({ data: result.data || result.dataValues || [] });
    } catch (err) {
        console.error("Read error:", err.message);
        res.status(500).json({ error: err.message });
    }
});

// Write (single)
app.get("/write", async (req, res) => {
    if (!isConnected) return res.status(400).json({ error: "Not connected to any port" });

    const { type, address, value } = req.query;

    try {
        const addr = parseInt(address);
        const val = parseInt(value);

        let resultAwait;
        switch (type) {
            case "coil":
                resultAwait = client.writeCoil(addr, val);
                break;
            case "holding":
                resultAwait = client.writeRegister(addr, val);
                break;
            default:
                return res.status(400).json({ error: "Invalid register type" });
        }

        // timeout
        const result = await withTimeout(resultAwait, timeoutMs);

        res.json({ status: "written", address: addr, value: val });
    } catch (err) {
        console.error("Write error:", err.message);
        res.status(500).json({ error: err.message });
    }
});



// ------
// List serial ports
app.get("/ports", async (req, res) => {
    try {
        const ports = await SerialPort.list();
        const portNames = ports.map(port => port.path);
        res.json(portNames);
    } catch (err) {
        console.error("Error listing ports:", err);
        res.status(500).json({ error: "Failed to list ports" });
    }
});

// Get connection status
app.get("/status", async (req, res) => {
    try {
        res.json({ status: isConnected, port: currentPort , slave: currentSlave });
    } catch (err) {
        console.error("Response error:", err);
    }
});

// Connect to a selected port
app.get("/connect", async (req, res) => {
    const { port, baud, slave } = req.query;

    try {
        const baudRate = parseInt(baud);

        if (!port) return res.status(400).json({ success: false, error: "No port specified" });
        if (!baudRate) return res.status(400).json({ success: false, error: "No baud rate specified" });
        if (!slave) return res.status(400).json({ success: false, error: "No slave address specified" });

        await client.connectRTUBuffered(port, { baudRate: baudRate, parity: "none" });
        client.setID(slave); // slave address
        isConnected = true;
        currentPort = port;
        currentSlave = slave;
        console.log(`Connected to ${port}`);
        res.json({ success: true });
    } catch (err) {
        console.error("Connection error:", err);
        res.status(500).json({ success: false, error: err.message });
    }
});

// Disconnect
app.get("/disconnect", async (req, res) => {
    try {
        client.close(() => {
            console.log("Disconnected.");
            isConnected = false;
            currentPort = null;
            currentSlave = null;
            res.json({ success: true });
        });
    } catch (err) {
        console.log("Disconnection error:", err);
        res.json({ success: false, error: err.message });
    }
});



app.listen(5000, () => {
    console.log("Server running on http://localhost:5000");
});