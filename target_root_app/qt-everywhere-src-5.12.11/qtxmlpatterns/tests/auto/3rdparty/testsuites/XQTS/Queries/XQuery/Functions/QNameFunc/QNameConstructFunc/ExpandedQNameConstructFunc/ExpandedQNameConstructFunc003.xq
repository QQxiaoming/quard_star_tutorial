(: Name: ExpandedQNameConstructFunc003 :)
(: Description: Test function fn:QName. Simple use case for 'no namespace' QName :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

element {fn:QName("", "person")}{ "test" }
