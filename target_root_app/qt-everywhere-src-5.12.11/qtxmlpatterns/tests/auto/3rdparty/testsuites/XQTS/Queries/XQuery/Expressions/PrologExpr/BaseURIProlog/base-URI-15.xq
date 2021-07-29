(: Name: base-URI-15 :)
(: Description: Evaluates base-uri property can contain a single character (after the "http://"section). Used with static-base-uri function.:)

declare base-uri "http://A";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:static-base-uri())