(: Name: LocalNameFromQNameFunc014 :)
(: Description: Test function fn:local-name-from-QName. Error case - typo in function name :)

declare default element namespace "http://www.example.com/QNameXSD"; 

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:localnamefromQName(($input-context/root/elemQN)[1])