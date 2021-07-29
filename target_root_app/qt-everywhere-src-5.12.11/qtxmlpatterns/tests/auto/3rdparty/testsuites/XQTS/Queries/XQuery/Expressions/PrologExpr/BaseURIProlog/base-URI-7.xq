(: Name: base-URI-7 :)
(: Description: Evaluates base-uri with the fn:resolve-uri function. Typical usage:)

declare base-uri "http://www.example.com/";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:resolve-uri("examples"))