(: Name: ExpandedQNameConstructFunc012 :)
(: Description: Test function fn:QName. Error case - invalid lexical representation for the local-name part :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

element {fn:QName("http://www.example.com/example", "person:")}{ "test" }
