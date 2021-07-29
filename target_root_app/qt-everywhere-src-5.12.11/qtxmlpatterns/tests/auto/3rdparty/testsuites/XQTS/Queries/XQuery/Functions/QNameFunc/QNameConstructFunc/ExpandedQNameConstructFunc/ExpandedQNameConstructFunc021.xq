(: Name: ExpandedQNameConstructFunc021 :)
(: Description: Test function fn:QName. URI exists and is linked to a different namespace prefix :)

declare namespace ht="http://www.example.com/example";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

element {fn:QName( "http://www.example.com/example", "ht2:person" )}{ "test" }
