(: Name: vardeclwithtype-22 :)
(: Description: Evaluates an internal variable declaration with type that uses the fn:empty function .:)

declare variable $var as xs:boolean := fn:empty((1,2,3));

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

$var