(: Name: fn-min-2:)
(: Description: Evaluation of type promotion when using mixed typed with fn:min function:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:min((xs:integer(5000000),xs:double(3e8)))