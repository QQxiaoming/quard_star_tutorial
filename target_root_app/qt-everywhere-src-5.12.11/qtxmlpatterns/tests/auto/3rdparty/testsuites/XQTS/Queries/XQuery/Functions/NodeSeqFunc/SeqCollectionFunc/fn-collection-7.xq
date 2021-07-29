(: Name: fn-collection-7 :)
(: Description: Return elements that immediately contain TCP/IP.:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:collection($input-context)//*[text()[contains(.,"TCP/IP")]]

