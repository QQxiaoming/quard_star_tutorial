(: Name: vardeclwithtype-6 :)
(: Description: Evaluates an internal variable declaration with type:)
(: Use type xs:float  :)

declare variable $var as xs:float := xs:float(12.5E10);

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

$var