(: Name: base-URI-25 :)
(: Description:Verify normalization of xs:anyURI (encoded whitespaces in the middle). Used with static-base--uri function.:)

declare base-uri "http://www.example.org/%20%20examples";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:static-base-uri())