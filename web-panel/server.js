const express = require("express");
const cors = require("cors");
const ModbusRTU = require("./modbus"); // custom

const app = express();
app.use(cors());
app.use(express.json());
app.use(express.static('public'));

const modbus = new ModbusRTU();

const FUNC_CODES = {
    SET_MODE_HOLDING: 0x6F,
    SET_MODE_INPUT: 0x70,
    GET_MODE: 0x71,
    ADD_RULE: 0x65,
    GET_RULE_COUNT: 0x66,
    GET_RULE: 0x67,
    DELETE_RULE: 0x68,
    ADD_VR: 0x69,
    READ_VR: 0x6A,
    WRITE_VR: 0x6B,
    COUNT_VR: 0x6C,
    CLEAR_VR: 0x6D,
    SET_RTC: 0x6E,
    SET_ESTOP: 0x72,
    FACTORY_RESET: 0x73
};

let connectionState = {
    isConnected: false,
    port: null,
    slave: null
};

// ----- MIDDLEWARE -----

// Connection Guard - to block requests if not connected
const requireConnection = (req, res, next) => {
    if (!connectionState.isConnected) {
        return res.status(400).json({ error: "Not connected to any port" });
    }
    next();
};

// Validation
const validate = (schema) => (req, res, next) => {
    try {
        const source = req.method === 'GET' ? req.query : req.body;
        const cleanData = {};

        for (const [field, rules] of Object.entries(schema)) {
            let value = source[field];

            // Check required
            if (value === undefined || value === null || value === '') {
                if (rules.required) throw new Error(`Field '${field}' is required.`);
                continue;
            }

            // Check number
            if (rules.type === 'number') {
                const parsed = parseInt(value);
                if (isNaN(parsed)) throw new Error(`Field '${field}' must be a number.`);
                if (rules.min !== undefined && parsed < rules.min) throw new Error(`Field '${field}' too small (min ${rules.min}).`);
                if (rules.max !== undefined && parsed > rules.max) throw new Error(`Field '${field}' too large (max ${rules.max}).`);
                value = parsed;
            }

            // Check bool
            if (rules.type === 'boolean') {
                value = (value === 'true' || value === '1' || value === true || value === 1);
            }

            // Check enum
            if (rules.enum && !rules.enum.includes(value)) {
                throw new Error(`Field '${field}' must be one of: ${rules.enum.join(', ')}`);
            }

            cleanData[field] = value;
        }

        req.cleanData = cleanData;
        next();
    } catch (err) {
        return res.status(400).json({ error: err.message });
    }
};

// Timeout function
async function withTimeout(promiseOp) {
    let lastError;

    for (let attempt = 1; attempt <= CONFIG.retries; attempt++) {
        try {
            // Create a timeout promise
            const timeout = new Promise((_, reject) =>
                setTimeout(() => reject(new Error('Timeout')), CONFIG.timeoutMs)
            );

            // Race the actual operation against the timeout
            return await Promise.race([promiseOp, timeout]);
        } catch (err) {
            lastError = err;
            if (err.message !== 'Timeout') throw err; // Only retry timeout errors
            console.warn(`Attempt ${attempt} timed out...`);
        }
    }
    throw lastError || new Error('Operation timed out after retries');
}

// ----- ROUTES -----

// List serial ports (does not require a connection)
app.get("/ports", async (req, res) => {
    try {
        const portNames = await ModbusRTU.listPorts();
        res.json(portNames);
    } catch (err) {
        res.status(500).json({ error: "Failed to list ports" });
    }
});

// Connect to a selected port
const connectSchema = {
    port: { required: true },
    baud: { type: 'number', min: 300, max: 115200, required: true },
    slave: { type: 'number', min: 1, max: 247, required: true }
};
app.post("/connect", validate(connectSchema), async (req, res) => {
    try {
        const { port, baud, slave } = req.cleanData;
        await modbus.connect(slave, port, baud, "none");
        
        connectionState = { isConnected: true, port, slave };
        console.log(`Connected to ${port}`);
        res.json({ success: true });
    } catch (err) {
        connectionState = { isConnected: false, port: null, slave: null };
        res.status(500).json({ success: false, error: err.message });
    }
});

