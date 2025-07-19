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

// Set mode (current or voltage for holding regs, input regs)
app.get("/setmode", async (req, res) => {
    if (!isConnected) return res.status(400).json({ error: "Not connected to any port" });

    const func = 0x6F;

    const type = req.query.type;
    const address = parseInt(req.query.address);
    const mode = parseInt(req.query.mode);

    if (isNaN(address) || address < 0 || address > 0xFFFF) {
        return res.status(400).json({ error: "Invalid or missing register address" });
    }

    if (isNaN(mode) || mode < 0 || mode > 1) {
        return res.status(400).json({ error: "Invalid or missing mode. 0 is voltage mode, 1 is current mode" });
    }

    if (!type || (type != "holding" && type != "input")) {
        return res.status(400).json({ error: "Invalid or missing type. Type must be 'holding' or 'input'." });
    }

    const request = Buffer.alloc(3);
    request.writeUInt16BE(index, 0);
    request.writeUInt8(mode + 1, 2); // add 1

    try {
        const result = await modbus.sendRequest(func, request);

        // Check response
        if (result.length < 3) throw new Error("Invalid response length");
        if (result[2] !== 0x01) throw new Error("Invalid status byte");

        res.json({
            success: true,
            raw: result.toString("hex") 
        });
    } catch (err) {
        console.error("Read error:", err.message);
        res.status(500).json({ error: err.message });
    }
});

