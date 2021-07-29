(: Name: ExpandedQNameConstructFunc020 :)
(: Description: Test function fn:QName. Local-name references an already defined namespace prefix which is assigned to a different URI:)

declare namespace ht="http://www.example.com/example";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

element {fn:QName( "http://www.example.com/another-example", "ht:person" )}{ "test" }
