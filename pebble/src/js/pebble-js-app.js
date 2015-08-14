/*global Pebble */

// Constants
////////////////////////////
var CONFIG_ROOT = 'http://410388.ngrok.com';
var CONFIG_ENDPOINT = '/pebble';

var API_ROOT = CONFIG_ROOT + '/api';
var API_VERSION = 'v1';

var SUBSCRIPTIONS = '/user/<%= user_token %>/subscriptions';
var SETTINGS = '/user/<%= user_token %>';

var APP_KEY_MAPPING = {
  'config': 0,
  'request': 1,
  'crunchyroll_premium': 2,
  'funimation_premium': 3,
  'country': 4,
  'num_subscriptions': 5
};

var REQUESTS = {
  'SETTINGS': 0,
  'UPDATE': 1,
  'SUBSCRIBE': 2,
  'UNSUBSCRIBE': 3
};

// Local Globals
////////////////////////////
var api = null;
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

function urljoin() {
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

  var msg = {};
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
    // We have to coerce the message down to the components
    // that make it up.
    if (typeof APP_KEY_MAPPING[k] !== 'undefined') {
      msg[k] = data[k];
    }
  }

  Pebble.sendAppMessage(msg, successHandler, errorHandler);
}

// API Handlers
/////////////////////////////
function Api() {
  if (!(this instanceof Api)) {
    return new Api();
  }
  this.baseUrl = urljoin(API_ROOT, API_VERSION);
  return this;
}

Api.prototype.ajax = function (url, method, data, success, error) {
  var xhr = new XMLHttpRequest();
  success = success || function() {};
  error = error || function() {};
  xhr.open(method, url, true);
  xhr.onload = function() {
    try {
      var res = JSON.parse(xhr.responseText);
      success(res);
    } catch (e) {
      console.log('Error in ajax request: ' + e);
      error(e, xhr.responseText);
    }
  };
  if (method == 'POST') {
    xhr.setRequestHeader('Content-Type', 'application/json; charset=UTF-8');
  }
  xhr.send(data ? JSON.stringify(data) : data);
};

Api.prototype.fetchUserSettings = function(token) {
  token = token || userToken;
  if (!token) {
    return;
  }
  var url = urljoin(this.baseUrl, formatString(SETTINGS, { 'user_token': token }));
  this.ajax(url, 'GET', null, function(res) {
    var msg = {};
    msg.crunchyroll_premium = res.crunchyroll_premium;
    msg.funimation_premium = res.funimation_premium;
    msg.country = res.country;
    msg.num_subscriptions = res.num_subscriptions;
    sendAppMessage(msg);
  });
};

Api.prototype.updateUserSettings = function(token, settings) {
  var keys = [
    'crunchyroll_premium',
    'funimation_premium',
    'country'
  ];
  var data = {};
  for (var i = 0; i < keys.length; i++) {
    var key = keys[i];
    if (typeof settings[key] !== 'undefined') {
      data[key] = settings[key];
    }
  }
  var url = urljoin(this.baseUrl, formatString(SETTINGS, { 'user_token': token || userToken }));
  this.ajax(url, 'POST', data, function(res) {
    console.log('Updated user settings: ', JSON.stringify(settings));
  });
};

// Event Listeners
////////////////////////////
Pebble.addEventListener('ready', function() {
  api = new Api();
  getTimelineToken();
});

Pebble.addEventListener('showConfiguration', function(e) {
  var url = urljoin(CONFIG_ROOT, CONFIG_ENDPOINT);
  getTimelineToken(function(token) {
    url += formatQueryParameters({ user_token: token });
    sendAppMessage({ config: true }, function() {
      Pebble.openURL(url);
    });
  });
});

Pebble.addEventListener('webviewclosed', function(e) {
  console.log('Telebble: webview closed');
  sendAppMessage({ config: false });
});

Pebble.addEventListener('appmessage', function(e) {
  if (e.payload) {
    switch (e.payload.request) {
      case REQUESTS.SETTINGS: // Settings Request Key
        api.fetchUserSettings();
        break;
      case REQUESTS.UPDATE: // Update user settings
        api.updateUserSettings(userToken, e.payload);
        break;
      default:
        console.log('NYI: ', JSON.stringify(e.payload));
    }
  }
});
