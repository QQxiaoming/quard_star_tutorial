(: Name: LocalNameFromQNameFunc015 :)
(: Description: Test function fn:local-name-from-QName. Error case - invalid parameter type (simple type)  :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:local-name-from-QName(($input-context//Folder)[1])
