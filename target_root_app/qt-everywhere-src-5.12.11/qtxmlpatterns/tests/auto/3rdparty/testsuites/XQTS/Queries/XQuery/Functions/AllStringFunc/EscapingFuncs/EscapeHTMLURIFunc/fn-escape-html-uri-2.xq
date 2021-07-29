(: Name: fn-escape-html-uri-2:)
(: Description: Examines that the fn:escape-html-uri function does not escape the upper cases letters.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:escape-html-uri("ABCDEFGHIJKLMNOPQRSTUVWXYZ")