// Get connection status
app.get("/status", async (req, res) => {
    res.json({ status: connectionState.isConnected, ...connectionState });
});

// Disconnect
app.post("/disconnect", async (req, res) => {
    try {
        await modbus.disconnect();
        connectionState.isConnected = false;
        res.json({ success: true });
    } catch (err) {
        res.json({ success: false, error: err.message });
    }
});

// PROTECTED ROUTES (Require Connection)

// Read (single or multiple)
const readSchema = {
    type: { enum: ['coil', 'discrete', 'holding', 'input'], required: true },
    address: { type: 'number', min: 0, max: 65535, required: true },
    quantity: { type: 'number', min: 1, max: 125, required: false } // optional
};
app.get("/read", requireConnection, validate(readSchema), async (req, res) => {
    try {
        const { type, address, quantity } = req.cleanData;
        const count = quantity || 1;
        let op;
        
        if (type === 'coil') op = modbus.readCoils(address, count);
        else if (type === 'discrete') op = modbus.readDiscreteInputs(address, count);
        else if (type === 'holding') op = modbus.readHoldingRegisters(address, count);
        else if (type === 'input') op = modbus.readInputRegisters(address, count);

        const result = await op;
        res.json({ data: result || [] });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

// Write (single or multiple)
const writeSchema = {
    type: { enum: ['coil', 'holding'], required: true },
    address: { type: 'number', min: 0, max: 65535, required: true },
    value: { required: true } // Validated manually below for CSV support
};
app.post("/write", requireConnection, validate(writeSchema), async (req, res) => {
    try {
        const { type, address, value } = req.cleanData;
        
        // Handle CSV values
        const valString = String(value);
        const vals = valString.split(',').map(v => {
            const parsed = parseInt(v);
            if (isNaN(parsed)) throw new Error("Invalid value in CSV");
            return parsed;
        });

        let op;
        if (type === 'coil') op = modbus.writeCoils(address, vals);
        else if (type === 'holding') op = modbus.writeRegisters(address, vals);

        await op;
        res.json({ status: "written", address, value: vals });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

// Set mode (current or voltage for holding regs, input regs)
const setModeSchema = {
    type: { enum: ['holding', 'input'], required: true },
    address: { type: 'number', min: 0, max: 65535, required: true },
    mode: { type: 'number', min: 0, max: 1, required: true }
};
app.post("/setmode", requireConnection, validate(setModeSchema), async (req, res) => {
    try {
        const { type, address, mode } = req.cleanData;
        const func = (type === 'holding') ? FUNC_CODES.SET_MODE_HOLDING : FUNC_CODES.SET_MODE_INPUT;
        
        const request = Buffer.alloc(3);
        request.writeUInt16BE(address, 0);
        request.writeUInt8(mode + 1, 2); // 0->1, 1->2

        const result = await modbus.sendRequest(func, request);
        if (result.length < 3 || result[2] !== 0x01) throw new Error("Device reported failure");
        res.json({ success: true, raw: result.toString("hex") });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

// Get mode (current or voltage for holding regs, input regs)
app.get("/getmode", requireConnection, validate({
    type: { enum: ['holding', 'input'], required: true },
    address: { type: 'number', min: 0, max: 65535, required: true }
}), async (req, res) => {
    try {
        const { type, address } = req.cleanData;
        const typeInt = (type === 'holding') ? 3 : 4;
        const request = Buffer.alloc(3);
        request.writeUInt16BE(address, 0);
        request.writeUInt8(typeInt, 2);

        const result = await modbus.sendRequest(FUNC_CODES.GET_MODE, request);
        const mode = result[2] - 1; 
        res.json({ success: true, mode, raw: result.toString("hex") });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

// Add a rule
const addRuleSchema = {
    input_type1: { type: 'number', min: 1, max: 6, required: true },
    input_reg1: { type: 'number', min: 0, max: 65535, required: true },
    op1: { type: 'number', min: 1, max: 6, required: true },
    compare_value1: { type: 'number', min: 0, max: 65535, required: true },
    join: { type: 'number', min: 1, max: 3, required: true },
    output_type: { type: 'number', min: 1, max: 255, required: true },
    output_reg: { type: 'number', min: 0, max: 65535, required: true },
    output_value: { type: 'number', min: 0, max: 65535, required: true },
    // Optional second rule parts
    input_type2: { type: 'number', min: 1, max: 6, required: false },
    input_reg2: { type: 'number', min: 0, max: 65535, required: false },
    op2: { type: 'number', min: 1, max: 6, required: false },
    compare_value2: { type: 'number', min: 0, max: 65535, required: false }
};

app.post("/addrule", requireConnection, validate(addRuleSchema), async (req, res) => {
    try {
        const d = req.cleanData; // d for data (short alias)
        
        const request = Buffer.alloc(18);
        request.writeUInt8(d.input_type1, 0);
        request.writeUInt16BE(d.input_reg1, 1);
        request.writeUInt8(d.op1, 3);
        request.writeUInt16BE(d.compare_value1, 4);

        // Handle optional second part (default to 0 if missing)
        request.writeUInt8(d.input_type2 || 0, 6);
        request.writeUInt16BE(d.input_reg2 || 0, 7);
        request.writeUInt8(d.op2 || 0, 9);
        request.writeUInt16BE(d.compare_value2 || 0, 10);

        request.writeUInt8(d.join, 12);
        request.writeUInt8(d.output_type, 13);
        request.writeUInt16BE(d.output_reg, 14);
        request.writeUInt16BE(d.output_value, 16);

        const result = await modbus.sendRequest(FUNC_CODES.ADD_RULE, request);

        if (result[2] === 0x01) res.json({ success: true });
        else res.status(409).json({ error: "Max rules reached" });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});


// Get rule count
app.get("/getrulecount", requireConnection, async (req, res) => {
    try {
        const result = await modbus.sendRequest(FUNC_CODES.GET_RULE_COUNT, Buffer.alloc(2));
        const ruleCount = (result[3] << 8) | result[4];
        res.json({ success: true, data: ruleCount });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

// Get Rule
app.get("/getrule", requireConnection, validate({
    index: { type: 'number', min: 0, max: 65535, required: true }
}), async (req, res) => {
    try {
        const request = Buffer.alloc(2);
        request.writeUInt16BE(req.cleanData.index, 0);
        const result = await modbus.sendRequest(FUNC_CODES.GET_RULE, request);
        
        if (result.length < 20) throw new Error("Response too short");

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
app.delete("/deleterule", requireConnection, validate({
    index: { type: 'number', min: 0, max: 65535, required: true }
}), async (req, res) => {
    try {
        const request = Buffer.alloc(2);
        request.writeUInt16BE(req.cleanData.index, 0);
        const result = await modbus.sendRequest(FUNC_CODES.DELETE_RULE, request);
        if (result[2] !== 0x01) throw new Error("Delete failed");
        res.json({ success: true });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

// ------
// Add a virtual register
app.post("/addvr", requireConnection, validate({
    type: { type: 'number', min: 0, max: 65535, required: true }
}), async (req, res) => {
    try {
        const request = Buffer.alloc(2);
        request.writeUInt8(req.cleanData.type, 0);
        request.writeUInt8(0, 1);
        const result = await modbus.sendRequest(FUNC_CODES.ADD_VR, request);
        if (result[2] !== 1) throw new Error("Failed");
        res.json({ success: true });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

// Read a virtual register
app.get("/readvr", requireConnection, validate({
    type: { type: 'number', min: 1, max: 2, required: true },
    address: { type: 'number', min: 0, max: 65535, required: true }
}), async (req, res) => {
    try {
        const { type, address } = req.cleanData;
        const request = Buffer.alloc(3);
        request.writeUInt8(type, 0);
        request.writeUInt16BE(address, 1);
        const result = await modbus.sendRequest(FUNC_CODES.READ_VR, request);
        
        let value;
        if (type == 1) value = result[3];
        else value = (result[3] << 8) | result[4];
        
        res.json({ success: true, value });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

// Write a virtual register
app.post("/writevr", requireConnection, validate({
    type: { type: 'number', min: 1, max: 2, required: true },
    address: { type: 'number', min: 0, max: 65535, required: true },
    value: { type: 'number', min: 0, max: 65535, required: true }
}), async (req, res) => {
    try {
        const { type, address, value } = req.cleanData;
        const request = Buffer.alloc(5);
        request.writeUInt8(type, 0);
        request.writeUInt16BE(address, 1);
        request.writeUInt16BE(value, 3);
        
        const result = await modbus.sendRequest(FUNC_CODES.WRITE_VR, request);
        if (result[2] !== 1) throw new Error("Failed");
        res.json({ success: true });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

// Get virtual register count
app.get("/countvr", requireConnection, validate({
    type: { type: 'number', min: 1, max: 2, required: true }
}), async (req, res) => {
    try {
        const { type } = req.cleanData;
        const request = Buffer.alloc(2);
        request.writeUInt8(type, 0);
        request.writeUInt8(0, 1);

        const result = await modbus.sendRequest(FUNC_CODES.COUNT_VR, request);
        
        if (result.length < 5 || result[2] !== 0x02) { 
             throw new Error("Invalid response from device");
        }

        const count = (result[3] << 8) | result[4];
        res.json({ success: true, count, raw: result.toString("hex") });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

// Delete a virtual register [BackEnd only at the moment]
app.post("/clearvr", requireConnection, validate({
    confirmation: { type: 'boolean', required: true }
}), async (req, res) => {
    try {
        if (!req.cleanData.confirmation) throw new Error("Not confirmed");
        const result = await modbus.sendRequest(FUNC_CODES.CLEAR_VR, Buffer.from([1, 1]));
        if (result[2] !== 1) throw new Error("Failed");
        res.json({ success: true });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

// ------
// Set RTC time
app.post("/setrtc", requireConnection, validate({
    tz: { required: true }
}), async (req, res) => {
    try {
        const { DateTime } = require("luxon");
        const now = DateTime.now().setZone(req.cleanData.tz);
        
        if (!now.isValid) throw new Error("Invalid timezone");

        const request = Buffer.from([
            now.second, now.minute, now.hour,
            now.weekday % 7 + 1, now.day, now.month, now.year % 100
        ]);

        const result = await modbus.sendRequest(FUNC_CODES.SET_RTC, request);
        if (result[2] !== 0x01) throw new Error("Failed");
        res.json({ success: true, timeSent: now.toFormat("yyyy-LL-dd HH:mm:ss") });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});


// Set emergency stop configuration
app.post("/setemergencystop", requireConnection, validate({
    channel: { type: 'number', min: 0, max: 65535, required: true },
    inputMode: { type: 'number', min: 0, max: 1, required: true } // 0 or 1
}), async (req, res) => {
    try {
        const { channel, inputMode } = req.cleanData;
        
        const request = Buffer.alloc(3);
        request.writeUInt16BE(channel, 0);
        request.writeUInt8(inputMode + 1, 2); // Convert 0->1 (NO), 1->2 (NC)

        const result = await modbus.sendRequest(FUNC_CODES.SET_ESTOP, request);

        // Validation: Expect Status byte 0x01
        if (result.length < 3 || result[2] !== 0x01) {
            throw new Error("Device reported failure");
        }

        res.json({ success: true, raw: result.toString("hex") });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});



// Factory Reset
app.post("/factoryreset", requireConnection, validate({
    confirmation: { type: 'boolean', required: true }
}), async (req, res) => {
    try {
        if (!req.cleanData.confirmation) throw new Error("Not confirmed");
        const result = await modbus.sendRequest(FUNC_CODES.FACTORY_RESET, Buffer.from([1, 0]));
        if (result[2] !== 0x01) throw new Error("Failed");
        res.json({ success: true });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

// ----- SHUTDOWN -----

app.listen(5000, () => {
    console.log("Server running on http://localhost:5000");
});

// Handle application shutdown cleanly
process.on("SIGINT", async () => {
    if (connectionState.isConnected) {
        await modbus.disconnect();
    }
    process.exit();
});

// Listen for serial disconnects
modbus.on("disconnect", () => {
    console.warn("Modbus device disconnected!");
    connectionState = { isConnected: false, port: null, slave: null };
});