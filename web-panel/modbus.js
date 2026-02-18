const { SerialPort } = require("serialport");
const EventEmitter = require("events");

class ModbusRTU extends EventEmitter {
    constructor() {
        super();
        this.port = null;
        this.slaveId = null;
        this.queue = Promise.resolve();
    }

    async connect(slaveId, path, baudRate, parity) {
        if (this.port?.isOpen) {
            await this.disconnect();
        }

        this.slaveId = slaveId;
        this.port = new SerialPort({
            path,
            baudRate,
            parity,
            autoOpen: false,
        });

        this.port.on("close", () => {
            this.emit("disconnect");
        });

        return new Promise((resolve, reject) => {
            this.port.open(err => err ? reject(err) : resolve());
        });
    }

    async disconnect() {
        if (!this.port) return;

        return new Promise((resolve, reject) => {
            this.port.close(err => {
                if (err) return reject(err);
                this.port = null;
                resolve();
            });
        });
    }

    checkCRC(buffer) {
        if (buffer.length < 3) return false;
        
        const receivedCRC = buffer.readUInt16LE(buffer.length - 2);
        const dataWithoutCRC = buffer.slice(0, buffer.length - 2);
        const calculatedCRC = ModbusRTU.crc16(dataWithoutCRC);
        return receivedCRC === calculatedCRC;
    }

    buildFrame(slaveId, functionCode, payloadBuffer) {
        const frame = Buffer.alloc(2 + payloadBuffer.length); // Slave + flunc + payload buffer
        frame.writeUInt8(slaveId, 0);
        frame.writeUInt8(functionCode, 1);
        payloadBuffer.copy(frame, 2);

        const crcVal = this.constructor.crc16(frame);
        const crcBuf = Buffer.alloc(2);
        crcBuf.writeUInt16LE(crcVal, 0);

        return Buffer.concat([frame, crcBuf]);
    }

    async sendRequest(functionCode, payload) {
        // queue requests
        const result = this.queue.catch(() =>{}).then(() =>
            this._executeRequest(functionCode, payload)
        );

        this.queue = result; // Update the queue tail
        return result;
    }


    async _executeRequest(functionCode, payload) {
        if (!this.port || !this.port.isOpen) {
            throw new Error("Serial port is not open.");
        }

        const frame = this.buildFrame(this.slaveId, functionCode, payload);
        console.log("Transmit:", frame.toString("hex"));

        // Clear old buffer before sending
        this.port.flush();

        return new Promise((resolve, reject) => {
            const timeout = setTimeout(() => {
                this.port.off("data", onData);
                reject(new Error("Response Timeout"));
            }, 1000);

            let chunks = Buffer.alloc(0);

            const onData = (data) => {
                chunks = Buffer.concat([chunks, data]);

                if (chunks.length >= 5) { // Slave + Func + data + 2 bytes CRC

                    // 1. Check for exception (MSB of function code set)
                    if ((chunks[1] & 0x80) === 0x80) {
                        cleanup();
                        const exceptionCode = chunks[2];
                        // Verify CRC of exception frame too
                        if (!this.checkCRC(chunks)) {
                            reject(new Error("Received Exception with invalid CRC"));
                            return;
                        }
                        const funcName = this.getFunctionName(chunks[1] & 0x7F);
                        const errMsg = `Modbus Exception 0x${exceptionCode.toString(16)} on ${funcName}`;
                        reject(new Error(errMsg));
                        return;
                    }
                    
                    // 2. Verify CRC
                    if (this.checkCRC(chunks)) {
                        cleanup();
                        console.log("Received Valid:", chunks.toString("hex"));
                        resolve(chunks);
                    }
                    // Ignore if invalid CRC
                }
            };

            const cleanup = () => {
                clearTimeout(timeout);
                this.port.off("data", onData);
            }

            this.port.on("data", onData);

            this.port.write(frame, (err) => {
                if (err) {
                    cleanup();
                    reject(err);
                }
            });
        });
    }

    getFunctionName(code) {
        const map = {
            0x01: "Read Coils",
            0x02: "Read Discrete Inputs",
            0x03: "Read Holding Registers",
            0x04: "Read Input Registers",
            0x05: "Write Single Coil",
            0x06: "Write Single Register",
            0x0F: "Write Multiple Coils",
            0x10: "Write Multiple Registers",
        };
        return map[code] || `Function 0x${code.toString(16)}`;
    }

