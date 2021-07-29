(: Name: LocalNameFromQNameFunc016 :)
(: Description: Test function fn:local-name-from-QName. Error case - invalid parameter type (integer)  :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:local-name-from-QName(xs:integer("100"))