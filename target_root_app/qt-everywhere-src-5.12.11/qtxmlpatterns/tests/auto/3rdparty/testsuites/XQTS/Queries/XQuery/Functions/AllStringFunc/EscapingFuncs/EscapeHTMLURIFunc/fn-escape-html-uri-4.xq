(: Name: fn-escape-html-uri-4:)
(: Description: Examines that the fn:escape-html-uri function does not escape the space.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:escape-html-uri("example example")