(: Name: XQueryComment009 :)
(: Description: Comments inside a conditional expression :)

declare namespace fs="http://www.example.com/filesystem";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

if (:test (: yada (: neato :) :) :) ($input-context/fs:MyComputer) 
	then (: yada :) "true"
	else "false"