    decodeExceptionCode(code) {
        const map = {
            0x01: "Illegal Function",
            0x02: "Illegal Data Address",
            0x03: "Illegal Data Value",
            0x04: "Slave Device Failure",
            0x05: "Acknowledge",
            0x06: "Slave Device Busy",
            0x08: "Memory Parity Error",
            0x0A: "Gateway Path Unavailable",
            0x0B: "Gateway Target Device Failed to Respond",
        };
        return map[code] || "Unknown Error";
    }

    buildReadPayload(addr, count) {
        const buf = Buffer.alloc(4);
        buf.writeUInt16BE(addr, 0);
        buf.writeUInt16BE(count, 2);
        return buf;
    }

    decodeCoilsResponse(buffer, count) {
        // data starts at byte 3, length is buffer[2] bytes
        // bits are packed in bytes
        const byteCount = buffer[2];
        const data = buffer.slice(3, 3 + byteCount);
        const coilValues = [];
        for (let i = 0; i < count; i++) {
            const byteIndex = Math.floor(i/8);
            const bitIndex = i % 8;
            coilValues.push((data[byteIndex] & (1 << bitIndex)) !== 0);
        }
        return coilValues;
    }

    decodeRegistersResponse(buffer, count) {
        // same, data starts at byte 3, length is buffer[2] bytes
        // each register is 2 bytes (16 bit), big endian
        const byteCount = buffer[2];
        const data = buffer.slice(3, 3 + byteCount);
        const registerValues = [];
        for (let i = 0; i < count; i++) {
            registerValues.push(data.readUInt16BE(i*2));
        }
        return registerValues;
    }

    buildWriteCoilsPayload(addr, values) { 
        const count = values.length;
        const byteCount = Math.ceil(count / 8);
        const buf = Buffer.alloc(5 + byteCount);
        buf.writeUInt16BE(addr, 0);
        buf.writeUInt16BE(count, 2);
        buf.writeUInt8(byteCount, 4);

        for (let i = 0; i < count; i++) {
            if (values[i]) {
                buf[5 + Math.floor(i / 8)] |= 1 << (i % 8);
            }
        }

        return buf;
    }

    buildWriteRegistersPayload(addr, values) {
        const count = values.length;
        const byteCount = count * 2;
        const buf = Buffer.alloc(5 + byteCount);
        buf.writeUInt16BE(addr, 0);
        buf.writeUInt16BE(count, 2);
        buf.writeUInt8(byteCount, 4);

        values.forEach((val, i) => {
            buf.writeUInt16BE(val, 5 + i * 2);
        });

        return buf;
    }

    // Read Coils (0x01)
    async readCoils(addr, count) {
        const payload = this.buildReadPayload(addr, count);
        const response = await this.sendRequest(0x01, payload);
        return this.decodeCoilsResponse(response, count);
    }

    // Read Discrete Inputs (0x02)
    async readDiscreteInputs(addr, count) {
        const payload = this.buildReadPayload(addr, count);
        const response = await this.sendRequest(0x02, payload);
        return this.decodeCoilsResponse(response, count);
    }

    // Read Holding Registers (0x03)
    async readHoldingRegisters(addr, count) {
        const payload = this.buildReadPayload(addr, count);
        const response = await this.sendRequest(0x03, payload);
        return this.decodeRegistersResponse(response, count);
    }

    // Read Input Registers (0x04)
    async readInputRegisters(addr, count) {
        const payload = this.buildReadPayload(addr, count);
        const response = await this.sendRequest(0x04, payload);
        return this.decodeRegistersResponse(response, count);
    }

    // Write Single Coil (0x05)
    // TODO

    // Write Multiple Coils (0x0F)
    async writeCoils(addr, values) {
        const payload = this.buildWriteCoilsPayload(addr, values);
        return await this.sendRequest(0x0F, payload);
    }

    // Write Single Register (0x06)
    // TODO

    // Write Multiple Registers (0x10)
    async writeRegisters(addr, values) {
        const payload = this.buildWriteRegistersPayload(addr, values);
        return await this.sendRequest(0x10, payload);
    }

    // CRC16
    // Source: https://stackoverflow.com/questions/19347685/calculating-modbus-rtu-crc-16
    // Same one im using on the STM32 for CRC16 calculation, just converted to js
    static crc16(buffer) {
        let crc = 0xFFFF;

        for (let i = 0; i < buffer.length; i++) {
            crc ^= buffer[i];

            for (let b = 0; b < 8; b++) {
                if ((crc & 0x0001) !== 0) {
                    crc >>= 1;
                    crc ^= 0xA001;
                } else {
                    crc >>= 1;
                }
            }
        }

        return crc;
    }

    static async listPorts() {
        const ports = await SerialPort.list();
        return ports.map(port => port.path);
    }
}

module.exports = ModbusRTU;