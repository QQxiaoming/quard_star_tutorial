(: Name: fn-trace-20 :)
(: Description: Simple call of "fn:trace" function where the first argument is the zero length string. :)
(: Uses fn:count to maybe avoid empty file. :)
 fn:count(fn:trace("" ,"The value of the zero length string is:"))