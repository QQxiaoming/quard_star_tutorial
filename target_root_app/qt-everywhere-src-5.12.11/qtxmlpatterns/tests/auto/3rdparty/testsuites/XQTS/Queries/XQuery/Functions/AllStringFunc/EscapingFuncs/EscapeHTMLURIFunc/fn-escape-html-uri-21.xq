(: Name: fn-escape-html-uri-21:)
(: Description: Examines that the fn:escape-html-uri function does escape the euro symbol.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:escape-html-uri("example&#x20AC;example")