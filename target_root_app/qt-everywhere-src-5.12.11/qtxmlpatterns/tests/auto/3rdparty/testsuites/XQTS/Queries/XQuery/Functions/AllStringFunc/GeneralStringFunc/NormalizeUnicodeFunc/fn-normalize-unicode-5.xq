(: Name: fn-normalize-unicode-5 :)
(: Description: Evaluation of fn:normalize-unicode with nothing to normalize and used as argument to fn:string-length:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string-length(fn:normalize-unicode("NORMALIZEDSTRING"))