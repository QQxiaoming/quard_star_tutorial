(: Name: fn-normalize-unicode-7 :)
(: Description: Evaluation of fn:normalize-unicode with nothing to normalize and used as argument to fn:substring:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:substring(fn:normalize-unicode("NORMALIZEDSTRING"),5)