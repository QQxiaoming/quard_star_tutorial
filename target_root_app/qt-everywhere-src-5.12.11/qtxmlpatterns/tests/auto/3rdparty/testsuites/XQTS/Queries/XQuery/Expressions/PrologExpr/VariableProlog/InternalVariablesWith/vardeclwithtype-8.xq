(: Name: vardeclwithtype-8 :)
(: Description: Evaluates an internal variable declaration with type:)
(: Use type xs:anyURI  :)

declare variable $var as xs:anyURI := xs:anyURI("http://example.com");

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

$var