(: Name: base-URI-23 :)
(: Description:Verify normalization of xs:anyURI (trailing spaces). Used with static-base--uri function.:)

declare base-uri "http://www.example.org/examples  ";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:static-base-uri())