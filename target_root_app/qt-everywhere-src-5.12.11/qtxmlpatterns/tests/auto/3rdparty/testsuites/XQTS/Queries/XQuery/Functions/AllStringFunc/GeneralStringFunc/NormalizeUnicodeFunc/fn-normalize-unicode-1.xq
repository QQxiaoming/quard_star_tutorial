(: Name: fn-normalize-unicode-1 :)
(: Description: Evaluation of fn:normalize-unicode to ensure that " NFC " is the same as "NFC".:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:concat(fn:normalize-unicode("&#x00C5;"," NFC "),fn:normalize-unicode("&#x00C5;","NFC"))