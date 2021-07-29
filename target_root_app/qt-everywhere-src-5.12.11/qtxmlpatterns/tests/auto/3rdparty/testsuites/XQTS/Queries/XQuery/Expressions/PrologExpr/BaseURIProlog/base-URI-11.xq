(: Name: base-URI-11 :)
(: Description: Evaluates base-uri property can contain numbers. Used with static-base--uri function.:)

declare base-uri "http://www.example.com/abc123";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:static-base-uri())