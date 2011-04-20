var serialport = require('serialport');
var events = require('events');

function Buzz(device) {
    var options = {
        baudrate: 115200,
        databits: 8,
        parity: 0,
        stopbits: 1,
        parser: serialport.parsers.readline("\n\r"),
    }

    var that  = this;

    this.port = port = new serialport.SerialPort(device, options);

    port.on("data", function(data) {
        that.emit("button", data[1] - '0'.charCodeAt(0));
    });
}

Buzz.prototype = new events.EventEmitter();

Buzz.prototype.set_led = function (led, brightness) {
    buf = new Buffer("L$$\n\r");

    buf[1] = '0'.charCodeAt(0) + led;
    buf[2] = brightness ? 0xff : 0x00;

    // async?
    this.port.write(buf);
};

Buzz.prototype.close = function () {
    this.port.close();
}

exports.Buzz = Buzz

