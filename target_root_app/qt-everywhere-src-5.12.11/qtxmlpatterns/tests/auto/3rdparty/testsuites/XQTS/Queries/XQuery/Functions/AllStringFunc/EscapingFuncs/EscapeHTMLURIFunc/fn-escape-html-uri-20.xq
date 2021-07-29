(: Name: fn-escape-html-uri-20:)
(: Description: Examines that the fn:escape-html-uri function does escape the euro symbol.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:escape-html-uri("example&#xe9;&#x20AC;example")