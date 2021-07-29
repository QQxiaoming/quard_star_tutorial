(: Name: vardeclwithtype-18 :)
(: Description: Evaluates an internal variable declaration with type that uses a declared namespace.:)
(: Two variable with same name declared in namespaces with same namespace uri. :)

declare namespace p1 = "http://www.example.com";
declare namespace p2 = "http://www.example.com";

declare variable $p1:var as xs:integer := 10;
declare variable $p2:var as xs:integer := 20;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

$p2:var