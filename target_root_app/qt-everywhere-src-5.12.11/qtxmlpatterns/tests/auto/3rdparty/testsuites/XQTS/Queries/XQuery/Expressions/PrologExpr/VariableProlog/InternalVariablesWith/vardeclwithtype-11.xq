(: Name: vardeclwithtype-11 :)
(: Description: Evaluates an internal variable declaration with type:)
(: Use type xs:date  :)

declare variable $var as xs:date := xs:date("1999-11-28Z");

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

$var