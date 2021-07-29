(: Name: vardeclwithtype-14 :)
(: Description: Evaluates an internal variable declaration with type and expression.:)
(: Expression uses a previously defined variable. :)

declare variable $x as xs:integer := 10;
declare variable $var as xs:integer := $x +1;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

$var