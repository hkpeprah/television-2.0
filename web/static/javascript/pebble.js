(function(window, $, _) {
  var api;
  var searchText = '';
  var defaultPage = 1;
  var currentPage = 1;
  var endOfResults = false;

  function API(user_token) {
    if (!(this instanceof API)) {
      return new API(user_token);
    }

    this.version = 'v1';
    this.baseUrl = '/api/' + this.version + '/';
    this.transactionId = 0;

    this.user_token = user_token;
    this.crunchyroll_premium = false;
    this.funimation_premium = false;
    this.country = 'US';

    return this;
  }

  API.prototype.subscribe = function(sid) {
  };

  API.prototype.unsubscribe = function(sid) {
  };

  API.prototype.getUser = function(callback) {
    if (!this.user_token) {
      return;
    }
    var self = this;
    var endpoint = 'user/' + this.user_token;
    var url = this.baseUrl + endpoint;

    callback = callback || function() {};
    $.getJSON(url, function(data) {
      callback(data);
    });
  };

  API.prototype.updateUser = function(settingName, settingValue) {
  };

  API.prototype.search = function(query, page, callback) {
    var endpoint = 'series';
    var self = this;
    var url = this.baseUrl + endpoint;
    var tid = this.transactionId + 1;
    this.transactionId = tid;
    url += formatQueryParameters({ 'q': query, 'page': page });
    callback = callback || function() {};
    $.getJSON(url, function(data) {
      if (self.transactionId == tid) {
        callback(data);
      }
    });    
  };

  function formatQueryParameters(params) {
    var queryParams = [];
    var keys = Object.keys(params);
    for (var i = 0; i < keys.length; i++) {
      var key = keys[i];
      var val = params[key];
      queryParams.push(encodeURIComponent(key) + '=' + encodeURIComponent(val));
    }
    return '?' + queryParams.join('&');
  }

  function onSearchResults(data) {
    if (typeof data == 'undefined') {
      endOfResults = true;
    }

    var items = data.items;
    var current_page = data.metadata.current_page;
    var num_pages = data.metadata.num_pages;
    var target = $('#search-results');

    currentPage = current_page;
    if (currentPage == defaultPage) {
      target.children().remove();
    }

    if (current_page == num_pages || items.length == 0) {
      endOfResults = true;
    }

    var tmpl = $('#series-template').html();
    $(items).each(function(idx, data) {
      if (data && data.latest && data.latest.air_date) {
        var date = data.latest.air_date;
        var d = new Date(date.substring(0, date.length - 1));
        data.latest.air_date = d.toLocaleString();
      }
      var rendered = _.template(tmpl)(data);
      var obj = $(rendered);
      target.append(obj);
    });
  }

  function attachListeners() {
    var $links = $([]);
    var $targets = $([]);
    var $window = $(window);
    $('a').each(function() {
      var href = $(this).attr('href');
      if (href && href[0] == '#' && href.length > 1) {
        $links = $links.add($(this));
        $targets = $targets.add($(href));
      }
    });

    $links.click(function(ev) {
      var idx = $links.index($(this));
      ev.preventDefault();
      ev.stopPropagation();
      $targets
        .css('display', 'none')
        .eq(idx)
        .css('display', 'block');
    });

    if ($links.length) {
      $links.first().click();
    }

    $('input#search').on('keyup', function(ev) {
      if (ev.which == 13) {
        searchText = $(this).val();
        ev.preventDefault();
        api.search(searchText, defaultPage, onSearchResults);
      }
    });

    $window.on('scroll', function(ev) {
      ev.preventDefault();
      ev.stopPropagation();

      if ($window.data('ajaxready') == false || !searchText || !searchText.length) {
        return;
      }

      if ($targets.eq(0).css('display') == 'none') {
        return;
      }

      // Mark false so that won't load more content
      $window.data('ajaxready', false);

      // Determine if we actually should load in new content
      var shouldLoad = false;
      shouldLoad = $window.scrollTop() >= $(document).height() - $window.outerHeight();
      if (shouldLoad) {
        api.search(searchText, currentPage + 1, function(data) {
          onSearchResults(data);
          $window.data('ajaxready', true);
        });
      } else {
        $window.data('ajaxready', true);
      }
    });
  }

  $(window).ready(function() {
    var userToken = $.QueryString['user_token'];
    api = API(userToken);
    api.getUser(function(data) {
      if (!data) {
        return;
      }
      api.crunchyroll_premium = data.crunchyroll_premium;
      api.funimation_premium = data.funimation_premium;
      api.country = data.country;
    });
    attachListeners();
  });
})(window, window.$ || window.jQuery, window._ || window.underscore);
