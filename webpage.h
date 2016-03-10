
#ifndef _WEBPAGE_H

#define _WEBPAGE_H

#include <stdint.h>
#include <avr/pgmspace.h>

#define WEB_PAGE_1  { \
  PSTR(" \
<style>\
body{\
background-color:#FFF;\
}\
*{\
font-family:Courier New;\
color:#2D2A2A;\
font-weight:bold;\
}\
h1{\
font-size:60px;\
text-align:center;\
}\
div.container{\
width:60%;\
margin-left:auto;\
margin-right:auto;\
}\
table{\
margin-left:auto;\
margin-right:auto;\
border:1px solid;\
border-radius:4px;\
width:100%;\
}\
td{\
border-top:1px solid;\
}\
tr td:last-child{\
text-align:end;\
border-left:1px solid;\
}\
th{\
font-size:25px;\
}\
</style>\
<div class=\"container\">\
<h1>AVR Webserver</h1>\
<table>\
<thead>\
<th colspan=\"2\">\
Server status\
</th>\
</thead>\
<tbody>\
<tr>\
<td>Temperature</td><td>\
" \
) \
}

#define WEB_PAGE_2 { \
PSTR(" \
 &#8451;</td>\
</tr>\
<tr>\
<td>Your IP:</td><td>192.255.39.84</td>\
</tr>\
</tbody>\
</table>\
</div>\
<script src=\"http://ajax.googleapis.com/ajax/libs/jquery/1.9.1/jquery.min.js\"> </script>\
<script>\
$(document).ready(function() {\
setInterval(function(){\
  $.ajax({\
  cache: false,\
  type: \"POST\",\
  url: \"http://169.254.222.184/TEMP\",\
  contentType: 'application/json',\
  dataType: \"json\",\
  data: JSON.stringify(null),\
  success: function (result) {\
    $('table tr:first-child td:last-child').html(result + ' &#8451;');\
  }\
});\
},2500);\
});\
</script>\
" \
) \
}

#endif
