const express = require("express");
const cors = require("cors");
const ModbusRTU = require("./modbus"); // custom

const app = express();
app.use(cors());
app.use(express.json());
app.use(express.static('public'));

const modbus = new ModbusRTU();
const timeoutMs = 1000; // timeout for all functions sent over modbus in milliseconds
const timeoutRetry = 3; // number of times to try when querying over modbus


let isConnected = false;
let currentPort;
let currentSlave;

// Timeout function
async function withTimeout(awaiting, ms) {
    for (let attempt = 1; attempt <= timeoutRetry; attempt++) {
        try {
            const timeout = new Promise((_, reject) =>
                setTimeout(() => reject(new Error('Timeout')), ms)
            );

            const result = await Promise.race([awaiting, timeout]);
            return result;
        } catch (err) {
            if (attempt === timeoutRetry || err.message !== 'Timeout') {
                throw err;
            }
            console.warn(`Attempt ${attempt} timed out, retrying...`);
        }
    }

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
                resultAwait = modbus.readCoils(addr, count);
                break;
            case "discrete":
                resultAwait = modbus.readDiscreteInputs(addr, count);
                break;
            case "holding":
                resultAwait = modbus.readHoldingRegisters(addr, count);
                break;
            case "input":
                resultAwait = modbus.readInputRegisters(addr, count);
                break;
            default:
                return res.status(400).json({ error: "Invalid register type" });
        }

        // timeout
        const result = await withTimeout(resultAwait, timeoutMs);

        res.json({ data: result || [] });
    } catch (err) {
        console.error("Read error:", err.message);
        res.status(500).json({ error: err.message });
    }
});

// Write (single and multiple)
app.get("/write", async (req, res) => {
    if (!isConnected) return res.status(400).json({ error: "Not connected to any port" });

    const { type, address, value } = req.query;

    try {
        const vals = value.split(',').map(v => parseInt(v)); // separate value(s) by comma, and convert to integers

        const addr = parseInt(address);

        let resultAwait;
        switch (type) {
            case "coil":
                resultAwait = modbus.writeCoils(addr, vals);
                break;
            case "holding":
                resultAwait = modbus.writeRegisters(addr, vals);
                break;
            default:
                return res.status(400).json({ error: "Invalid register type" });
        }

        // timeout
        const result = await withTimeout(resultAwait, timeoutMs);

        res.json({ status: "written", address: addr, value: vals });
    } catch (err) {
        console.error("Write error:", err.message);
        res.status(500).json({ error: err.message });
    }
});

// -----

async function sendRule(slave, rule) {
    // Custom function
    const func = 0x65; // Function code 101

    const b = Buffer.alloc(18); // 18 bytes for the rule data (no index)

    b.writeUInt8(rule.input_type1, 0);
    b.writeUInt16BE(rule.input_reg1, 1);
    b.writeUInt8(rule.op1, 3);
    b.writeUInt16BE(rule.compare_value1, 4);

    b.writeUInt8(rule.input_type2, 6);
    b.writeUInt16BE(rule.input_reg2, 7);
    b.writeUInt8(rule.op2, 9);
    b.writeUInt16BE(rule.compare_value2, 10);

    b.writeUInt8(rule.join, 12);
    b.writeUInt8(rule.output_type, 13);
    b.writeUInt16BE(rule.output_reg, 14);
    b.writeUInt16BE(rule.output_value, 16);

    try {
        const response = await modbus.sendRequest(func, b);
        return response;
    } catch (err) {
        console.error("Failed to send rule", err);
        throw err;
    }
}

// Add a rule
app.get("/addrule", async (req, res) => {
    try {
        if (!isConnected) return res.status(400).json({ error: "Not connected to any port" });

        const rule = {
            input_type1: parseInt(req.query.input_type1),
            input_reg1: parseInt(req.query.input_reg1),
            op1: parseInt(req.query.op1),
            compare_value1: parseInt(req.query.compare_value1),
            
            input_type2: parseInt(req.query.input_type2),
            input_reg2: parseInt(req.query.input_reg2),
            op2: parseInt(req.query.op2),
            compare_value2: parseInt(req.query.compare_value2),

            join: parseInt(req.query.join),
            output_type: parseInt(req.query.output_type),
            output_reg: parseInt(req.query.output_reg),
            output_value: parseInt(req.query.output_value),
        };

        const sendRes = await sendRule(currentSlave, rule);
        res.json({ success: true, raw: sendRes.toString("hex") });

    } catch (err) {
        console.error("Failed to add rule:", err);
        res.status(500).json({ error: err.message });
    }
});


