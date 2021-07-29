(: Name: base-URI-22 :)
(: Description:Verify normalization of xs:anyURI (leading spaces). Used with static-base--uri function.:)

declare base-uri " http://www.example.org/examples";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:static-base-uri())