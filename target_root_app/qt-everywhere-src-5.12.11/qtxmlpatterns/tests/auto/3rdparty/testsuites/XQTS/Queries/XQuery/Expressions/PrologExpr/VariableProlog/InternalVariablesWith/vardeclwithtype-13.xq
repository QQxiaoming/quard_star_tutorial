(: Name: vardeclwithtype-13 :)
(: Description: Evaluates an internal variable declaration with type and expression.:)
(: Expression result does not match given type. :)

declare variable $var as xs:date := fn:true() and fn:true();

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

$var