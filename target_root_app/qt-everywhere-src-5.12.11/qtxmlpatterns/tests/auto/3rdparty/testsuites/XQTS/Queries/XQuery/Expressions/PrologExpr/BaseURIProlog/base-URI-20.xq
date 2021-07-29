(: Name: base-URI-20 :)
(: Description: Evaluates that base-uri property can contain "base-uri". Used with static-base--uri function.:)

declare base-uri "http://www.base-uri.com";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:static-base-uri())