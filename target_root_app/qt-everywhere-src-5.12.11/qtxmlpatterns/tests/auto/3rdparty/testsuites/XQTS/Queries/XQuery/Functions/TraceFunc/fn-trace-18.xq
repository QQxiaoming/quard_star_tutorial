(: Name: fn-trace-18 :)
(: Description: Simple call of "fn:trace" function used in a math expression involving boolean operations ("or" and fn:true()). :)

 fn:trace((for $var in (fn:true(),fn:false(),fn:true()) return $var or fn:true()) ,"The value of 'for $var in (fn:true(),fn:false(),fn:true() return $var or fn:true()' is:")