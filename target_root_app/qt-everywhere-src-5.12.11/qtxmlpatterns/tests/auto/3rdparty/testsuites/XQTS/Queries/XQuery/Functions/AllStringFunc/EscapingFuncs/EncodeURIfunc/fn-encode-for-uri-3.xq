(: Name: fn-encode-for-uri-3 :)
(: Description: Examines that fn-encode-for-uri function does not escape the "-" symbol. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(fn:encode-for-uri("examples-example"))