const http = require('http');
const fs = require('fs');
const { execSync } = require('child_process')

const hostname = '127.0.0.1';
const port = 8000;

let router = (req, res) => {
  var url = req.url;
  if ('/' == url) {
    fs.readFile('index.html', function (err, data) {
      res.writeHead(200, {'Content-Type': 'text/html'});
      res.write(data);
      res.end();
    });
  } else if ('/favicon.ico' == url) {
    fs.readFile('./favicon.ico', function (err, data) {
      res.writeHead(200, {'Content-Type': 'image/x-icon'});
      res.write(data);
      res.end();
    });
  } else if ('/css/index.css' == url) {
    fs.readFile('./css/index.css', function (err, data) {
      res.writeHead(200, {'Content-Type': 'text/css'});
      res.write(data);
      res.end();
    });
  } else if ('/script/index.js' == url) {
    fs.readFile('./script/index.js', function (err, data) {
      res.writeHead(200, {'Content-Type': 'text/javascript'});
      res.write(data);
      res.end();
    });
  } else if ('/script/jquery-3.6.1.min.js' == url) {
    fs.readFile('./script/jquery-3.6.1.min.js', function (err, data) {
      res.writeHead(200, {'Content-Type': 'text/javascript'});
      res.write(data);
      res.end();
    });
  } else if ('/result/result.css' == url) {
    fs.readFile('./result/result.css', function (err, data) {
      res.writeHead(200, {'Content-Type': 'text/css'});
      res.write(data);
      res.end();
    });
  } else if ('/save' == url) {
    var reqdata = '';
    req.on('data', function(chunk) {reqdata += chunk})
      .on('end', function() {
      let pair = decodeURI(reqdata).split("=", 2);
      let value = decodeURIComponent(pair[1]);
      fs.writeFileSync("./result/temp.md", value + "\n");
      execSync("\"./bin/main.bin\" \"./result/temp.md\"");
      fs.readFile('./result/result.html', function (err, data) {
        res.writeHead(200, {'Content-Type': 'text/html'});
        res.write(encodeURI(data));
        res.end();
      });
    });
  }
}

const server = http.createServer();
server.on('request', router);
server.listen(port, hostname, () => {
  console.log(`Server running at http://${hostname}:${port}/`);
});

