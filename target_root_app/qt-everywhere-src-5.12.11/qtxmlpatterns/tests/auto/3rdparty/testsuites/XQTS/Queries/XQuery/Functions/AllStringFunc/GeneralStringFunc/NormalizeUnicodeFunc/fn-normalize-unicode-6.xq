(: Name: fn-normalize-unicode-6 :)
(: Description: Evaluation of fn:normalize-unicode with nothing to normalize and used as argument to fn:concat:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:concat(fn:normalize-unicode("NORMALIZEDSTRING"),"another string")