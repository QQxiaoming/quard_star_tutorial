(: Name: base-URI-19 :)
(: Description: Evaluates base-uri property can contain "declarebase-uri". Used with static-base--uri function.:)

declare base-uri "http://declarebase-uri.com";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:static-base-uri())