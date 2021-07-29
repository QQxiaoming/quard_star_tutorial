(: Name: vardeclwithtype-1 :)
(: Description: Evaluates an internal variable declaration with type:)
(: Use type xs:string. :)

declare variable $var as xs:string := "abc";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

$var