// Get mode (current or voltage for holding regs, input regs)
app.get("/getmode", async (req, res) => {
    if (!isConnected) return res.status(400).json({ error: "Not connected to any port" });

    const func = 0x6F;

    const type = req.query.type;
    const address = parseInt(req.query.address);

    if (isNaN(address) || address < 0 || address > 0xFFFF) {
        return res.status(400).json({ error: "Invalid or missing register address" });
    }

    if (!type || (type != "holding" && type != "input")) {
        return res.status(400).json({ error: "Invalid or missing type. Type must be 'holding' or 'input'." });
    }

    let typeInt;
    switch (type) {
        case "holding": {
            typeInt = 3;
            break;
        }
        case "input": {
            typeInt = 4;
            break;
        }
    }

    const request = Buffer.alloc(3);
    request.writeUInt16BE(index, 0);
    request.writeUInt16BE(typeInt, 2);

    try {
        const result = await modbus.sendRequest(func, request);

        // Check response
        if (result.length < 3) throw new Error("Invalid response length");
        
        mode = result[2] - 1;

        res.json({
            success: true,
            mode: mode,
            raw: result.toString("hex") 
        });
    } catch (err) {
        console.error("Read error:", err.message);
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

        // Get status byte to discern success
        const statusByte = sendRes[2]; // index 2 = 3rd byte

        if (statusByte === 1) { // true if 0x01 (success)
            res.json({
                success: true,
                raw: sendRes.toString("hex") 
            });
        } else { // false if 0x00 (failure due to max rules reached)
            res.status(409).json({
                error: "Rule not added - maximum number of rules already reached.",
                statusByte
            });
        }
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

// Delete Rule
app.get("/deleterule", async (req, res) => {
    if (!isConnected) return res.status(400).json({ error: "Not connected to any port" });

    const func = 0x68;

    const index = parseInt(req.query.index);

    if (isNaN(index) || index < 0 || index > 0xFFFF) {
        return res.status(400).json({ error: "Invalid or missing rule index" });
    }

    const request = Buffer.alloc(2);
    request.writeUInt16BE(index, 0);

    try {
        const result = await modbus.sendRequest(func, request);

        // Check response
        if (result.length < 3) throw new Error("Invalid response length");
        if (result[2] !== 0x01) throw new Error("Invalid status byte");

        res.json({
            success: true,
            raw: result.toString("hex") 
        });
    } catch (err) {
        console.error("Read error:", err.message);
        res.status(500).json({ error: err.message });
    }
});

// ------
// Add a virtual register
app.get("/addvr", async (req, res) => { 
    try {
        if (!isConnected) return res.status(400).json({ error: "Not connected to any port" });

        const func = 0x69;
        const type = parseInt(req.query.type);

        if (isNaN(type) || type < 0 || type > 0xFFFF) {
            return res.status(400).json({ error: "Invalid or missing register type" });
        }

        const request = Buffer.alloc(2);
        request.writeUInt8(type, 0);
        request.writeUInt8(0, 1);

        const result = await modbus.sendRequest(func, request);

        if (result.length != 5 || result[2] != 1) {
            throw new Error(`Invalid response.`);
        }

        // Send parsed data
        res.json({
            success: true,
            raw: result.toString("hex") 
        });
    } catch (err) {
        console.error("Read error:", err.message);
        res.status(500).json({ error: err.message });
    }
});

// Read a virtual register
app.get("/readvr", async (req, res) => { 
    try {
        if (!isConnected) return res.status(400).json({ error: "Not connected to any port" });

        const func = 0x6A;
        const type = parseInt(req.query.type);
        const address = parseInt(req.query.address);

        if (![1, 2].includes(type)) {
            return res.status(400).json({ error: "Invalid register type (must be 1 or 2)" });
        }
        if (isNaN(address) || address < 0 || address > 0xFFFF) {
            return res.status(400).json({ error: "Invalid or missing register address" });
        }

        const request = Buffer.alloc(3);
        request.writeUInt8(type, 0);
        request.writeUInt16BE(address, 1);

        const result = await modbus.sendRequest(func, request);

        if ((type == 1 && result.length < 4) || (type == 2 && result.length < 5)) { // type 1 = VIR_COIL, type 2 = VIR_HOLDING
            throw new Error(`Response too short: expected ${(type == 2) ? "5" : "4"}, got ${result.length}`);
        }

        // Decode result
        let value;
        if (type == 1) {
            value = result[3]; // 1 byte
            if (![0,1].includes(value)) {
                throw new Error(`Invalid response value: expected 0 or 1, got ${value}`);
            }
        } else {
            value = (result[3] << 8) | result[4]; // 2 bytes
        }

        // Send parsed data
        res.json({
            success: true,
            value,
            raw: result.toString("hex") 
        });
    } catch (err) {
        console.error("Read error:", err.message);
        res.status(500).json({ error: err.message });
    }
});

// Write a virtual register
app.get("/writevr", async (req, res) => { 
    try {
        if (!isConnected) return res.status(400).json({ error: "Not connected to any port" });

        const func = 0x6B;
        const type = parseInt(req.query.type);
        const address = parseInt(req.query.address);
        const value = parseInt(req.query.value);

        if (![1, 2].includes(type)) {
            return res.status(400).json({ error: "Invalid register type (must be 1 or 2)" });
        }
        if (isNaN(address) || address < 0 || address > 0xFFFF) {
            return res.status(400).json({ error: "Invalid or missing register address" });
        }
        if (isNaN(value) || value < 0 || value > 0xFFFF) {
            return res.status(400).json({ error: "Invalid or missing write value" });
        }

        const request = Buffer.alloc(5);
        request.writeUInt8(type, 0);
        request.writeUInt16BE(address, 1);
        request.writeUInt16BE(value, 3);

        const result = await modbus.sendRequest(func, request);

        if (result.length != 5 || result[2] != 1) {
            throw new Error(`Invalid response.`);
        }

        // Send parsed data
        res.json({
            success: true,
            raw: result.toString("hex") 
        });
    } catch (err) {
        console.error("Read error:", err.message);
        res.status(500).json({ error: err.message });
    }
});

// Get virtual register count
app.get("/countvr", async (req, res) => {
    try {
        if (!isConnected) return res.status(400).json({ error: "Not connected to any port" });

        const func = 0x6C;
        const type = parseInt(req.query.type);

        if (![1, 2].includes(type)) {
            return res.status(400).json({ error: "Invalid register type (must be 1 or 2)" });
        }

        const request = Buffer.alloc(2);
        request.writeUInt8(type, 0);
        request.writeUInt8(0, 1);

        const result = await modbus.sendRequest(func, request);

        const count = (result[3] << 8) | result[4];

        if (result.length != 7 || result[2] != 2) { // if length is wrong or byte count != 2
            throw new Error(`Invalid response.`);
        }

        // Send parsed data
        res.json({
            success: true,
            count,
            raw: result.toString("hex") 
        });
    } catch (err) {
        console.error("Read error:", err.message);
        res.status(500).json({ error: err.message });
    }
});

// Delete a virtual register [BackEnd only at the moment]
app.get("/clearvr", async (req, res) => { 
    try {
        if (!isConnected) return res.status(400).json({ error: "Not connected to any port" });

        const func = 0x6D;
        const confirm = req.query.confirmation; // boolean
        if (confirm !== '1' && confirm !== 'true') return res.status(400).json({ error: "Confirmation is required." }); 

        const request = Buffer.alloc(2);
        request.writeUInt8(1, 0); // confirm byte 1
        request.writeUInt8(1, 1); // confirm byte 2

        const result = await modbus.sendRequest(func, request);

        if (result.length != 5 || result[2] != 1) {
            throw new Error(`Invalid response.`);
        }

        // Send parsed data
        res.json({
            success: true,
            raw: result.toString("hex") 
        });
    } catch (err) {
        console.error("Read error:", err.message);
        res.status(500).json({ error: err.message });
    }
});

// ------
// Set RTC time
app.get("/setrtc", async (req, res) => { 
    try {
        if (!isConnected) return res.status(400).json({ error: "Not connected to any port" });

        const timezone = req.query.tz;
        if (!timezone) {
            return res.status(400).json({ error: "Missing timezone (tz) query parameter" });
        }

        // Use luxon to get the time in the specified timezone
        const { DateTime } = require("luxon");
        const now = DateTime.now().setZone(timezone);

        if (!now.isValid) {
            return res.status(400).json({ error: "Invalid timezone" });
        }

        const func = 0x6E;

        const request = Buffer.from([
            now.second,
            now.minute,
            now.hour,
            now.weekday % 7 + 1, // 1 = monday, 7 = sunday
            now.day,
            now.month,
            now.year % 100 // last two digits
        ]);

        const result = await modbus.sendRequest(func, request);

        if (!result || result.length < 3 || result[2] !== 0x01) {
            throw new Error("Failed to set RTC on device");
        }

        res.json({
            success: true,
            timeSent: now.toFormat("yyyy-LL-dd HH:mm:ss"),
            rawResponse: result.toString("hex")
        });
    } catch (err) {
        console.error("RTC Set error:", err.message);
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

// Listen for serial disconnects
modbus.on("disconnect", () => {
    console.warn("Modbus device disconnected!");
    isConnected = false;
    currentPort = null;
    currentSlave = null;
});