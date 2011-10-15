var serialport = require('serialport');
var events = require('events');

// create a buzzer interface
// - device: path to the serial device (e.g. /dev/ttyUSB0)
function Buzz(device) {
    var options = {
        baudrate: 115200,
        databits: 8,
        parity: 0,
        stopbits: 1,
        parser: serialport.parsers.readline("\n"),
    }

    var that  = this;

    this.port = port = new serialport.SerialPort(device, options);

    port.on("data", function(data) {
        if(data[0] == "K") {
            that.emit("button", data[1].charCodeAt(0) - 'A'.charCodeAt(0));
        }
    });

    // TODO: quickfixing
    var i;
    for(i = 0; i < 4; ++i) {
        that.set_led(i, 0);
    }
}

// implements the following events:
// - button(player)
Buzz.prototype = new events.EventEmitter();

// set leds on or off
// - player: [0-3]
// - led: the led of the player [0-1]
// - brightness the brightness of the led [0-1]
Buzz.prototype.set_led = function (player, brightness) {
    // there is only one in the hardware
    var led = 1;

    // TODO: remove me later, i am a quickfix
    if(player == 2) {
        player = 1;
    }
    if(player == 1) {
        player = 2;
    }

    var buf = new Buffer("QL$$$Q");

    buf[2] = 'A'.charCodeAt(0) + player;
    buf[3] = '1'.charCodeAt(0) + led;

    // TODO: another quickfix
    if(!brightness) {
        buf[4] = '1'.charCodeAt(0);
    } else {
        buf[4] = '0'.charCodeAt(0);
    }

    // async?
    this.port.write(buf);
};

// close the connection to the buzzer
Buzz.prototype.close = function () {
    this.port.close();
}

exports.Buzz = Buzz

