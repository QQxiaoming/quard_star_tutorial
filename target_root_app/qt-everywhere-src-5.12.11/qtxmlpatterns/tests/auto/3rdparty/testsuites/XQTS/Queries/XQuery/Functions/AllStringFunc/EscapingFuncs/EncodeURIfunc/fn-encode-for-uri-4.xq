(: Name: fn-encode-for-uri-4 :)
(: Description: Examines that fn-encode-for-uri function does not escape the "_" symbol. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(fn:encode-for-uri("examples_example"))