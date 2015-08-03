(function(window, $) {
  function attachListeners() {
    var dropdowns = [];

    $(document).mouseup(function(e) {
      dropdowns.forEach(function(dropdown) {
        if (!dropdown[1].is(e.target)) {
          dropdown[1].slideUp();
          dropdown[0].removeClass('dropped-down');
        }
      });
    });

    $('[data-type="dropdown"]').each(function() {
      var nav = $(this);
      var target = nav.data('target');
      if (target) {
        dropdowns.push([nav, $(target)]);
      }
    });

    $('[data-type="dropdown"]').click(function() {
      var nav = $(this);
      var target = nav.data('target');
      if (target) {
        target = $(target);
        if (target.css('display') == 'none') {
          target.slideDown();
          nav.addClass('dropped-down');
        } else {
          target.slideUp();
          nav.removeClass('dropped-down');
        }
      }
    });
  }

  $(window).ready(function() {
    attachListeners();
  });
})(window, window.$ || window.jQuery);
