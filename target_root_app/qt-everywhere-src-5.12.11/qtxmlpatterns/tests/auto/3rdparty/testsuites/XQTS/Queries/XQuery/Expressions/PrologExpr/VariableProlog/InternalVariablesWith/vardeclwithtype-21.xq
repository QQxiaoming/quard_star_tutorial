(: Name: vardeclwithtype-21 :)
(: Description: Evaluates an internal variable declaration with type that uses the fn:not function .:)

declare variable $var as xs:boolean := fn:not(fn:true());

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

$var