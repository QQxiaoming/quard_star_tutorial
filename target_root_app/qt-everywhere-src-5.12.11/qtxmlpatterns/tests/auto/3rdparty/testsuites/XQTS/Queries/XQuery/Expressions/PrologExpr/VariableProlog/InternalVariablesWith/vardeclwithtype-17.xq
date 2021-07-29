(: Name: vardeclwithtype-17 :)
(: Description: Evaluates an internal variable declaration with type that uses a declared namespace.:)
(: Two variable with same name declared in two deferent namespaces. :)

declare namespace p1 = "http://www.example.com";
declare namespace p2 = "http://www.example.com/examples";

declare variable $p1:var as xs:integer := 10;
declare variable $p2:var as xs:integer := 20;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

$p2:var