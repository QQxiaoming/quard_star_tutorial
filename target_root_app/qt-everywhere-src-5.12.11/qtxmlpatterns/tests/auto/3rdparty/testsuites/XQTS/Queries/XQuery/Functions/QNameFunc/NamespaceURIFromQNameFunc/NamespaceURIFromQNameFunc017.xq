(: Name: NamespaceURIFromQNameFunc017 :)
(: Description: Test function fn:namespace-uri-from-QName. Error case - invalid parameter type (time)  :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:namespace-uri-from-QName(xs:time("12:00:00Z"))