(: Name: LocalNameFromQNameFunc012 :)
(: Description: Test function fn:local-name-from-QName. Error case - typo in function name :)

declare default element namespace "http://www.example.com/QNameXSD"; 

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:local-name-from-Qname(($input-context/root/elemQN)[1])