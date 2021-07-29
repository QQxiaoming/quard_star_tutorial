(: Name: ExpandedQNameConstructFunc002 :)
(: Description: Test function fn:QName. Simple use case from functions and operators spec :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

element {fn:QName("http://www.example.com/example", "ht:person")}{ "test" }
