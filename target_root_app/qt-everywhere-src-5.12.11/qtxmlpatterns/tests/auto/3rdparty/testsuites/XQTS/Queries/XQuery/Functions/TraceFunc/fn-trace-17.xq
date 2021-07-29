(: Name: fn-trace-17 :)
(: Description: Simple call of "fn:trace" function used in a math expression involving boolean operations ("and" and fn:true()). :)

 fn:trace((for $var in (fn:true(),fn:false(),fn:true()) return $var and fn:true()) ,"The value of 'for $var in (fn:true(),fn:false(),fn:true() return $var and fn:true()' is:")