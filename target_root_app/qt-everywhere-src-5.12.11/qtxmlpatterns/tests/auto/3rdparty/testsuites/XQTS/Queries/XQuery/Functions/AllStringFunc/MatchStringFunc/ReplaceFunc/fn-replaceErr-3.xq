(: Name: fn-replaceErr-3:)
(: Description: The value of $replacement contains a "$" character that is not immediately followed by a digit 0-9 and not immediately preceded by a "\".:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:replace("abracadabra", "bra", "$y") 