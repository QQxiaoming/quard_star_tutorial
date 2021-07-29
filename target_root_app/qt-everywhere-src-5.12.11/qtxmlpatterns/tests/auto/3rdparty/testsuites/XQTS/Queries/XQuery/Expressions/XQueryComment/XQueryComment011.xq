(: Name: XQueryComment011 :)
(: Description: Comments inside a conditional expression :)

declare namespace fs="http://www.example.com/filesystem";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

if (:test:)(:t2:)(:t3:) ($input-context/fs:MyComputer) 
	then "true"
	else "false"

