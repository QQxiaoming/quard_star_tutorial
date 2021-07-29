(: Name: NamespaceURIFromQNameFunc007 :)
(: Description: Test function fn:namespace-uri-from-QName. Empty sequence literal as input :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<empty>{fn:namespace-uri-from-QName(((),()))}</empty>
