(: Name: vardeclwithtype-9 :)
(: Description: Evaluates an internal variable declaration with type:)
(: Use type xs:dateTime  :)

declare variable $var as xs:dateTime := xs:dateTime("1999-11-28T09:00:00Z");

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

$var