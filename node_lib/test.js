var buzz = require('./buzz.js');

b = new buzz.Buzz("/dev/ttyUSB0");

b.set_led(1, 0);

b.on("button", function(key) {
    console.log(key);
});

//b.close();

