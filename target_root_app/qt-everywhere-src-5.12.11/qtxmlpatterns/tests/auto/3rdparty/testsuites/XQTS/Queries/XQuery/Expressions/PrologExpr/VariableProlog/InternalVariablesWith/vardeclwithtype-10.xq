(: Name: vardeclwithtype-10 :)
(: Description: Evaluates an internal variable declaration with type:)
(: Use type xs:time  :)

declare variable $var as xs:time := xs:time("11:12:00Z");

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

$var