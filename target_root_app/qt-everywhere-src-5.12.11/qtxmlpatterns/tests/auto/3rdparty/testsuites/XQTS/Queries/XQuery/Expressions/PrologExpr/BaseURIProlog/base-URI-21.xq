(: Name: base-URI-21 :)
(: Description: Evaluates that base-uri property can contain "BASE-URI". Used with static-base-uri function.:)

declare base-uri "http://www.BASE-URI.com";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:static-base-uri())