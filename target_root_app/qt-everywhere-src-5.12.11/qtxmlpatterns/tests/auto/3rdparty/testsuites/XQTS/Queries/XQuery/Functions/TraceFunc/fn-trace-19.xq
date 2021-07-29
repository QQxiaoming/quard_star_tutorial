(: Name: fn-trace-19 :)
(: Description: Simple call of "fn:trace" function where the first argument is the empty sequence. :)
(: Uses fn:count to maybe avoid empty file. :)
 fn:count(fn:trace(() ,"The value of the empty sequence is:"))