// Get rule count
app.get("/getrulecount", async (req, res) => {
    if (!isConnected) return res.status(400).json({ error: "Not connected to any port" });

    const func = 0x66;

    const request = Buffer.alloc(2); // empty, but 2 bytes as the minimum modbus request length on the controller is 6 bytes

    try {
        const result = await modbus.sendRequest(func, request);

        // Decode response
        if (result.length < 5) throw new Error("Invalid response length");
        if (result[2] !== 0x02) throw new Error("Unexpected byte count");

        const ruleCount = (result[3] << 8) | result[4];

        res.json({ data: ruleCount });
    } catch (err) {
        console.error("Read error:", err.message);
        res.status(500).json({ error: err.message });
    }
});

// Get Rule
app.get("/getrule", async (req, res) => {
    if (!isConnected) return res.status(400).json({ error: "Not connected to any port" });

    const func = 0x67;
    const index = parseInt(req.query.index);

    if (isNaN(index) || index < 0 || index > 0xFFFF) {
        return res.status(400).json({ error: "Invalid or missing rule index" });
    }

    const request = Buffer.alloc(2);
    request.writeUInt16BE(index, 0);

    try {
        const result = await modbus.sendRequest(func, request);

        if (result.length < 20) {
            throw new Error(`Response too short: expected 20+, got ${result.length}`);
        }

        // Decode result
        const input_type1Raw = result[2];
        const input_reg1 = (result[3] << 8) | result[4];
        const op1Raw = result[5];
        const compare_value1 = (result[6] << 8) | result[7];
        const input_type2Raw = result[8];
        const input_reg2 = (result[9] << 8) | result[10];
        const op2Raw = result[11];
        const compare_value2 = (result[12] << 8) | result[13];
        const joinRaw = result[14];
        const output_typeRaw = result[15];
        const output_reg = (result[16] << 8) | result[17];
        const output_value = (result[18] << 8) | result[19];

        // Send parsed data
        res.json({
            data: {
                input_type1Raw,
                input_reg1,
                op1Raw,
                compare_value1,
                input_type2Raw,
                input_reg2,
                op2Raw,
                compare_value2,
                joinRaw,
                output_typeRaw,
                output_reg,
                output_value
            }
        });
    } catch (err) {
        console.error("Read error:", err.message);
        res.status(500).json({ error: err.message });
    }
});


// ------
// List serial ports
app.get("/ports", async (req, res) => {
    try {
        const portNames = await ModbusRTU.listPorts();
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

// Connect to a selected port UPDATED
app.get("/connect", async (req, res) => {
    const { port, baud, slave } = req.query;

    try {
        const baudRate = parseInt(baud);

        if (!port) return res.status(400).json({ success: false, error: "No port specified" });
        if (!baudRate) return res.status(400).json({ success: false, error: "No baud rate specified" });
        if (!slave) return res.status(400).json({ success: false, error: "No slave address specified" });

        await modbus.connect(slave, port, baudRate, "none");
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

// Disconnect UPDATED
app.get("/disconnect", async (req, res) => {
    try {
        await modbus.disconnect();
        console.log("Disconnected.");
        isConnected = false;
        currentPort = null;
        currentSlave = null;
        res.json({ success: true });
    } catch (err) {
        console.log("Disconnection error:", err);
        res.json({ success: false, error: err.message });
    }
});


app.listen(5000, () => {
    console.log("Server running on http://localhost:5000");
});

// Handle application shutdown cleanly
process.on("SIGINT", async () => {
    if (isConnected) {
        await modbus.disconnect();
    }
    process.exit();
});