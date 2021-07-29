(: Name: NamespaceURIFromQNameFunc016 :)
(: Description: Test function fn:namespace-uri-from-QName. Error case - invalid parameter type (integer)  :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:namespace-uri-from-QName(xs:integer("100"))