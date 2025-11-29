
const express = require('express');
var admin = require("firebase-admin");
var serviceAccount = require('../smart-blinds-rtdb-firebase-adminsdk-fbsvc-241619ba3b.json');


//connect to db
admin.initializeApp({
  credential: admin.credential.cert(serviceAccount),
  databaseURL: "https://smart-blinds-rtdb-default-rtdb.firebaseio.com"
});

//app setup
const app = express();
app.use(express.json());
app.use(express.static('public'));
const db = admin.database();

//middleware - static files that will be served to the browser
app.use(express.static('public'));

//Get sensor data to display on webapp (fetched from database)
app.get('/sensor', async (req, res) => {
  const ldr = await db.ref('Sensor/ldr_data').once('value');
  const voltage = await db.ref('Sensor/voltage').once('value');

  res.json({
    ldr: ldr.val(),
    voltage: voltage.val()
  });
  console.log("LDR data fetched");
});

app.get('/motor', async (req, res) => {
  const turnCW = await db.ref('Motor/turnCW').once('value');
  const turnCounterCW = await db.ref('Motor/turnCounterCW').once('value');
  const manual = await db.ref('Motor/manual').once('value');

  res.json({
    turnCW: turnCW.val(),
    turnCounterCW: turnCounterCW.val(),
    manual: manual.val()
  });
  console.log("Motor data fetched");
});

// send bool data from app to DB: set manual mode (true) or light mode (false)
app.post('/blinds/isManual', async (req, res) => {
  const { state } = req.body; //state: true = manual mode, false = light mode
  await db.ref('/Motor/manual').set(state);
  res.json({ success: true });
});


// send bool data from app to DB: do we want to turn the blinds (after enabling)?
app.post('/blinds/turnCW', async (req, res) => {
  const { state } = req.body; //state: true/false
  await db.ref('/Motor/turnCW').set(state);
  res.json({ success: true });
  console.log("Turn command sent!");
})

app.post('/blinds/turnCounterCW', async (req, res) => {
  const { state } = req.body; //state: true/false
  await db.ref('/Motor/turnCounterCW').set(state);
  res.json({ success: true });
  console.log("Turn command sent!");
})

// Automatic light sensor control
// If in light mode (manual = false):
//   - If it's dark (ldr_data < 1000), set motor/turn to true
//   - If it's bright (ldr_data >= 1000), set motor/turn to false
async function checkLightSensorMode() {
  try {
    const ldr = await db.ref('Sensor/ldr_data').once('value');
    const manual = await db.ref('Motor/manual').once('value');

    const ldrValue = ldr.val();
    const manualValue = manual.val();

    // Only control motor automatically if in light mode (manual = false)
    if (ldrValue !== null && manualValue === false) {
      if (ldrValue < 1000) {
        // It's dark, turn the motor
        await db.ref('/Motor/turn').set(true);
        console.log(`Light sensor mode: LDR=${ldrValue} (dark), setting motor/turn to true`);
      } else {
        // It's bright, turn off the motor
        await db.ref('/Motor/turn').set(false);
        console.log(`Light sensor mode: LDR=${ldrValue} (bright), setting motor/turn to false`);
      }
    }
  } catch (error) {
    console.error("Error checking light sensor mode:", error);
  }
}

// Check light sensor mode every 5 seconds
setInterval(checkLightSensorMode, 5000);

// host web server
const server = app.listen(4000, function() {
  console.log("Server listening on port 4000");
  console.log("Connected to Firebase Realtime Database");
  console.log("Light sensor auto-control enabled");
});