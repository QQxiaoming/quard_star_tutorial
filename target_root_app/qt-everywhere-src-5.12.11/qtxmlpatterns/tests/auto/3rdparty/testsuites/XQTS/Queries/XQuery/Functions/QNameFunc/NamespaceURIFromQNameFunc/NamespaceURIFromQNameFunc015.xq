(: Name: NamespaceURIFromQNameFunc015 :)
(: Description: Test function fn:namespace-uri-from-QName. Error case - invalid parameter type (simple type)  :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:namespace-uri-from-QName(($input-context//Folder)[1])
