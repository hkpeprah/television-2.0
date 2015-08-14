/*global Pebble */

// Constants
////////////////////////////
var CONFIG_ROOT = 'http://410388.ngrok.com';
var CONFIG_ENDPOINT = '/pebble';

var API_ROOT = CONFIG_ROOT + '/api';
var API_VERSION = 'v1';

var SUBSCRIPTIONS = '/user/<%= user_token %>/subscriptions';
var SETTINGS = '/user/<%= user_token %>';

var ERRORS = {
  'UNKNOWN': 0,
  'NO_TOKEN': 1
};

var APP_KEY_MAPPING = {
  'config': 0,
  'request': 1,
  'crunchyroll_premium': 2,
  'funimation_premium': 3,
  'country': 4,
  'num_subscriptions': 5,
  'id': 6,
  'name': 7,
  'subscribed': 8,
  'network_id': 9,
  'network_name': 10,
  'runtime': 11,
  'latest_name': 12,
  'latest_summary': 13,
  'latest_season': 13,
  'latest_number': 14,
  'latest_timestamp': 15,
  'latest_runtime': 16,
  'error': 17
};

var REQUESTS = {
  'SETTINGS': 0,
  'UPDATE': 1,
  'SUBSCRIBE': 2,
  'UNSUBSCRIBE': 3,
  'SUBSCRIPTIONS': 4
};

// Local Globals
////////////////////////////
var api = null;
var mq = null;
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

// App Message Queue
/////////////////////////////
function MessageQueue() {
  if (!(this instanceof MessageQueue)) {
    return new MessageQueue();
  }
  this.in_progress = false;
  this._queue = [];
  return this;
}

MessageQueue.prototype.send = function() {
  if (this._queue.length == 0 || this.in_progress) {
    return;
  }
  var self = this;
  this.in_progress = true;

  var queue_data = this._queue.shift();
  var obj = queue_data['obj'];
  // On Android there's an issue with sending booleans over AppMessage,
  // so we'll want to convert any Booleans we find in the data object
  // into integer values.
  var keys = [];
  for (var key in obj) {
    if (obj.hasOwnProperty(key)) {
      keys.push(key);
    }
  }

  var msg = {};
  for (var i = 0; i < keys.length; i++) {
    var k = keys[i];
    var v = obj[k];
    if (typeof v === 'boolean') {
      if (v == true) {
        obj[k] = 1;
      } else if (v == false) {
        obj[k] = 0;
      }
    }
    // We have to coerce the message down to the components
    // that make it up.
    if (typeof APP_KEY_MAPPING[k] !== 'undefined') {
      msg[k] = obj[k];
    }
  }

  var success = queue_data['success'];
  var failure = queue_data['failure'];
  Pebble.sendAppMessage(msg, function(res) {
    success(res);
    self.in_progress = false;
    self.send();
  }, function(res) {
    failure(res);
    self.in_progress = false;
    self.send();
  });
};

MessageQueue.prototype.queue = function(data, successHandler, errorHandler) {
  data = data || {};
  successHandler = successHandler || function() {};
  errorHandler = errorHandler || successHandler;
  this._queue.push({ 'obj': data, 'success': successHandler, 'error': errorHandler });
  this.send();
};

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
  error = error || function() {
    mq.queue({ 'error': ERRORS.UKNOWN });
  };
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

Api.prototype.getUserSettings = function(token) {
  token = token || userToken;
  if (!token) {
    mq.queue({ 'error': ERRORS.NO_TOKEN });
    return;
  }
  var url = urljoin(this.baseUrl, formatString(SETTINGS, { 'user_token': token }));
  this.ajax(url, 'GET', null, function(res) {
    var msg = {};
    msg.crunchyroll_premium = res.crunchyroll_premium;
    msg.funimation_premium = res.funimation_premium;
    msg.country = res.country;
    msg.num_subscriptions = res.num_subscriptions;
    mq.queue(msg);
  });
};

Api.prototype.getUserSubscriptions = function(token) {
  token = token || userToken;
  if (!token) {
    mq.queue({ 'error': ERRORS.NO_TOKEN });
    return;
  }
  var url = urljoin(this.baseUrl, formatString(SUBSCRIPTIONS, { 'user_token' : token }));
  this.ajax(url, 'GET', null, function(res) {
    mq.queue({ 'error': ERRORS.UNKNOWN });
  });
};

Api.prototype.updateUserSettings = function(token, settings) {
  var keys = [
    'crunchyroll_premium',
    'funimation_premium',
    'country'
  ];
  var data = {};

  token = token || userToken;
  if (!token) {
    mq.queue({ 'error': ERRORS.NO_TOKEN });
    return;
  }

  for (var i = 0; i < keys.length; i++) {
    var key = keys[i];
    if (typeof settings[key] !== 'undefined') {
      data[key] = settings[key];
    }
  }
  var url = urljoin(this.baseUrl, formatString(SETTINGS, { 'user_token': token }));
  this.ajax(url, 'POST', data, function(res) {
    console.log('Updated user settings: ', JSON.stringify(settings));
  });
};

// Event Listeners
////////////////////////////
Pebble.addEventListener('ready', function() {
  api = new Api();
  mq = new MessageQueue();
  getTimelineToken();
});

Pebble.addEventListener('showConfiguration', function(e) {
  var url = urljoin(CONFIG_ROOT, CONFIG_ENDPOINT);
  getTimelineToken(function(token) {
    url += formatQueryParameters({ user_token: token });
    mq.queue({ config: true }, function() {
      Pebble.openURL(url);
    });
  });
});

Pebble.addEventListener('webviewclosed', function(e) {
  console.log('Telebble: webview closed');
  mq.queue({ config: false });
});

Pebble.addEventListener('appmessage', function(e) {
  if (e.payload) {
    switch (e.payload.request) {
      case REQUESTS.SETTINGS: // Settings Request Key
        api.getUserSettings();
        break;
      case REQUESTS.UPDATE: // Update user settings
        api.updateUserSettings(userToken, e.payload);
        break;
      case REQUESTS.SUBSCRIPTIONS: // Get user subscriptions
        api.getUserSubscriptions(userToken);
        break;
      default:
        console.log('NYI: ', JSON.stringify(e.payload));
    }
  }
});
