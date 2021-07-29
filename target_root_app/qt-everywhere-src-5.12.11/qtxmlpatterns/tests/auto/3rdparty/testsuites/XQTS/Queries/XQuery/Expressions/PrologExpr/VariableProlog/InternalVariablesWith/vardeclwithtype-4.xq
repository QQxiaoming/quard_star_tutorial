(: Name: vardeclwithtype-4 :)
(: Description: Evaluates an internal variable declaration with type:)
(: Use type xs:boolean (true value). :)

declare variable $var as xs:boolean := fn:true();

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

$var