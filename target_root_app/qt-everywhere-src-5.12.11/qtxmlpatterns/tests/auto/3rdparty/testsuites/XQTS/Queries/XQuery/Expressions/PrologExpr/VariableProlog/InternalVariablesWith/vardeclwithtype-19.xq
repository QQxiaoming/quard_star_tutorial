(: Name: vardeclwithtype-19 :)
(: Description: Evaluates an internal variable declaration with type that uses the fn:count function .:)

declare variable $var as xs:integer := fn:count((10,2));

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

$var