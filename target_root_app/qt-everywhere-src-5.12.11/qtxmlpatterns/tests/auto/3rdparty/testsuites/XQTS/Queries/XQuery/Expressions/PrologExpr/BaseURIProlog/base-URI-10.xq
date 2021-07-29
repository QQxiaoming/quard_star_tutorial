(: Name: base-URI-10 :)
(: Description: Evaluates base-uri with the fn:static-base-uri function.  base-uri not defined:)
(: Use fn:count to avoid empty file :)


(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:static-base-uri())