/*global Pebble */

// Constants
////////////////////////////
var CONFIG_ROOT = 'http://410388.ngrok.com';
var CONFIG_ENDPOINT = '/pebble';

var API_ROOT = CONFIG_ROOT + '/api';
var API_VERSION = 'v1';

var SUBSCRIPTIONS = '/user/<%= user_token %>/subscriptions';
var SETTINGS = '/user/<%= user_token %>';

// Local Globals
////////////////////////////
var userToken = null;

// Functions
////////////////////////////
function formatString(string, data) {
  for (var key in data) {
    if (data.hasOwnProperty(key)) {
      var val = data[key];
      var pattern = new RegExp('<%=\\s*' + key + '\\s*%>', 'gi');
      string = string.replace(pattern, val);
    }
  }
  return string;
}

function urlJoin() {
  var frags = Array.prototype.slice.call(arguments);
  var url = '';
  for (var i = 0; i < frags.length; i++) {
    var len = url.length;
    var flen = frags[i].length;
    if (len == 0) {
      url += frags[i];
    } else if (url[len - 1] == '/') {
      if (frags[i][0] == '/') {
        url += frags[i].substring(1, flen);
      } else {
        url += frags[i];
      }
    } else {
      if (frags[i][0] == '/') {
        url += frags[i];
      } else {
        url += '/' + frags[i];
      }
    }
  }
  return url;
}

function formatQueryParameters(params) {
  var queryParams = [];
  for (var key in params) {
    if (params.hasOwnProperty(key)) {
      var val = params[key];
      queryParams.push(encodeURIComponent(key) + '=' + encodeURIComponent(val));
    }
  }
  return '?' + queryParams.join('&');
}

function getTimelineToken(callback) {
  callback = callback || function() {};
  if (userToken) {
    callback(userToken);
    return;
  }
  Pebble.getTimelineToken(function(token) {
    userToken = token;
    console.log('Grabbed user token: ', token);
    callback(userToken);
  });
}

function sendAppMessage(data, successHandler, errorHandler) {
  data = data || {};
  successHandler = successHandler || function() {};
  errorHandler = errorHandler || successHandler;

  // On Android there's an issue with sending booleans over AppMessage,
  // so we'll want to convert any Booleans we find in the data object
  // into integer values.
  var keys = [];
  for (var key in data) {
    if (data.hasOwnProperty(key)) {
      keys.push(key);
    }
  }

  for (var i = 0; i < keys.length; i++) {
    var k = keys[i];
    var v = data[k];
    if (typeof v === 'boolean') {
      if (v == true) {
        data[k] = 1;
      } else if (v == false) {
        data[k] = 0;
      }
    }
  }

  console.log(JSON.stringify(data));
  Pebble.sendAppMessage(data, successHandler, errorHandler);
}

// Event Listeners
////////////////////////////
Pebble.addEventListener('ready', function() {
  console.log('Telebble: Javascript is ready');
  getTimelineToken();
});

Pebble.addEventListener('showConfiguration', function(e) {
  var url = urlJoin(CONFIG_ROOT, CONFIG_ENDPOINT);
  getTimelineToken(function(token) {
    url += formatQueryParameters({ user_token: token });
    sendAppMessage({ config: true }, function() {
      console.log(url);
      Pebble.openURL(url);
    });
  });
});

Pebble.addEventListener('webviewclosed', function(e) {
  console.log('Telebble: webview closed');
  sendAppMessage({ config: false });
});

Pebble.addEventListener('appmessage', function(e) {
  console.log('Received message: ' + JSON.stringify(e.payload));
});
