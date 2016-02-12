
#ifndef _WEBPAGE_H

#define _WEBPAGE_H

#include <stdint.h>
#include <avr/pgmspace.h>

#define WEB_PAGE  { \
  PSTR(" \
  <style>\
    \
    body {\
      background-color: #FFF;\
    }\
  \
    * {\
      font-family:Courier New;\
      color: #2D2A2A;\
      font-weight: bold;\
    }\
   \
    h1 {\
      font-size: 60px;\
      text-align: center;\
    }\
  \
    div.container {\
      width: 60%;\
      margin-left: auto;\
      margin-right: auto;\
    }\
   \
    table {\
      margin-left: auto;\
      margin-right: auto;\
      border: 1px solid #2D2A2A;\
      border-radius: 4px;\
      width: 100%;\
    }\
    \
    table tbody td {\
      border-top: 1px solid #2D2A2A;\
    }\
    \
    table tr td:last-child{\
      text-align: end;\
      border-left: 1px solid #2D2A2A;\
    }\
    \
    table th {\
      font-size: 25px;\
    }\
   \
   \
  </style>\
  \
  \
  <div class=\"container\">\
    <h1>AVR Webserver<h1>\
  \
    <table>\
      <thead>\
        <th colspan=\"2\">\
          Server status\
        </th>\
      </thead>\
      <tbody>\
        <tr>\
        <td>Temperature</td><td>22 &#8451;</td>\
        </tr>\
        <tr>\
        <td>Your IP:</td><td>192.255.39.84</td>\
        </tr>      \
      </tbody>\
    </table>\
  </div>\
  " \
  ) \
}

#endif