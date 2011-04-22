var buzz = require('./buzz.js');

b = new buzz.Buzz("/dev/ttyUSB0");

//b.set_led(0, 0, 1);
b.set_led(0, 1, 1);
//b.set_led(1, 0, 0);
b.set_led(1, 1, 0);

b.on("button", function(key) {
    console.log(key);
});

//b.close();

