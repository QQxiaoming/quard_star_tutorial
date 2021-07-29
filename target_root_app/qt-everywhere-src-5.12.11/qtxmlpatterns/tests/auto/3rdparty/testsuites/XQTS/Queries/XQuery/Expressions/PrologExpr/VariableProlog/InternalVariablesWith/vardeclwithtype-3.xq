(: Name: vardeclwithtype-3 :)
(: Description: Evaluates an internal variable declaration with type:)
(: Use type xs:decimal. :)

declare variable $var as xs:decimal := 100;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

$var