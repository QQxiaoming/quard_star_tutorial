(: Name: vardeclwithtype-7 :)
(: Description: Evaluates an internal variable declaration with type:)
(: Use type xs:double  :)

declare variable $var as xs:double := xs:double(1267.43233E12);

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

$var