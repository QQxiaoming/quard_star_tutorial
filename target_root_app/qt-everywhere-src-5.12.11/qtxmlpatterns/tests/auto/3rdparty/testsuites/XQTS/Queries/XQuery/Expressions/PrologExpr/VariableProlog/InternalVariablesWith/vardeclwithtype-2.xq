(: Name: vardeclwithtype-2 :)
(: Description: Evaluates an internal variable declaration with type:)
(: Use type xs:integer. :)

declare variable $var as xs:integer := 100;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

$var