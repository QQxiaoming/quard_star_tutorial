(: Name: fn-encode-for-uri-12 :)
(: Description: Examines that fn-encode-for-uri function does not escape numbers. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(fn:encode-for-uri("examples0123456789example"))