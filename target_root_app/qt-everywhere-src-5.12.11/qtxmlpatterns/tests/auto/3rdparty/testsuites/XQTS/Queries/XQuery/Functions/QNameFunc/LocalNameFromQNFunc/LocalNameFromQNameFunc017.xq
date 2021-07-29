(: Name: LocalNameFromQNameFunc017 :)
(: Description: Test function fn:local-name-from-QName. Error case - invalid parameter type (time)  :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:local-name-from-QName(xs:time("12:00:00Z"))