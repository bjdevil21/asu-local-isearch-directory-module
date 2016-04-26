(function ($) {
  Drupal.behaviors.asu_isearch_pane_config = {
    attach: function (context, settings) {
      $('#toggle-all-employee-types').click(function(){
        var checked_state = $(this).prop('checked');
        $('.employee-types-group input:checkbox').each(function(){
          $(this).prop('checked', checked_state);
        });
      });

      $('.employee-types-group input:checkbox').each(function(){
        $(this).change(function(){
          if ($(this).prop('checked') === false) {
            $('#toggle-all-employee-types').prop('checked', false);
          }
        });
      });
    }
  }
})(jQuery);
