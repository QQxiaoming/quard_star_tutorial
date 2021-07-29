(: Name: fn-escape-html-uri-11:)
(: Description: Examines that the fn:escape-html-uri function does not escape the "*" symbol.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:escape-html-uri("example*example")