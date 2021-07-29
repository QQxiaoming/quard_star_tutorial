(: Name: LocalNameFromQNameFunc010 :)
(: Description: Test function fn:local-name-from-QName. Error case - multiple input values :)

declare default element namespace "http://www.example.com/QNameXSD";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:local-name-from-QName($input-context/root/elemQN)