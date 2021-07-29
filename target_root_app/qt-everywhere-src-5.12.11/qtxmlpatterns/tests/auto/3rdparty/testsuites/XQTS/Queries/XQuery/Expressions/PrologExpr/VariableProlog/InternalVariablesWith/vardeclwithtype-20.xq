(: Name: vardeclwithtype-20 :)
(: Description: Evaluates an internal variable declaration with type that uses the fn:string-length function .:)

declare variable $var as xs:integer := fn:string-length("ABC");

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

$var