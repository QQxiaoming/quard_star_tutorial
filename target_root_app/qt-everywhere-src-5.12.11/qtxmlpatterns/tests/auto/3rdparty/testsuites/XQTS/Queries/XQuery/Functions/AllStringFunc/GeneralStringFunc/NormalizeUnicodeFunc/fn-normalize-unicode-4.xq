(: Name: fn-normalize-unicode-4 :)
(: Description: Evaluation of fn:normalize-unicode with nothing to normalize and used as argument to fn:lower-case:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:lower-case(fn:normalize-unicode("NORMALIZEDSTRING"))