(: Name: fn-replaceErr-2:)
(: Description:the value of $replacement contains a "\" character that is not part of a "\\" pair, unless it is immediately followed by a "$" character.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:replace("abracadabra", "bra", "\") 