const express = require('express');
const mongoose = require('mongoose');
const bodyParser = require('body-parser');

const app = express();
const PORT = 3000;
app.use(bodyParser.urlencoded({ extended: true }));
app.use(bodyParser.json());

mongoose.connect('mongodb+srv://test123:test123@cluster0.p1liq0o.mongodb.net/test', {
  useNewUrlParser: true,
  useUnifiedTopology: true,
});

const buttonSchema = new mongoose.Schema({
  value: String,
  timestamp: Date,
});

const buttonSchema1 = new mongoose.Schema({
  value: String,
  timestamp: Date,
});
const billSchema = new mongoose.Schema({
  loadtype: String,
  value: String,
  timestamp: Date,
});
const moment = require('moment-timezone');
const Button = mongoose.model('Button', buttonSchema);
const Pc = mongoose.model('Pc', buttonSchema1);
const Bill = mongoose.model('Bill',billSchema);

app.get('/', (req, res) => {
    res.sendFile(__dirname + '/index.html');
  });
app.post('/button', async(req, res) => {
  const { value } = req.body;
  const timestamp = moment().tz('Asia/Kolkata').format();
  const button = new Button({ value, timestamp });
  button.save()
    .then(() => res.send('added to database'))
    .catch((err) => console.error(err));
});

app.get('/button', async (req, res) => {
  try {
    const lastTwoEntries = await Button.find().sort({ timestamp: -1}).limit(2);
    const timeDifference = Math.floor((lastTwoEntries[0].timestamp - lastTwoEntries[1].timestamp) / 1000);
    var energy = 0.009 * (timeDifference/3600);
    const billCollection = mongoose.connection.client.db('test').collection('bills');
    const billDocument = { loadtype:"LED", value: energy, timestamp: new Date() };
    const result = await billCollection.insertOne(billDocument);
    console.log(`Energy saved to bill collection with ID: ${result.insertedId}`);
    res.json({energy});
  } catch (error) {
    res.status(500).json({ error: 'Failed to fetch timestamps' });
  }
});

app.post('/pc', async(req, res) => {
  const { value } = req.body;
  const timestamp = moment().tz('Asia/Kolkata').format();
  const pc = new Pc({ value, timestamp });
  pc.save()
    .then(() => res.send('added to database'))
    .catch((err) => console.error(err));
});

app.get('/pc', async (req, res) => {
  try {
    const lastTwoEntries = await Pc.find().sort({ timestamp: -1}).limit(2);
    const timeDifference = Math.floor((lastTwoEntries[0].timestamp - lastTwoEntries[1].timestamp) / 1000);
    var energy = 0.07 * (timeDifference/3600);
    const billCollection = mongoose.connection.client.db('test').collection('bills');
    const billDocument = { loadtype:"PC", value: energy, timestamp: new Date() };
    const result = await billCollection.insertOne(billDocument);
    res.json({energy});

  } catch (error) {
    res.status(500).json({ error: 'Failed to fetch timestamps' });
  }
});

app.get('/total', async (req, res) => {
  try {
    const bills = await Bill.aggregate([
      { $group: { _id: null, total: { $sum: "$value" } } }
    ]);
    const total = bills[0].total;
    res.send(total.toString());
  } catch (error) {
    console.error(error);
    res.sendStatus(500);
  }
});

app.get('/lightEnergy', async (req, res) => {
  try {
    const bills = await Bill.aggregate([
      { $match: { loadtype: "LED" } },
      { $group: { _id: null, total: { $sum: "$value" } } }
    ]);
    const lightEnergy = bills[0].total;
    res.send(lightEnergy.toString());
  } catch (error) {
    console.error(error);
    res.sendStatus(500);
  }
});

app.get('/pcEnergy', async (req, res) => {
  try {
    const bills = await Bill.aggregate([
      { $match: { loadtype: "PC" } },
      { $group: { _id: null, total: { $sum: "$value" } } }
    ]);
    const pcEnergy = bills[0].total;
    res.send(pcEnergy.toString());
  } catch (error) {
    console.error(error);
    res.sendStatus(500);
  }
});





app.listen(PORT, () => console.log(`running ${PORT}`));
