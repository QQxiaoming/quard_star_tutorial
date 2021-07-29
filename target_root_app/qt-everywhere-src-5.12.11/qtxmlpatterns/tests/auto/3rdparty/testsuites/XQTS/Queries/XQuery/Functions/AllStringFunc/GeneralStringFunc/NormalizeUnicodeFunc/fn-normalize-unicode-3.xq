(: Name: fn-normalize-unicode-3 :)
(: Description: Evaluation of fn:normalize-unicode with nothing to normalize and used as argument to fn:upper-case:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:upper-case(fn:normalize-unicode("normalizedstring"))