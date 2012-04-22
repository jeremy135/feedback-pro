/**
* Provide the HTML to create the modal dialog.
*/
Drupal.theme.prototype.CToolsFeedbackModal = function () {
  var html = ''

  html += '<div id="ctools-modal">';
 
  html += '              <span id="modal-title" class="modal-title"></span>';
  html += '              <span class="popups-close"><a class="close" href="#"></a></span>';
  html += '              <div class="clear-block"></div>';
  html += '            </div>';
  html += '          <div class="popups-container">';
  html += '            <div class="modal-scroll"><div id="modal-content" class="modal-content ></div></div>';
  html += '            </div>';
  //html += '            <div class="popups-buttons"></div>'; //Maybe someday add the option for some specific buttons.
  //  html += '            <div class="popups-footer"></div>'; //Maybe someday add some footer.

  html += '</div>';

  return html;

}
