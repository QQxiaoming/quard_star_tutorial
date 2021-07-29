(: Name: fn-escape-html-uri-3:)
(: Description: Examines that the fn:escape-html-uri function does not escape digits.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:escape-html-uri("a0123456789")