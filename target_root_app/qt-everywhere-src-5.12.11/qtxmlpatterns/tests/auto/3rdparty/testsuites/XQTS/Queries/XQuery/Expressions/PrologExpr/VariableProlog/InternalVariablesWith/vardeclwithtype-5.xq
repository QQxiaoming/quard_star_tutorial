(: Name: vardeclwithtype-5 :)
(: Description: Evaluates an internal variable declaration with type:)
(: Use type xs:boolean (false value). :)

declare variable $var as xs:boolean := fn:false();

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

$var