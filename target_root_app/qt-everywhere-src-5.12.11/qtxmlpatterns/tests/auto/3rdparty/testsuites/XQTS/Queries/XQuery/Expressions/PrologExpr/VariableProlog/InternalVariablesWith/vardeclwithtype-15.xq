(: Name: vardeclwithtype-15 :)
(: Description: Evaluates an internal variable declaration with type and expression.:)
(: Test usage of variable with no assigned value at time of expression definition. :)

declare variable $var as xs:integer := $e +1;
declare variable $e as xs:integer := 10;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

$var