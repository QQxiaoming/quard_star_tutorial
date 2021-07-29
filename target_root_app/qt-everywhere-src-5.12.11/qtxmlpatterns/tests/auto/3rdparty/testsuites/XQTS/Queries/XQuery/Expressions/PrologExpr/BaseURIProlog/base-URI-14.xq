(: Name: base-URI-14 :)
(: Description: Evaluates base-uri property can contain "##0;". Used with static-base--uri function.:)

declare base-uri 'http://www.example.com/abc##0;';

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:static-base-uri())