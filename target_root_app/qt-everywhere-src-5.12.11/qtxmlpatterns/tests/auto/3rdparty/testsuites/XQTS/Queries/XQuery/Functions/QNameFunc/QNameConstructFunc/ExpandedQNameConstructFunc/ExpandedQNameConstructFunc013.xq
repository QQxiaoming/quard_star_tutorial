(: Name: ExpandedQNameConstructFunc013 :)
(: Description: Test function fn:QName. Error case - wrong number of input parameters:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

element {fn:QName("person")}{ "test" }
