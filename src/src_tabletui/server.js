var express = require('express');
var app = express();
var path = require('path');
var server = require('http').createServer(app);
var io = require('socket.io')(server);
var express = require('express');
var session = require('express-session');
var bodyParser = require('body-parser');
var sessionstore = require('sessionstore');
var os = require("os");
var chalk = require('chalk');
var mqtt = require('mqtt');
var config = require('./config.json');
var uuidv1 = require('uuid/v1');
var got = require('got');
var mqtt = require('mqtt');
var mqtt_client = mqtt.connect(config.mqtt_server || 'mqtt://127.0.0.1');
var fs = require('fs');




//const rosnodejs = require('rosnodejs');
//const std_msgs = rosnodejs.require('std_msgs').msg;







//{"display":{"curr_kmh":0,"curr_kn":0,"alert":0,"motor":1,"light":0, "alert_msg":"12m3"}}
var VERSION = 0.2;

var port = process.env.PORT || config.webserver_default_port || 3000;

//----------------------------- EXPRESS APP SETUP ------------------------------------------//
app.set('trust proxy', 1);
app.use(function (req, res, next) {
    if (!req.session) {
        return next(); //handle error
    }
    next(); //otherwise continue
});
app.set('views', __dirname + '/views');
app.engine('html', require('ejs').renderFile);
// Routing
app.use(express.static(path.join(__dirname, 'public')));
app.use(session({
    secret: 'ssshhfddghjhgewretjrdhfsgdfadvsgvshthhh',
    store: sessionstore.createSessionStore(),
    resave: true,
    saveUninitialized: true
}));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({
    extended: true
}));

app.use(function (req, res, next) {
    res.header("Access-Control-Allow-Origin", "*");
    res.header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
    next();
});

server.listen(port, function () {
    console.log('Server listening at port %d', port);
});

var rosnode = null;
var pub = null;


mqtt_client.on('connect', function () {
    mqtt_client.subscribe('tablet_node', function (err) {
        if (!err) {
            mqtt_client.publish('tablet_status', JSON.stringify({state:'ok', msg:'table_ui_server_started'}));
        }
    });
});

mqtt_client.on('message', function (topic, message) {
    // message is Buffer
    if (topic !== "tablet_node"){
        console.log("--- MSG IS NOT: tablet_node ");
        console.log(topic);
        console.log(message);
        return;
    }
    console.log(message.toString());
    
    var json = message;
    try {
        json = JSON.parse(message);
    } catch (error) {

    }

    var ctl = 0;
    if (json.ctlmode) {
        ctl = 0;
    } else {
        ctl = 1;
    }
    io.emit('broadcast', {
        display: {
            kn: json.kn,
            kmh: json.kmh,
            kompressordruck: json.kompressordruck,
            breaklevel: json.breaklevel,
            direction: json.direction,
            state_v0: json.state_v0,
            state_v1: json.state_v1,
            state_v2: json.state_v2,
            state_v3: json.state_v3,
            ctlmode: ctl,
            asc_state: json.asc_state,
            fire_detcted: json.fire_detcted,
            temperature: json.temperature,
            batt_charge: json.batt_charge,
            light_state: json.light_state,
            asc_rest_dist: json.asc_rest_dist,
            emergencybrake: json.emergencybrake,
            kompressorstate: json.kompressorstate,
            lightstate: json.lightstate,
            kompressor_power_state: json.kompressor_power_state,
            emergencybrakereset: json.emergencybrakereset,
            hupe_state: json.hupe_state,
            storedenergy: json.storedenergy,
            storemode: json.storemode,
            vellevl: json.vellevl,
            vel:json.vel

        }
    });

});









var last_update = Math.round(new Date().getTime() / 1000);


app.get('/', function (req, res) {
    sess = req.session;
    res.render('index.ejs', {
    });
});

app.get('/slider', function (req, res) {
    sess = req.session;
    res.render('index_slider.ejs', {
    });
});

app.get('/break', function (req, res) {
    sess = req.session;
    res.render('emergencyctl.ejs', {
    });
});

app.get('/hupe', function (req, res) {
    sess = req.session;
    res.render('hupe.ejs', {
    });
});


io.on('connection', (socket) => {
    socket.on('disconnect', function () {
        console.log('user disconnected');
        if (mqtt_client != null) {
            mqtt_client.publish('tablet_status', JSON.stringify({ state: 'userevent', msg: 'user_connected' }));
        }
    });



    socket.on('event', function (_msg) {
        if (_msg.state == undefined){
            _msg.state = -1;
        }
        console.log(JSON.stringify(_msg )+ ' event');
        if (mqtt_client != null) {       
            mqtt_client.publish('tablet_event', JSON.stringify(_msg));
        }
    });


});


function RESET_ALL() {
    last_update = Math.round(new Date().getTime() / 1000);
}
RESET_ALL(); //AUSGANGSZUSTAND




