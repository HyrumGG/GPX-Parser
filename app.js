'use strict'

// C library API
const ffi = require('ffi-napi');

const mysql = require('mysql2/promise')

let gpxparser = ffi.Library('./libgpxparser', {
  'createGPXFileFromJSON' : ['string', ['string', 'string']],
  'createGPXJSONFromFile' : ['string', ['string']],
  'getGPXComponents' : ['string', ['string']],
  'gpxDataListToJSON' : ['string', ['string', 'int', 'int']],
  'renameComponent' : ['string', ['string', 'string', 'int', 'int']],
  'createRoute' : ['string', ['string', 'string', 'string']],
  'getPathBetween' : ['string', ['string', 'float', 'float', 'float', 'float', 'float']],
  'waypointListToJSON' : ['string', ['string', 'int']]
});

// Express App (Routes)
const express = require("express");
const app = express();
const path = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());
app.use(express.static(path.join(__dirname + '/uploads')));

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');
const { connect } = require('http2');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/', function (req, res) {
  res.sendFile(path.join(__dirname + '/public/index.html'));
});

// Send Style, do not change
app.get('/style.css', function (req, res) {
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname + '/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js', function (req, res) {
  fs.readFile(path.join(__dirname + '/public/index.js'), 'utf8', function (err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {
      compact: true,
      controlFlowFlattening: true
    });
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function (req, res) {
  if (!req.files) {
    return res.status(400).send('No files were uploaded.');
  }

  let uploadFile = req.files.uploadFile;

  // Verify the file does not exist already
  if (doesFileExist(uploadFile.name)) {
    return res.status(400).send('');
  }
  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function (err) {
    if (err) {
      return res.status(500).send(err);
    }
    res.redirect('/');
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function (req, res) {
  fs.stat('uploads/' + req.params.name, function (err, stat) {
    if (err == null) {
      res.sendFile(path.join(__dirname + '/uploads/' + req.params.name));
    } else {
      console.log('Error in file downloading route: ' + err);
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** 

app.get('/getSingleGPX', function (req, res) {
  let str = {};
  let result = true;

  if (req.query.name.substr(req.query.name.length - 4) === '.gpx') {
    str = JSON.parse(gpxparser.createGPXJSONFromFile('uploads/' + req.query.name));

    if (Object.keys(str).length == 0) {
      // Remove file and return error
      fs.unlink('uploads/' + req.query.name, function (err) {
        if (err) console.log(err);
      });
      result = false;
    }
  } else {
    // Remove file and return error
    fs.unlink('uploads/' + req.query.name, function (err) {
      if (err) console.log(err);
    });
    result = false;
  }
  str["filename"] = req.query.name;
  res.send({
    son: str,
    stat: result
  });
});

app.get('/getAllGPX', function (req, res) {
  let fls = fs.readdirSync('uploads/');
  let arr = [];
  let result = true;

  for (let i in fls) {

    if (fls[i].substr(fls[i].length - 4) === '.gpx') {
      let str = JSON.parse(gpxparser.createGPXJSONFromFile('uploads/' + fls[i]));

      if (Object.keys(str).length > 0) {
        str['filename'] = fls[i];
        arr.push(str);
      } else {
        // Remove file and set status
        fs.unlink('uploads/' + fls[i], function (err) {
          if (err) console.log(err);
        });
        result = false;
      }
    } else if (fls[i] === '.gitkeep') {
      // Remove gitkeep
      fs.unlink('uploads/' + fls[i], function (err) {
        if (err) console.log(err);
      });
    } else {
      // Remove file and set status
      fs.unlink('uploads/' + fls[i], function (err) {
        if (err) console.log(err);
      });
      result = false;
    }
  }
  res.send({
    array: arr,
    stat: result
  });
});

app.get('/showComponents', function (req, res) {
  let str = JSON.parse(gpxparser.getGPXComponents('uploads/' + req.query.filename));
  res.send(str);
});

app.get('/showOtherData', function (req, res) {
  let spl = req.query.component.split(' ');
  let str = {};

  if (spl[0] === 'Route') {
    str = JSON.parse(gpxparser.gpxDataListToJSON('uploads/' + req.query.filename, 1, parseInt(spl[1])));
  } else {
    str = JSON.parse(gpxparser.gpxDataListToJSON('uploads/' + req.query.filename, 0, parseInt(spl[1])));
  }
  res.send(str);
});

app.get('/renameComponent', function (req, res) {
  let spl = req.query.component.split(' ');
  let str = {};

  if (spl[0] === 'Route') {
    str = JSON.parse(gpxparser.renameComponent('uploads/' + req.query.filename, req.query.name, 1, parseInt(spl[1])));
  } else {
    str = JSON.parse(gpxparser.renameComponent('uploads/' + req.query.filename, req.query.name, 0, parseInt(spl[1])));
  }
  res.send(str);
});

app.get('/createGPX', function (req, res) {
  let str = {};
  str['status'] = false;
  if (!doesFileExist(req.query.filename)) {
    let obj = '{\"version\":' + req.query.version + ',\"creator\":\"' + req.query.creator + '\"}';
    str = JSON.parse(gpxparser.createGPXFileFromJSON(obj, 'uploads/' + req.query.filename));
  }
  res.send({
    stat: str.status
  });
});

app.get('/addRoute', function (req, res) {
  let str = JSON.parse(gpxparser.createRoute('uploads/' + req.query.filename, req.query.name, req.query.wpts));
  res.send(str);
});

app.get('/getPathBetween', function (req, res) {
  let fls = fs.readdirSync('uploads/');
  let routeList = [];
  let trackList = [];
  let stat = true;

  for (let i in fls) {
    let str = JSON.parse(gpxparser.getPathBetween('uploads/' + fls[i], req.query.start.lat, req.query.start.lon, req.query.end.lat, req.query.end.lon, req.query.delta));
    if (str.status) {
      if (str.routes.length > 0) {
        routeList = routeList.concat(str.routes);
      }
      if (str.tracks.length > 0) {
        trackList = trackList.concat(str.tracks);
      }
    } else {
      stat = false;
    }
  }
  res.send({
    routeList,
    trackList,
    status: stat
  });
});

let connection;
let credentials;

app.get('/dblogin', async function (req, res) {
  let stat = true;
  credentials = {
    host : 'dursley.socs.uoguelph.ca',
    user : req.query.username,
    password : req.query.password,
    database : req.query.databaseName
  };

  try {
    connection = await mysql.createConnection(credentials);

    await connection.execute("CREATE TABLE IF NOT EXISTS FILE ("
      + "gpx_id INT AUTO_INCREMENT PRIMARY KEY, "
      + "file_name VARCHAR(60) NOT NULL, "
      + "ver DECIMAL(2,1) NOT NULL, "
      + "creator VARCHAR(256) NOT NULL)");
    await connection.execute("CREATE TABLE IF NOT EXISTS ROUTE ("
      + "route_id INT AUTO_INCREMENT PRIMARY KEY, "
      + "route_name VARCHAR(256), "
      + "route_len FLOAT(15,7) NOT NULL, "
      + "gpx_id INT NOT NULL, "
      + "FOREIGN KEY(gpx_id) REFERENCES FILE(gpx_id) ON DELETE CASCADE)");
    await connection.execute('CREATE TABLE IF NOT EXISTS POINT ('
      + 'point_id INT AUTO_INCREMENT PRIMARY KEY, '
      + 'point_index INT NOT NULL, '
      + 'latitude DECIMAL(11,7) NOT NULL, '
      + 'longitude DECIMAL(11,7) NOT NULL, '
      + 'point_name VARCHAR(256), '
      + 'route_id INT NOT NULL, '
      + 'FOREIGN KEY(route_id) REFERENCES ROUTE(route_id) ON DELETE CASCADE)');
  } catch (e) {
    console.log("Query error: " + e);
    stat = false;
  } finally {
    if (connection && connection.end) connection.end();
  }
  res.send({status: stat});
});

app.get('/storeFiles', async function (req, res) {
  let stat = true;
  try {
    connection = await mysql.createConnection(credentials);

    let fls = fs.readdirSync('uploads/');
    for (let i in fls) {
      // Check if file exists
      let [row, field] = await connection.execute('SELECT COUNT(1) FROM FILE WHERE file_name = \'' + fls[i] + '\'');
      if (row[0]['COUNT(1)'] == 0) {
        // insert file information
        let str = JSON.parse(gpxparser.createGPXJSONFromFile('uploads/' + fls[i]));
        await connection.execute('INSERT INTO FILE (file_name, ver, creator) VALUES (' 
        + '\'' + fls[i] + '\', '
        + '\'' + str.version + '\', '
        + '\'' + str.creator + '\')');

        // Insert Route
        let [row1, field1] = await connection.execute('SELECT LAST_INSERT_ID()');
        let gpxId = row1[0]['LAST_INSERT_ID()'];

        str = JSON.parse(gpxparser.getGPXComponents('uploads/' + fls[i]));
        for (let j in str.routes) {
          // Set route name to null if missing
          let rtName = str.routes[j].name;
          if (rtName === '') {
            rtName = 'NULL';
          }
          await connection.execute('INSERT INTO ROUTE (route_name, route_len, gpx_id) VALUES ('
          + '\'' + rtName + '\', '
          + '\'' + str.routes[j].len + '\', '
          + '\'' + gpxId + '\')');

          // Insert Points
          [row1, field1] = await connection.execute('SELECT LAST_INSERT_ID()');
          let rteId = row1[0]['LAST_INSERT_ID()'];
          let wptStr = JSON.parse(gpxparser.waypointListToJSON('uploads/' + fls[i], j));

          for (let k in wptStr) {
            let wptName = wptStr[k].name;
            if (wptName === '') {
              wptName = 'NULL';
            }
            await connection.execute('INSERT INTO POINT (point_index, latitude, longitude, point_name, route_id) VALUES ('
            + '\'' + wptStr[k].index + '\', '
            + '\'' + wptStr[k].lat + '\', '
            + '\'' + wptStr[k].lon + '\', '
            + '\'' + wptName + '\', '
            + '\'' + rteId + '\')');
          }
        }
      }
    }
  } catch (e) {
    console.log("Query error: " + e);
    stat = false;
  } finally {
    if (connection && connection.end) connection.end();
  }
  res.send({status: stat});
});

app.get('/clearData', async function(req, res) {
  let stat = true;
  try {
    connection = await mysql.createConnection(credentials);
    await connection.execute('DELETE FROM FILE');
  } catch (e) {
    console.log("Query error: " + e);
    stat = false;
  } finally {
    if (connection && connection.end) connection.end();
  }
  res.send({status: stat});
});

app.get('/displayStatus', async function(req, res) {
  let stat = true;
  let numFiles;
  let numRoutes;
  let numPoints;
  try {
    connection = await mysql.createConnection(credentials);
    let [row, field] = await connection.execute('SELECT COUNT(*) FROM FILE');
    numFiles = row[0]['COUNT(*)'];
    [row, field] = await connection.execute('SELECT COUNT(*) FROM ROUTE');
    numRoutes = row[0]['COUNT(*)'];
    [row, field] = await connection.execute('SELECT COUNT(*) FROM POINT');
    numPoints = row[0]['COUNT(*)'];
  } catch (e) {
    console.log("Query error: " + e);
    stat = false;
  } finally {
    if (connection && connection.end) connection.end();
  }
  res.send({files: numFiles, routes: numRoutes, points: numPoints, status: stat});
});

app.get('/updateFile', async function (req, res) {
  let stat = true;
  let ex = false;
  try {
    connection = await mysql.createConnection(credentials);

    // Check if file exists
    let [row, field] = await connection.execute('SELECT COUNT(1) FROM FILE WHERE file_name = \'' + req.query.file + '\'');
    if (row[0]['COUNT(1)'] != 0) {
      ex = true;
      await connection.execute('DELETE FROM FILE WHERE file_name = \'' + req.query.file + '\'');
      // insert file information
      let str = JSON.parse(gpxparser.createGPXJSONFromFile('uploads/' + req.query.file));
      await connection.execute('INSERT INTO FILE (file_name, ver, creator) VALUES (' 
      + '\'' + req.query.file + '\', '
      + '\'' + str.version + '\', '
      + '\'' + str.creator + '\')');

      // Insert Route
      let [row1, field1] = await connection.execute('SELECT LAST_INSERT_ID()');
      let gpxId = row1[0]['LAST_INSERT_ID()'];

      str = JSON.parse(gpxparser.getGPXComponents('uploads/' + req.query.file));
      for (let j in str.routes) {
        // Set route name to null if missing
        let rtName = str.routes[j].name;
        if (rtName === '') {
          rtName = 'NULL';
        }
        await connection.execute('INSERT INTO ROUTE (route_name, route_len, gpx_id) VALUES ('
        + '\'' + rtName + '\', '
        + '\'' + str.routes[j].len + '\', '
        + '\'' + gpxId + '\')');

        // Insert Points
        [row1, field1] = await connection.execute('SELECT LAST_INSERT_ID()');
        let rteId = row1[0]['LAST_INSERT_ID()'];
        let wptStr = JSON.parse(gpxparser.waypointListToJSON('uploads/' + req.query.file, j));

        for (let k in wptStr) {
          let wptName = wptStr[k].name;
          if (wptName === '') {
            wptName = 'NULL';
          }
          await connection.execute('INSERT INTO POINT (point_index, latitude, longitude, point_name, route_id) VALUES ('
          + '\'' + wptStr[k].index + '\', '
          + '\'' + wptStr[k].lat + '\', '
          + '\'' + wptStr[k].lon + '\', '
          + '\'' + wptName + '\', '
          + '\'' + rteId + '\')');
        }
      }
    }
  } catch (e) {
    console.log("Query error: " + e);
    stat = false;
  } finally {
    if (connection && connection.end) connection.end();
  }
  res.send({status: stat, exists: ex});
});

app.get('/routeQuery', async function(req, res) {
  let stat = true;
  let [rows2, fields2] = [];
  let [rows1, fields1] = [];
  let routelist = [];

  try {
    connection = await mysql.createConnection(credentials);
    if (req.query.allFiles === 'true') {
      [rows1, fields1] = await connection.execute('SELECT * FROM FILE');

      for (let i in rows1) {
        let gpxId = rows1[i].gpx_id;
        if (req.query.type === 'Sort By Name') {
          [rows2, fields2] = await connection.execute('SELECT * FROM ROUTE WHERE gpx_id = \'' + gpxId + '\' ORDER BY route_name');
        } else {
          [rows2, fields2] = await connection.execute('SELECT * FROM ROUTE WHERE gpx_id = \'' + gpxId + '\' ORDER BY route_len');
        }
        for (let j in rows2) {
          rows2[j]['filename'] = rows1[i].file_name;
        }
        routelist = routelist.concat(rows2);
        
        if (req.query.type === 'Sort By Name') {
          routelist.sort(function(a, b) {
            return a['route_name'].toLowerCase().localeCompare(b['route_name'].toLowerCase());
          });
        } else {
          routelist.sort(function(a, b) {
            return a['route_len'] - b['route_len'];
          });
        }
      }
    } else {
      [rows2, fields2] = await connection.execute('SELECT * FROM FILE WHERE file_name = \'' + req.query.filename + '\'');
      let gpxId = rows2[0].gpx_id;
      if (req.query.type === 'Sort By Name') {
        [rows2, fields2] = await connection.execute('SELECT * FROM ROUTE WHERE gpx_id = \'' + gpxId + '\' ORDER BY route_name');
      } else {
        [rows2, fields2] = await connection.execute('SELECT * FROM ROUTE WHERE gpx_id = \'' + gpxId + '\' ORDER BY route_len');
      }
      for (let j in rows2) {
        rows2[j]['filename'] = req.query.filename;
      }
      routelist = routelist.concat(rows2);
    }
  } catch(e) {
    console.log("Query error: " + e);
    stat = false;
  } finally {
    if (connection && connection.end) connection.end();
  }
  res.send({status: stat, routes: routelist});
});

app.get('/pointQuery', async function(req, res) {
  let stat = true;
  let [rows2, fields2] = [];
  try {
    connection = await mysql.createConnection(credentials);
    [rows2, fields2] = await connection.execute('SELECT * FROM FILE WHERE file_name = \'' + req.query.filename + '\'');
    let gpxId = rows2[0].gpx_id;
    [rows2, fields2] = await connection.execute('SELECT * FROM ROUTE WHERE route_name = \'' + req.query.rteName + '\' AND gpx_id = \'' + gpxId + '\'');
    if (typeof rows2[0] !== 'undefined') {
      let rteId = rows2[0].route_id;
      [rows2, fields2] = await connection.execute('SELECT * FROM POINT WHERE route_id = \'' + rteId + '\' ORDER BY point_index');
    }
  } catch(e) {
    console.log("Query error: " + e);
    stat = false;
  } finally {
    if (connection && connection.end) connection.end();
  }
  res.send({status: stat, points: rows2});
});

app.get('/pointFileQuery', async function(req, res) {
  let stat = true;
  let [rows2, fields2] = [];
  let [rows1, fields1] = [];
  let qPoints = [];
  try {
    connection = await mysql.createConnection(credentials);
    [rows2, fields2] = await connection.execute('SELECT * FROM FILE WHERE file_name = \'' + req.query.filename + '\'');
    let gpxId = rows2[0].gpx_id;
    if (req.query.type === 'Sort By Route Name') {
      [rows1, fields1] = await connection.execute('SELECT * FROM ROUTE WHERE gpx_id = \'' + gpxId + '\' ORDER BY route_name');
    } else {
      [rows1, fields1] = await connection.execute('SELECT * FROM ROUTE WHERE gpx_id = \'' + gpxId + '\' ORDER BY route_len');
    }
    let k = 0;
    for (let i in rows1) {
      if (rows1[i].route_name === 'NULL') {
        k++;
      }
      let rteId = rows1[i].route_id;
      [rows2, fields2] = await connection.execute('SELECT * FROM POINT WHERE route_id = \'' + rteId + '\' ORDER BY point_index');
      for (let j in rows2) {
        if (rows1[i].route_name === 'NULL') {
          rows2[j]['route_name'] = 'Unnamed route ' + k;
        } else {
          rows2[j]['route_name'] = rows1[i].route_name;
        }
        rows2[j]['route_len'] = rows1[i].route_len;
        rows2[j]['route_id'] = rows1[i].route_id;
      }
      qPoints = qPoints.concat(rows2);
    }
  } catch(e) {
    console.log("Query error: " + e);
    stat = false;
  } finally {
    if (connection && connection.end) connection.end();
  }
  res.send({status: stat, points: qPoints});
});

app.get('/routeLongestQuery', async function(req, res) {
  let stat = true;
  let [rows2, fields2] = [];

  try {
    connection = await mysql.createConnection(credentials);
    [rows2, fields2] = await connection.execute('SELECT * FROM FILE WHERE file_name = \'' + req.query.filename + '\'');
    let gpxId = rows2[0].gpx_id;
    if (req.query.asc === 'Shortest') {
      [rows2, fields2] = await connection.execute('SELECT * FROM ROUTE WHERE gpx_id = \'' + gpxId + '\' ORDER BY route_len');
    } else {
      [rows2, fields2] = await connection.execute('SELECT * FROM ROUTE WHERE gpx_id = \'' + gpxId + '\' ORDER BY route_len DESC');
    }
    if (rows2.length >= req.query.size) {
      rows2.length = req.query.size;
    }
    if (req.query.type === 'Sort By Name') {
      rows2.sort(function (a, b) {
        return a['route_name'].toLowerCase().localeCompare(b['route_name'].toLowerCase());
      });
    }
  } catch(e) {
    console.log("Query error: " + e);
    stat = false;
  } finally {
    if (connection && connection.end) connection.end();
  }
  res.send({status: stat, routes: rows2});
});

app.get('/getFilesDB', async function(req, res) {
  let stat = true;
  let [rows2, fields2] = [];
  try {
    connection = await mysql.createConnection(credentials);
    [rows2, fields2] = await connection.execute('SELECT file_name FROM FILE');
  } catch(e) {
    console.log("Query error: " + e);
    stat = false;
  }
  res.send({status: stat, files: rows2});
});

function doesFileExist(file) {
  let fls = fs.readdirSync('uploads/');

  for (let i in fls) {
    if (fls[i] === file) {
      return true;
    }
  }
  return false;
}

app.listen(portNum);
console.log('Running app at localhost:' + portNum);
