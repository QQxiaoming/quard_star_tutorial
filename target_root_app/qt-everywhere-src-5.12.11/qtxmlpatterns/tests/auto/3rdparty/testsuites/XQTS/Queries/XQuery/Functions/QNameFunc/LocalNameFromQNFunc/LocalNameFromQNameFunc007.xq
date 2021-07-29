(: Name: LocalNameFromQNameFunc007 :)
(: Description: Test function fn:local-name-from-QName. Empty sequence literal as input :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<empty>{fn:local-name-from-QName(((),()))}</empty>
