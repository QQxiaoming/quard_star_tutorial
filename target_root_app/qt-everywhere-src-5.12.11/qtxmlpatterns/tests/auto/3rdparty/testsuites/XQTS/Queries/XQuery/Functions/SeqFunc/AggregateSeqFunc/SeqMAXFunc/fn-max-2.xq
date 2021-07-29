(: Name: fn-max-2:)
(: Description: Evaluation of type promotion when using mixed typed with fn:max function:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:max((xs:integer(5000000000),xs:double(3e0)))