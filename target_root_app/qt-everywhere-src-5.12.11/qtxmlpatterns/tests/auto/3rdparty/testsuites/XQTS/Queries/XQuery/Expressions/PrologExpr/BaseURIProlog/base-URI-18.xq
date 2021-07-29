(: Name: base-URI-18 :)
(: Description: Evaluates base-uri property can contain the string "&#xa; (newline)". Used with static-base--uri function.:)

declare base-uri "http:/www.abc&#xa;.com";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:static-base-uri())