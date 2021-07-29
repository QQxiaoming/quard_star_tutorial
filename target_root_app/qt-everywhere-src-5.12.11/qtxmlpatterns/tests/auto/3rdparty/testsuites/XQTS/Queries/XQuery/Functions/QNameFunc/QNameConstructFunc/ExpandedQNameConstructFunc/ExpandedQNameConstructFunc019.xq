(: Name: ExpandedQNameConstructFunc019 :)
(: Description: Test function fn:QName. Error case - zero length string for local-name :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

element {fn:QName( "http://www.example.com/example", "" )}{ "test" }
