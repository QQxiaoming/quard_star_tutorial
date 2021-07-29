(: Name: fn-escape-html-uri-1:)
(: Description: Examines that the fn:escape-html-uri function does not escape the lower cases letters.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:escape-html-uri("abcdedfghijklmnopqrstuvwxyz")