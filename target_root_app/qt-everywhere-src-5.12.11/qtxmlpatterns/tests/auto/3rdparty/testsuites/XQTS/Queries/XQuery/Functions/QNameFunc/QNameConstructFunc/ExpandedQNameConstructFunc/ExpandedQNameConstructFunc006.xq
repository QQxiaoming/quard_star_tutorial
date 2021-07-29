(: Name: ExpandedQNameConstructFunc006 :)
(: Description: Test function fn:QName. Error case - local name contains a prefix, but no namespace URI is specified :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

element {fn:QName((), "ht:person")}{ "test" }
