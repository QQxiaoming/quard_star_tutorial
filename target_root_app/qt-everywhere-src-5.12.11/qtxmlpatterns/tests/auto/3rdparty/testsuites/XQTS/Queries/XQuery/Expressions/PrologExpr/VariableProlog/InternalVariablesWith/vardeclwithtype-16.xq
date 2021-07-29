(: Name: vardeclwithtype-16 :)
(: Description: Evaluates an internal variable declaration with type that uses a declared namesapce.:)

declare namespace p1 = "http://www.example.com";
declare variable $p1:var as xs:integer := 10;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

$p1:var