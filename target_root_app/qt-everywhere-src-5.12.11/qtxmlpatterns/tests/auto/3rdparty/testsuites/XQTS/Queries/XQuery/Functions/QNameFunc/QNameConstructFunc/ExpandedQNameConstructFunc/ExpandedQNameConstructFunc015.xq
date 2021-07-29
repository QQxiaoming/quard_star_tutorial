(: Name: ExpandedQNameConstructFunc015 :)
(: Description: Test function fn:QName. Error case - invalid input type for parameters (integer) :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

element {fn:QName("http://www.example.com/example", xs:integer("100"))}{ "test" }
